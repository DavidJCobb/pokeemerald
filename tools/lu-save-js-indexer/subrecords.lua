
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
   write_subrecord(view, "VARIABLS", 1, function()
      for _, pair in ipairs(pair_list) do
         write_variable(pair[1], pair[2])
      end
   end)
end