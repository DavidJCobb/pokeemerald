
c_lexed_token = make_class({
   constructor = function(self, t, d)
      self.type = t
      self.data = d
   end,
   instance_members = {
      is_symbol = function(self, s)
         return self.type == "token" and self.data == s
      end
   },
   __tostring = function(self)
      if self.type == "token" then
         return self.data
      end
      if self.type == "identifier" then
         return self.data
      end
      if self.type == "constant" then
         return tostring(self.data)
      end
      if self.type == "string-literal" then
         return self.data:gsub("\\", "\\\\"):gsub('"', "\\\"")
      end
      return "@"
   end
})

--[[--

   A function which, given a string, lexes it into a sequence of c_lexed_token 
   instances.

--]]--
lex_c = nil
do
   local CHARS = {
      "<",
      ">",
      "{",
      "}",
      "(",
      ")",
      "[",
      "]",
      "!", -- not
      "%", -- modulo
      "^", -- XOR
      "&", -- AND/addressof
      "*", -- multiply/dereference
      "+", -- plus/positive
      "-", -- minus/negative
      "=", -- assign
      "/", -- divide
      ".", -- member access
      "|", -- OR
      ";", -- statement separator
      "?", -- ternary condition separator
      ":", -- ternary branch separator
   }
   local TOKENS = {
      "<<", -- shift left
      ">>", -- shift right
      "==", -- equal
      "!=", -- not equal
      ">=", -- greater than or equal
      "<=", -- less than or equal
      "->", -- dereferencing member access
   }

   function lex(str)
      local tokens = {}
   
      local data      = ""
      local str_delim = "" -- delimiter of the string we're currently in, or an empty string
      local escaped   = false
      local skip      = 0
      
      function commit_data()
         if data == "" then
            return
         end
         local n = tonumber(data)
         if n then
            tokens[#tokens + 1] = c_lexed_token("constant", n)
         else
            --
            -- TODO: Validate identifier syntax.
            --
            tokens[#tokens + 1] = c_lexed_token("identifier", data)
         end
         data = ""
      end
      
      for i = 1, #str do
         if skip > 0 then
            skip = skip - 1
         else
            local c = str:sub(i, i)
            if str_delim ~= "" then
               if escaped then
                  data    = data .. c
                  escaped = false
               else
                  if c == str_delim then
                     tokens[#tokens + 1] = c_lexed_token("string-literal", data)
                     str_delim = ""
                  elseif c == "\\" then
                     escaped = true
                  end
               end
            else
               if c == "'" or c == '"' then
                  commit_data()
                  str_delim = c
               elseif c == " " or c == "\r" or c == "\n" then
                  commit_data()
               else
                  local matched = nil
                  for _, v in ipairs(TOKENS) do
                     if c == v:sub(1, 1) and str:sub(i, i + #v - 1) == v then
                        matched = v
                        break
                     end
                  end
                  if matched then
                     commit_data()
                     tokens[#tokens + 1] = c_lexed_token("token", matched)
                     skip = #matched - 1
                  else
                     for _, v in ipairs(CHARS) do
                        if c == v then
                           matched = v
                           break
                        end
                     end
                     if matched then
                        commit_data()
                        tokens[#tokens + 1] = c_lexed_token("token", matched)
                     else
                        data = data .. c
                     end
                  end
               end
            end
         end
      end
      if #str_delim > 0 then
         error("Unterminated string literal.")
      end
      commit_data()
      
      return tokens
   end
   lex_c = lex
end
