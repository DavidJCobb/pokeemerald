--[[--

   Stringify a table or nil value.
   
   Many years ago, if you went to a web browser and called `foo.toSource()` 
   on some JavaScript object `foo`, the browser would print the object in a 
   non-standard JSON notation, wherein all nested Objects were tagged with 
   a number sign and a unique ID, e.g.
   
      {bar:#1{baz:#2{hello:"world"}}}
      
   The purpose of this notation was to let you distinguish between multiple 
   references to the same object, versus references to different objects 
   with identical contents. It had the added benefit of ensuring that the 
   stringifier wouldn't choke on cyclical references.
   
   We use the same convention here.

--]]--
function stringify_table(tbl, indent, do_not_invoke_tostring, _seen)
   if not indent then
      indent = ""
   end
   if not tbl then
      return indent .. "nil"
   end
   if not _seen then
      _seen = {}
   end
   local text  = "{"
   local empty = true
   for k, v in pairs(tbl) do
      empty = false
      local line = indent .. "   " .. k .. " = "
      local t    = type(v)
      if t == "table" then
         local seen = nil
         for i, prior in ipairs(_seen) do
            if prior == v then
               seen = i
               break
            end
         end
         if seen then
            line = line .. "#" .. tostring(seen)
         else
            seen = #_seen + 1
            _seen[seen] = v
            line = line .. "#" .. tostring(seen)
            
            local as_string = nil
            if not do_not_invoke_tostring then
               local mt = getmetatable(v)
               if mt and mt.__tostring then
                  as_string = tostring(v)
                  if as_string == "" then
                     as_string = nil
                  end
               end
            end
            if as_string then
               as_string = as_string:gsub("\\", "\\\\"):gsub("\"", "\\\"")
               line = line .. '"' .. as_string .. '"'
            else
               line = line .. stringify_table(v, indent .. "   ", do_not_invoke_tostring, _seen)
            end
         end
      elseif t == "function" then
         line = line .. "function() ... end"
      elseif t == "string" then
         v = v:gsub("\\", "\\\\"):gsub("\"", "\\\"")
         line = line .. '"' .. v .. '"'
      else
         line = line .. tostring(v)
      end
      text = text .. "\n" .. line .. ","
   end
   if not empty then
      text = text .. "\n" .. indent
   end
   text = text .. "}"
   return text
end

function print_table(tbl, indent, do_not_invoke_tostring)
   local text = stringify_table(tbl, indent, do_not_invoke_tostring)
   print(text)
end