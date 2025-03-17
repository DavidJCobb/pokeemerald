
-- Args: dataview, string (eightCC), int, function
function write_subrecord(view, signature, version, functor)
   assert(dataview.is(view), "write_subrecord must be given a dataview to write to")
   signature = tostring(signature)
   assert(#signature == 8, "write_subrecord must be given an eight-CC signature")
   view:append_eight_cc(signature)
   view:append_uint32(version)
   local size_offset = view.size
   view:append_uint32(0) -- dummy for size
   functor(view)
   assert(view.size >= size_offset, "your subrecord-writing functor shouldn't rewind the dataview into or before the subrecord header")
   local subrecord_size = view.size - size_offset - 4
   view:set_uint32(size_offset, subrecord_size)
end

--
-- Specific subrecord handlers:
--

-- Argument types: dataview, enumeration
function write_ENUMDATA_subrecord(view, enumeration)
   enumeration:update_cache()
   write_subrecord(view, "ENUMDATA", 1, function(view)
      local signed = (enumeration.cache.lowest or 0) < 0
      local prefix = enumeration.name .. "_"
      local sorted = enumeration:to_sorted_pairs()
      
      do
         local flags = 0
         if signed then
            flags = flags | 1 -- FLAG: Enum values should be interpreted as signed.
         end
         if enumeration.cache.sparse then
            flags = flags | 2 -- FLAG: Enum is sparse.
         end
         if enumeration.cache.lowest ~= 0 then
            flags = flags | 4 -- FLAG: Enum's lowest value is non-zero.
         end
         view:append_uint8(flags)
      end
      view:append_length_prefixed_string(1, prefix)
      view:append_uint32(#sorted)
      
      local names_start = nil
      if enumeration.cache.sparse then
         names_start = view.size
         view:append_uint32(0)
      else
         if enumeration.cache.lowest ~= 0 then
            view:append_uint32(enumeration.cache.lowest or 0)
         end
      end
      do -- Serialize enum member names as a block
         local first = true
         for _, pair in ipairs(sorted) do
            if first then
               first = false
            else
               view:append_uint8(0x00) -- separator
            end
            local name = pair[1]:sub(#prefix + 1)
            view:append_raw_string(name)
         end
      end
      if enumeration.cache.sparse then
         view:set_uint32(names_start, view.size - names_start - 4) -- subtract size of the size
         --
         -- Serialize enum values as a block
         --
         for _, pair in ipairs(sorted) do
            view:append_uint32(pair[2])
         end
      end
   end)
end

function write_ENUMDATA_slice_subrecords(enumeration, slice_size)
   enumeration:update_cache()
   
   local signed = (enumeration.cache.lowest or 0) < 0
   local prefix = enumeration.name .. "_"
   local sorted = enumeration:to_sorted_pairs()
   local flags  = 0
   do
      if signed then
         flags = flags | 1 -- FLAG: Enum values should be interpreted as signed.
      end
      --[[--if enumeration.cache.sparse then
         flags = flags | 2 -- FLAG: Enum is sparse.
      end--]]--
      if enumeration.cache.lowest ~= 0 then
         flags = flags | 4 -- FLAG: Enum's lowest value is non-zero.
      end
   end
   
   local out = {}
   
   local value_count = (enumeration.cache.highest or 0) - (enumeration.cache.lowest or 0)
   local slice_count = math.ceil(value_count / slice_size)
   for i = 1, slice_count do
      local from = (enumeration.cache.lowest or 0) + ((i - 1) * slice_size)
      local to   = from + slice_size
      
      local these  = {}
      local count  = 0
      local sparse = false
      do
         for j = 1, #sorted do
            local pair  = sorted[j]
            local value = pair[2]
            if value >= from and value <= to then
               if not sparse and count > 0 then
                  if these[count][2] < value - 1 then
                     sparse = true
                  end
               end
               count = count + 1
               these[count] = pair
            end
         end
      end
      
      local view = dataview()
      out[i] = view
      write_subrecord(view, "ENUMDATA", 1, function()
         do
            local f = flags
            if sparse then
               f = f | 2
            end
            view:append_uint8(f)
         end
         view:append_length_prefixed_string(1, prefix)
         view:append_uint32(count)
         
         local names_start
         if sparse then
            names_start = view.size
            view:append_uint32(0)
         else
            if enumeration.cache.lowest ~= 0 then
               view:append_uint32(enumeration.cache.lowest or 0)
            end
         end
         do -- Serialize enum member names as a block
            local first = true
            for _, pair in ipairs(these) do
               if first then
                  first = false
               else
                  view:append_uint8(0x00) -- separator
               end
               local name = pair[1]:sub(#prefix + 1)
               view:append_raw_string(name)
            end
         end
         if sparse then
            view:set_uint32(names_start, view.size - names_start - 4) -- subtract size of the size
            --
            -- Serialize enum values as a block
            --
            for _, pair in ipairs(these) do
               view:append_uint32(pair[2])
            end
         end
      end)
   end
   
   return out
end

function write_ENUNUSED_subrecord(view, enumeration)
   local size = #enumeration.unused_ranges
   if size <= 0 then
      return
   end
   write_subrecord(view, "ENUNUSED", 1, function(view)
      view:append_length_prefixed_string(1, enumeration.name .. "_")
      for i = 1, size do
         local range = enumeration.unused_ranges[i]
         view:append_uint32(range.first)
         view:append_uint32(range.last)
      end
   end)
end

function write_MAPSDATA_subrecord(view, map_data)
   if not map_data
   or not map_data.groups
   or not map_data.group_order
   then
      return
   end
   local group_count = #map_data.group_order
   if group_count <= 0 then
      return
   end
   write_subrecord(view, "MAPSDATA", 1, function(view)
      local function _serialize_name_block(names, detect_prefix)
         if detect_prefix then
            local prefix
            local count = #names
            if count > 2 then
               prefix = names[1]
               local p_size = #prefix
               for i = 2, count do
                  local name = names[i]
                  local size = math.min(p_size, #name)
                  local same = 0
                  for j = 1, size do
                     local u = prefix:byte(j)
                     local v = name:byte(j)
                     if u ~= v then
                        break
                     end
                     same = j
                  end
                  if same > 0 then
                     if p_size > same then
                        p_size = same
                     end
                  else
                     prefix = nil
                     p_size = 0
                     break
                  end
               end
               if p_size > 0 then
                  if p_size > 255 then
                     p_size = 255
                  end
                  prefix = prefix:sub(1, p_size)
                  
                  local altered = {}
                  for i = 1, count do
                     altered[i] = names[i]:sub(p_size + 1)
                  end
                  names = altered
               end
            end
            view:append_length_prefixed_string(1, prefix or "")
         end
         local start = view.size
         view:append_uint32(0)
         do -- Serialize enum member names as a block
            local first = true
            for _, name in ipairs(names) do
               if first then
                  first = false
               else
                  view:append_uint8(0x00) -- separator
               end
               view:append_raw_string(name)
            end
         end
         view:set_uint32(start, view.size - start - 4)
      end
   
      _serialize_name_block(map_data.group_order, true)
      for _, name in ipairs(map_data.group_order) do
         local map_names = map_data.groups[name]
         _serialize_name_block(map_names, true)
      end
   end)
end

-- Argument types: dataview, array<pair<name, value>>
--
-- We take a list of name/value pairs, rather than a conventional table, 
-- to ensure that variables are written in a consistent order, no matter 
-- what order they were originally loaded and parsed in.
function write_VARIABLS_subrecord(view, pair_list)
   function write_variable(name, value)
      if not value then
         return
      end
      view:append_length_prefixed_string(2, name)
      view:append_uint8((value < 0) and 0x01 or 0x00) -- value < 0 ? 1 : 0 -- "is signed" byte
      view:append_uint32(value)
   end
   write_subrecord(view, "VARIABLS", 1, function(view)
      for _, pair in ipairs(pair_list) do
         write_variable(pair[1], pair[2])
      end
   end)
end