
xml = {}

xml.builtin_entities = {
   apos = "'",
   lt   = "<",
   gt   = ">",
   amp  = "&",
   quot = '"',
}

xml.node = make_class({})

xml.text = make_class({
   superclass  = xml.node,
   constructor = function(self, text)
      self.data = text
   end
})

do
   local instance_members = {}
   xml.element = make_class({
      superclass  = xml.node,
      constructor = function(self, name)
         self.node_name   = name or ""
         self.attributes  = {}
         self.children    = {}
         self.parent      = nil
         self.self_closed = false
      end,
      instance_members = instance_members
   })
   do -- Member functions: child manipulation
      function instance_members:append_child(node)
         local size = #self.children
         do
            local t = type(node)
            if t == "boolean" or t == "number" then
               node = tostring(node)
            end
         end
         if type(node) == "string" then
            local back = self.children[size]
            if back and xml.text.is(back) then
               back.data = back.data .. node
               return
            end
            node = xml.text(node)
         else
            if not xml.node.is(node) then
               error("Invalid argument type.")
            end
            if node.parent then
               node.parent:remove_child(node)
            end
         end
         self.children[size + 1] = node
         node.parent = self
      end
      function instance_members:remove_child(node)
         if not node or node.parent ~= self then
            error("Can only remove our own child nodes.")
         end
         local i = nil
         for k, v in ipairs(self.children) do
            if v == node then
               i = k
               break
            end
         end
         assert(i, "A node must be present in its parent's child list.")
         local size = #self.children
         for j = i, size do
            self.children[j] = self.children[j + 1]
         end
         node.parent = nil
      end
      function instance_members:remove()
         if self.parent then
            self.parent:remove_child(self)
         end
      end
   end
   do -- Member functions: convenience accessors
      function instance_members:children_by_node_name(name)
         local list  = {}
         local count = 0
         for i = 1, #self.children do
            local child = self.children[i]
            if xml.element.is(child) and child.node_name == name then
               count = count + 1
               list[count] = child
            end
         end
         return list
      end
      function instance_members:for_each_attribute(functor)
         for k, v in pairs(self.attributes) do
            local result = functor(k, v)
            if result == false then
               break
            end
         end
      end
      function instance_members:for_each_child(functor)
         for i = 1, #self.children do
            local result = functor(self.children[i])
            if result == false then
               break
            end
         end
      end
      function instance_members:for_each_child_element(functor)
         for i = 1, #self.children do
            local node = self.children[i]
            if xml.element.is(node) then
               local result = functor(node)
               if result == false then
                  break
               end
            end
         end
      end
   end
end

