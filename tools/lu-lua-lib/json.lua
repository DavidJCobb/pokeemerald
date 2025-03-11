
local function is_representable_as_array(v)
   if type(v) ~= "table" then
      return false
   end
   local keys = {}
   local max  = 0
   for k, _ in pairs(v) do
      k = tonumber(k)
      if not k then -- non-numeric key present
         return false
      end
      if k < 1 or k ~= math.floor(k) then -- non-positive-integer key present
         return false
      end
      keys[k] = true
      if k > max then
         max = k
      end
   end
   if max < 1 then -- none of the found keys are array indices
      return false
   end
   for i = 1, max do
      if not keys[i] then -- non-contiguous keys
         return false
      end
   end
   return true
end

local function table_has_cyclical_references(subject, _seen)
   if not _seen then
      _seen = { v }
   end
   for _, v in pairs(subject) do
      if type(v) == "table" or type(v) == "userdata" then
         local i = 0
         for j, w in ipairs(_seen) do
            if w == v then
               return true
            end
            i = j
         end
         _seen[i + 1] = v
         if table_has_cyclical_references(v, _seen) then
            return true
         end
      end
   end
   return false
end

local function string_to_literal(v)
   local needs_escape = v:find('[^"\x20\x21\x23-\x5B\x5D-\u{10FFFF}]')
   if not needs_escape then
      local has_quotes = v:find('"', 1, true)
      if has_quotes then
         v = v:gsub('"', '\\"')
      end
      return '"' .. v .. '"'
   end
   local esc = ""
   local i   = 1
   while i <= #t do
      local j = v:find('[^\x20\x21\x23-\x5B\x5D-\u{10FFFF}]', i)
      if j then
         if j > i then
            esc = esc .. v:sub(i, j - 1)
         end
         local ch = t:byte(j)
         if ch > 0xFF then
            esc = esc .. string.format("\\u%04X", ch)
         else
            esc = esc .. string.format("\\x%02X", ch)
         end
         i = j + 1
      else
         esc = esc .. v:sub(i)
         break
      end
   end
   return '"' .. esc .. '"'
end

-- Exports:
json = {}

function json.to(v, options)
   if v == nil then
      return "null"
   end
   local t = type(v)
   if t == "function" then
      error("Cannot convert a function to JSON.")
   end
   if t == "userdata" then
      local mt = getmetatable(v)
      if mt and mt.__tostring then
         v = tostring(v)
         t = "string"
      else
         error("Cannot convert a userdata to JSON (absent a __tostring metamethod).")
      end
   else
      if t == "boolean" then
         return v and "true" or "false"
      end
      if t == "number" then
         return tostring(v)
      end
   end
   if t == "string" then
      return string_to_literal(v)
   end
   --
   -- Handle JSON objects.
   --
   assert(t == "table", "unhandled Lua type")
   if table_has_cyclical_references(v) then
      error("cannot serialize an object tree to JSON if it contains cyclical references")
   end
   local indent = 0
   local pretty = false
   if options then
      pretty = options.pretty_print or false
      indent = options.indent or 0
   end
   local out = ""
   
   local indent_here = ""
   local indent_nest = ""
   if pretty and indent then
      if indent > 0 then
         indent_here = string.format("%" .. indent .. "s", "")
      end
      indent_nest = indent_here .. "   "
      if not options.__recursing then
         out = indent_here
      end
   end
   local sub_options = { __recursing = true }
   if options then
      for k, v in pairs(options) do
         sub_options[k] = v
      end
      if pretty then
         sub_options.indent = indent + 3
      end
   end
   
   if is_representable_as_array(v) then
      out = out .. '['
      local empty = true
      for i, w in ipairs(v) do
         empty = false
         if i > 1 then
            out = out .. ','
         end
         local x = json.to(w, sub_options)
         if pretty then
            out = out .. "\n"
            out = out .. indent_nest
         end
         out = out .. x
      end
      if pretty and not empty then
         out = out .. "\n"
         out = out .. indent_here
      end
      out = out .. ']'
   else
      out = out .. '{'
      --
      local sorted_keys = {} -- ensure deterministic ordering
      do
         local count = 0
         for k, _ in pairs(v) do
            count = count + 1
            sorted_keys[count] = k
         end
         if count > 0 then
            table.sort(sorted_keys, function(a, b)
               local na = tonumber(a)
               local nb = tonumber(b)
               if na and nb then
                  return na < nb
               end
               return tostring(a) < tostring(b)
            end)
         end
      end
      for i, k in ipairs(sorted_keys) do
         if i > 1 then
            out = out .. ','
         end
         local w = v[k]
         local x = json.to(w, sub_options)
         if pretty then
            out = out .. "\n"
            out = out .. indent_nest
         end
         if type(k) ~= "string" then
            k = tostring(k)
         end
         out = out .. json.to(k, sub_options)
         out = out .. ": "
         out = out .. x
      end
      if pretty and #sorted_keys > 0 then
         out = out .. "\n"
         out = out .. indent_here
      end
      out = out .. '}'
   end
   return out
end

