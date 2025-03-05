
-- Analogue to JavaScript's DataView. Note that this will be drastically 
-- less efficient than native code (including the native code that backs 
-- the JavaScript implementation). We should investigate building a C++ 
-- library to help Lua out here.
dataview = make_class({
   constructor = function(value)
      self._bytes = {}
   end,
   getters = {
      size = function(self)
         return #self._bytes
      end,
   },
   instance_members = {
      _validate_offset = function(self, offset, size)
         if not offset then
            error("offset required")
         end
         if offset < 0 then
            error("offset out of bounds")
         end
         if size then
            if offset + size >= #self.value then
               error("offset out of bounds")
            end
         else
            if offset >= #self.value then
               error("offset out of bounds")
            end
         end
      end,
      
      resize = function(self, size)
         local prior = #self._bytes
         if prior == size then
            return
         end
         if prior > size then -- if shrinking
            for i = size + 1, prior do
               self._bytes[i] = nil
            end
         else -- if enlarging
            for i = prior, size + 1 do
               self._bytes[i] = 0
            end
         end
      end,
      
      save_to_file = function(self, path)
         local AVOID_LUA_STRING_INTERNING = true
      
         local file = io.open(path, "wb")
         if AVOID_LUA_STRING_INTERNING then
            for i = 1, #self._bytes do
               local s = string.char(self._bytes[i])
               file:write(s)
            end
         else
            local str = string.char(unpack(self._bytes))
            file:write(str)
         end
         file:close()
      end,
      
      get_uint8 = function(self, offset)
         self:_validate_offset(offset)
         return self._bytes[offset + 1]
      end,
      set_uint8 = function(self, offset, value)
         self:_validate_offset(offset)
         if value < 0 then
            value = value + 255 -- cast from signed to unsigned
         end
         if value < 0 or value > 255 then
            error("value out of bounds")
         end
         self._bytes[offset] = value
      end,
      append_uint8 = function(self, value)
         local size = #self._bytes
         self._bytes[size + 1] = 0
         self.set_uint8(size, value)
      end,
      
      get_uint16 = function(self, offset, big_endian)
         self:_validate_offset(offset, 2)
         offset = offset + 1
         local a = self._bytes[offset]
         local b = self._bytes[offset + 1]
         if big_endian
            return b | (a << 16)
         else
            return a | (b << 16)
         end
      end,
      set_uint16 = function(self, offset, value, big_endian)
         self:_validate_offset(offset, 2)
         offset = offset + 1
         if value < 0 then
            value = value + 65535 -- cast from signed to unsigned
         end
         if value < 0 or value > 65535 then
            error("value out of bounds")
         end
         local lo = value & 0xFF
         local hi = value >> 16
         if big_endian then
            self._bytes[offset]     = hi
            self._bytes[offset + 1] = lo
         else
            self._bytes[offset]     = lo
            self._bytes[offset + 1] = hi
         end
      end,
      append_uint16 = function(self, value, big_endian)
         local size = #self._bytes
         self._bytes[size + 1] = 0
         self._bytes[size + 2] = 0
         self.set_uint16(size, value, big_endian)
      end,
      
      get_uint32 = function(self, offset, big_endian)
         self:_validate_offset(offset, 4)
         offset = offset + 1
         
         local v = 0
         if big_endian then
            for i = 0, 3 do
               local byte = self._bytes[offset + i]
               v = v | (byte << (8 * (3 - i)))
            end
         else
            for i = 0, 3 do -- DD CC BB AA -> 0xAABBCCDD
               local byte = self._bytes[offset + i]
               v = v | (byte << (8 * i))
            end
         end
         return v
      end,
      set_uint32 = function(self, offset, value, big_endian)
         self:_validate_offset(offset, 4)
         
         local units = {} -- 0xAABBCCDD -> DD, CC, BB, AA
         for i = 1, 4 do
            units[i] = (offset >> (8 * (i - 1))) & 0xFF
         end
         if big_endian then
            for i = 1, 4 do
               self._bytes[offset + i] = units[4 - i + 1]
            end
         else
            for i = 1, 4 do
               self._bytes[offset + i] = units[i]
            end
         end
      end,
      append_uint32 = function(self, value, big_endian)
         local size = #self._bytes
         self._bytes[size + 1] = 0
         self._bytes[size + 2] = 0
         self._bytes[size + 3] = 0
         self._bytes[size + 4] = 0
         self.set_uint32(size, value, big_endian)
      end,
   }
})