do
   local instance_getters = {}
   local instance_members = {}
   xml.parser = make_class({
      constructor = function(self, options)
         if options then
            self.entities = options.entities or {}
         else
            self.entities = {}
         end
         self.pos      = nil
         self.text     = nil
         self.length   = nil
         self.prologue = nil
         self.root     = nil -- XML element
         self.state    = {
            target  = nil, -- element we're inside of
            parsing = nil, -- nil, "OPEN", "CHILDREN", "CLOSE"
         }
      end,
      getters          = instance_getters,
      instance_members = instance_members,
   })
   
   function instance_members:parse(str)
      self.pos      = 1
      self.text     = str
      self.length   = #str
      self.prologue = nil
      
      self:_parse_prologue()
      self:_parse_element()
      while self:_consume_misc() do
      end
      if self.pos < self.length then
         error("Unexpected content at the end of the file.")
      end
   end
   
   function instance_getters:_all_consumed_text()
      return self.text:sub(1, self.pos - 1)
   end
   function instance_getters:_all_remaining_text()
      return self.text:sub(self.pos)
   end
   function instance_getters:_remaining()
      return self.length - self.pos + 1
   end
   
   -- "Checkpoint" system. If a function needs to parse an optional 
   -- sequence of tokens, you can spawn a checkpoint in a to-be-closed 
   -- variable: when it goes out of scope, it'll automatically rewind 
   -- the parser. If you fully parse the optional token sequence, you 
   -- can "commit" the checkpoint in order to prevent the rewind.
   local checkpoint_metatable = {}
   do
      function checkpoint_metatable:__close()
         if rawget(self, "disarmed") then
            return
         end
         rawset(self, "disarmed", true)
         rawget(self, "parser").pos = rawget(self, "pos")
      end
      function checkpoint_metatable:__index(k)
         if k == "commit" then
            return checkpoint_metatable.commit
         end
      end
      function checkpoint_metatable:commit()
         rawset(self, "disarmed", true)
      end
   end
   function instance_members:_make_checkpoint()
      local checkpoint = {
         parser   = self,
         pos      = self.pos,
         disarmed = false,
      }
      setmetatable(checkpoint, checkpoint_metatable)
      return checkpoint
   end
   
   -- Consumes the desired text if it's immediately ahead of `pos`, 
   -- and returns whether it was consumed.
   function instance_members:_consume(desired)
      local size = #desired
      if size > self._remaining then
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
   
   -- Consumes the desired text if it's immediately ahead of `pos`. 
   -- Returns nil if nothing is consumed. If the pattern has any 
   -- captures, then returns a list of the captured text; otherwise, 
   -- returns the whole captured text.
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
   
   function instance_members:_consume_char()
      if self._remaining <= 0 then
         return nil
      end
      local c = self.text:sub(self.pos, self.pos)
      self.pos = self.pos + 1
      return c
   end
   
   function instance_members:_consume_until(desired)
      local i = self.text:find(desired, self.pos, true)
      if i then
         local s = self.text:sub(self.pos, i - 1)
         self.pos = i
         return s
      end
      return false
   end
   
   -- Consumes consecutive whitespace characters, and returns whether 
   -- at least one character was consumed.
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
   
   -- Common token.
   function instance_members:_consume_eq()
      local prior = self.pos
      self:_consume_whitespace()
      if self:_consume_char() ~= "=" then
         self.pos = prior
         return false
      end
      self:_consume_whitespace()
      return true
   end
   
   -- Common token.
   function instance_members:_consume_quoted_string(desired, plain)
      local prior = self.pos
      local delim = self:_consume_char()
      if not delim or not (delim == '"' or delim == "'") then
         self.pos = prior
         return nil
      end
      local value
      if desired then
         if plain then
            value = self:_consume(desired)
         else
            value = self:_consume_pattern(desired)
         end
      else
         value = self:_consume_until(delim)
      end
      if not value or self:_consume_char() ~= delim then
         self.pos = prior
         return nil
      end
      return value
   end
   
   -- Common token.
   function instance_members:_consume_name()
      return self:_consume_pattern("^[:A-Za-z_\xC0-\xD6\xF8-\u{2FF}\u{370}-\u{37D}\u{37F}-\u{1FFF}\u{200C}\u{200D}\u{2070}-\u{218F}\u{2C00}-\u{2FEF}\u{3001}-\u{D7FF}\u{F900}-\u{FDCF}\u{FDF0}-\u{FFFD}\u{10000}-\u{EFFFF}][:A-Za-z_\xC0-\xD6\xF8-\u{2FF}\u{370}-\u{37D}\u{37F}-\u{1FFF}\u{200C}\u{200D}\u{2070}-\u{218F}\u{2C00}-\u{2FEF}\u{3001}-\u{D7FF}\u{F900}-\u{FDCF}\u{FDF0}-\u{FFFD}\u{10000}-\u{EFFFF}%.0-9\xB7\u{300}-\u{36F}\u{203F}\u{2040}-]*")
   end
   
   -- Common token.
   function instance_members:_consume_comment()
      local checkpoint <close> = self:_make_checkpoint()
      if not self:_consume("<!--") then
         return false
      end
      local n = self.text:find("--", self.pos, true)
      if not n then
         error("Unterminated comment.")
      end
      local c = self.text:sub(n + 2, n + 2)
      if c ~= ">" then
         error("Unexpected `--` in a comment.")
      end
      -- comment text == self.text:sub(self.pos, n - 1)
      self.pos = n + 3
      checkpoint:commit()
      return true
   end
   
   -- Common token.
   function instance_members:_consume_misc()
      return self:_consume_whitespace()
      or     self:_consume_comment()
      -- TODO: or self:_consume_processing_instruction()
   end
   
   function instance_members:_parse_prologue()
      if self:_consume("<?xml") then -- XMLDecl
         local version    = nil
         local encoding   = nil
         local standalone = nil
         do -- VersionInfo
            if not self:_consume_whitespace() then
               error("Invalid XML declaration.")
            end
            if not self:_consume("version") then
               error("Invalid XML declaration.")
            end
            if not self:_consume_eq() then
               error("Invalid XML declaration.")
            end
            local value = self:_consume_quoted_string("^(1%.%d+)")
            if not value then
               error("Invalid XML declaration.")
            end
            version = tonumber(value)
         end
         encoding = (function(self)
            local checkpoint <close> = self:_make_checkpoint()
            
            if not self:_consume_whitespace()
            or not self:_consume("encoding")
            or not self:_consume_eq()
            then
               return nil
            end
            local value = self:_consume_quoted_string("[A-Za-z][A-Za-z0-9%._-]*")
            if not value then
               return nil
            end
            checkpoint:commit()
            return value
         end)(self)
         standalone = (function(self)
            local checkpoint <close> = self:_make_checkpoint()
            
            if not self:_consume_whitespace()
            or not self:_consume("standalone")
            or not self:_consume_eq()
            then
               return nil
            end
            local value = self:_consume_quoted_string("[A-Za-z][A-Za-z0-9%._-]*")
            if not value or (value == "yes" or value == "no") then
               return nil
            end
            checkpoint:commit()
            return value == "yes"
         end)(self)
         --
         if version or encoding or standalone then
            self.prologue = {
               version    = version,
               encoding   = encoding,
               standalone = standalone,
            }
         end
      end
      while self:_consume_misc() do
      end
      -- TODO: doctypedecl and, if it's present, _consume_misc() again
   end
   
   -- CharData
   function instance_members:_parse_char_data()
      local i = self.text:find("]]>", self.pos, true)
      local j = math.min(
         self.text:find("<", self.pos, true) or self.length + 1,
         self.text:find("&", self.pos, true) or self.length + 1
      )
      if i and i < j then
         error("Unexpected CDATA section terminator (`]]>`) in text content.")
      end
      if j > self.pos then
         local text = self.text:sub(self.pos, j - 1)
         self.state.target:append_child(text)
         self.pos = j
         return true
      end
      return false
   end
   
   -- CD
   function instance_members:_parse_cdata_section()
      if self:_consume("<![CDATA[") then
         local data = self:_consume_until("]]>")
         self.pos = self.pos + 3
         self.state.target:append_child(data)
         return true
      end
      return false
   end
   
   function instance_members:_consume_entity_reference()
      if self:_consume("&#") then
         local cc = self:_consume_pattern("x(%x+);")
         if cc then
            cc = tonumber("0x" .. cc[1])
         else
            cc = self:_consume_pattern("(%d+);")
            if cc then
               cc = tonumber(cc[1])
            else
               error("Malformed XML character-code entity.")
            end
         end
         return string.char(cc)
      end
      --
      -- Named entities:
      --
      if not self:_consume("&") then
         return false
      end
      local name = self:_consume_name()
      if self:_consume_char() ~= ";" then
         error("Malformed XML entity `" .. name .. "`.")
      end
      local data = xml.builtin_entities[name]
      if not data then
         data = self.entities[name]
         if not data then
            error("Unrecognized XML entity `" .. name .. "`.")
         end
      end
      return data
   end
   
   function instance_members:_parse_entity_reference()
      local data = self:_consume_entity_reference()
      if data then
         self.state.target:append_child(data)
         return true
      end
      return false
   end
   
   function instance_members:_consume_attribute_value()
      local delim = self:_consume_char()
      if delim ~= "'" and delim ~= '"' then
         return false
      end
      local prior = self.pos
      local value = ""
      local ended = false
      while true do
         local i = self.text:find(delim, self.pos, true)
         local j = self.text:find("&",   self.pos, true)
         local k = self.text:find("<",   self.pos, true)
         if k and k < i and k < j then
            error("Unexpected `<`.")
         end
         if j and j < i then
            if j > self.pos then
               value = value .. self.text:sub(self.pos, j - 1)
               self.pos = j
            end
            value = value .. self:_consume_entity_reference()
         elseif i then
            value    = value .. self.text:sub(self.pos, i - 1)
            self.pos = i + 1
            ended    = true
            break
         else
            break -- unterminated attribute
         end
      end
      if not ended then
         self.pos = prior
         return false
      end
      return value
   end
   
   function instance_members:_parse_element()
      local checkpoint <close> = self:_make_checkpoint()
      
      if not self:_consume("<") then
         return nil
      end
      local node_name = self:_consume_name()
      if not node_name then
         return nil
      end
      
      local elem = xml.element(node_name)
      if self.state.target then
         self.state.target:append_child(elem)
      else
         if self.root then
            error("Multiple top-level elements?")
         else
            self.root = elem
         end
      end
      self.state.target = elem
      
      while true do
         local checkpoint <close> = self:_make_checkpoint()
         if not self:_consume_whitespace() then
            break
         end
         local name = self:_consume_name()
         if not name then
            break
         end
         if not self:_consume_eq() then
            error("Malformed attribute `" .. name .. "` on element `" .. node_name .. "`.")
         end
         local value = self:_consume_attribute_value()
         if not value then
            error("Malformed attribute `" .. name .. "` on element `" .. node_name .. "`.")
         end
         if elem.attributes[name] then
            error("Attribute `" .. name .. "` appears more than once on element `" .. node_name .. "`.")
         end
         elem.attributes[name] = value
         checkpoint:commit()
      end
      self:_consume_whitespace()
      
      if self:_consume("/>") then
         elem.self_closed  = true
         self.state.target = elem.parent
         checkpoint:commit()
         return elem
      elseif self:_consume_char() == ">" then
         elem.self_closed = false
      else
         error("Malformed start tag for `" .. node_name .. "`.")
      end
      --
      -- Parse child content.
      --
      do
         self:_parse_char_data()
         do
            local any = true
            while any do
               any = false
               if self:_parse_cdata_section()
               or self:_parse_entity_reference()
               or self:_consume_comment()
               or self:_parse_element()
               then
                  any = true
                  self:_parse_char_data()
               end
            end
         end
      end
      --
      -- Parse closing tag.
      --
      if not self:_consume("</") then
         error("Expected closing tag for `" .. node_name .. "`.")
      end
      local close_name = self:_consume_name()
      if close_name ~= node_name then
         error("Expected closing tag for `" .. node_name .. "`; saw closing tag for `" .. close_name .. "`.")
      end
      self:_consume_whitespace()
      if self:_consume_char() ~= ">" then
         error("Malformed closing tag for `" .. node_name .. "`.")
      end
      
      self.state.target = elem.parent
      checkpoint:commit()
      return elem
   end
end