local parser
do
   local instance_members = {}
   parser = make_class({
      constructor = function(self, text)
         self.pos    = 1
         self.text   = text
         self.length = #text
         self.result = nil
      end,
      instance_members = instance_members
   })
   
   function instance_members:_consume_char()
      if self.pos > self.length then
         return nil
      end
      local c = self.text:sub(self.pos, self.pos)
      self.pos = self.pos + 1
      return c
   end
   function instance_members:_consume_pattern(desired)
      local m = { self.text:match(desired, self.pos) }
      local s = #m
      if s == 0 or m[1] == nil then
         return nil
      end
      --
      -- The Lua manual doesn't document this, but the last match 
      -- is always the full matched text.
      --
      local whole = m[s]
      self.pos = self.pos + #whole
      if s == 1 then
         return whole
      end
      m[s] = nil
      return m
   end
   function instance_members:_consume_plaintext(desired)
      local size = #desired
      if self.pos + size - 1 > self.length then
         return false
      end
      local n = self.pos
      for i = 1, size do
         local a = desired:sub(i,i)
         local b = self.text:sub(n,n)
         if a ~= b then
            return false
         end
         n = n + 1
      end
      self.pos = self.pos + size
      return true
   end
   function instance_members:_consume_whitespace()
      if self.pos > self.length then
         return false
      end
      local i = self.text:find("%S", self.pos)
      if i == self.pos then
         return false
      end
      --
      -- If `i` is nil, then the entire rest of the string is whitespace.
      --
      self.pos = i or self.length + 1
      return true
   end
   
   function instance_members:_demand_char(c)
      local d = self:_consume_char()
      if d ~= c then
         if not d then
            error("Unexpected EOF (expected `" .. c .. "` at position " .. self.pos .. ").")
         end
         error("Unexpected `" ..d .. "` (expected `" .. c .. "` at position " .. self.pos .. ").")
      end
   end
   
   function instance_members:_consume_boolean() -- returns found boolean, or nil if none
      if self:_consume_plaintext("true") then
         return true
      end
      if self:_consume_plaintext("false") then
         return false
      end
      return nil
   end
   function instance_members:_consume_null() -- returns true if found; false otherwise
      return self:_consume_plaintext("null")
   end
   function instance_members:_consume_number() -- returns found value, or nil if none
      local prior = self.pos
      local negative = self:_consume_plaintext("-")
      local integer  = self:_consume_pattern("^[0-9]+")
      if not integer then
         prior = self.pos
         return nil
      end
      local fraction = self:_consume_pattern("^%.[0-9]+") or ""
      local exponent = self:_consume_pattern("^[eE][+-]?[0-9]+") or ""
      
      local str = (negative and "-" or "") .. integer .. fraction .. exponent
      local val = tonumber(str)
      return val
   end
   function instance_members:_consume_string() -- returns found value, or nil if none
      if not self:_consume_plaintext('"') then
         return nil
      end
      local text = ""
      local i    = self.pos
      while true do
         local j = self.text:find('\\', i, true)
         local k = self.text:find('"',  i, true)
         if j and (not k or j < k) then
            if j > i then
               text = text .. self.text:sub(i, j - 1)
            end
            i = j
            local esc = self.text:match("^x(%x%x)", i)
            if esc then
               text = text .. string.char(tonumber("0x" .. esc))
               i    = i + 3
            else
               esc = self.text:match("^u(%x%x%x%x)", i)
               if esc then
                  text = text .. string.char(tonumber("0x" .. esc))
                  i    = i + 5
               else
                  esc  = self.text:sub(i, i)
                  text = text .. esc
                  i    = i + 1
               end
            end
         elseif k then
            if k > i then
               text = text .. self.text:sub(i, k - 1)
            end
            i = k + 1
            break
         else
            error("unterminated string literal")
         end
      end
      self.pos = i
      return text
   end
   function instance_members:_consume_array() -- returns found value, or nil if none
      local prior = self.pos
      if self:_consume_char() ~= '[' then
         self.pos = prior
         return nil
      end
      local array = {}
      local index = 1
      local first = true
      while true do
         self:_consume_whitespace()
         if first then
            first = false
         else
            if not self:_consume_plaintext(",") then
               break
            end
            self:_consume_whitespace()
         end
         if self:_parse_value(array, index) then
            index = index + 1
         else
            break
         end
      end
      self:_consume_whitespace()
      self:_demand_char(']')
      return array
   end
   function instance_members:_consume_object() -- returns found value, or nil if none
      local prior = self.pos
      if self:_consume_char() ~= '{' then
         self.pos = prior
         return nil
      end
      local value = {}
      local first = true
      while true do
         self:_consume_whitespace()
         if first then
            first = false
         else
            if not self:_consume_plaintext(",") then
               break
            end
            self:_consume_whitespace()
         end
         local subkey = self:_consume_string()
         if not subkey then
            break
         end
         self:_consume_whitespace()
         self:_demand_char(":")
         self:_consume_whitespace()
         if self:_parse_value(value, subkey) then
            self:_consume_whitespace()
         else
            error("Value for key `" .. subkey .. "` expected at position " .. self.pos .. ".")
         end
      end
      self:_consume_whitespace()
      self:_demand_char('}')
      return value
   end
   
   -- If it finds a value, writes the value to `dst[key]` and returns 
   -- true. Otherwise, returns false without writing anything.
   function instance_members:_parse_value(dst, key)
      local function _set(v)
         local nk = tonumber(key)
         local sk = tostring(key)
         if nk then
            dst[sk] = nil -- in Lua, foo[5] ~= foo["5"]
            dst[nk] = v
         else
            dst[sk] = v
         end
      end
      
      if self:_consume_null() then
         _set(nil)
         return true
      end
      local v = self:_consume_boolean()
      if v ~= nil then
         _set(v)
         return true
      end
      v = self:_consume_number() or
         self:_consume_string() or
         self:_consume_array() or
         self:_consume_object()
      if v then
         _set(v)
         return true
      end
      
      return false
   end
   
   function instance_members:exec()
      self:_consume_whitespace()
      if not self:_parse_value(self, "result") then
         error("No value present.")
      end
      self:_consume_whitespace()
   end
end

json.parser = parser

function json.from(text)
   local p = parser(text)
   p:exec()
   return p.result
end