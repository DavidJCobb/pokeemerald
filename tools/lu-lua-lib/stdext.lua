
function string:findlast(needle)
   local i = self:match(".*" .. needle .. "()")
   if i == nil then
      return nil
   else
      return i + 1
   end
end

function string:startswith(start)
   return self:sub(1, #start) == start
end

do -- table
   function table.findindex(t, item)
      local size = #t
      for i = 1, size do
         local v = t[i]
         if v == item then
            return i
         end
      end
   end
   function table.findkey(t, item)
      for k, v in pairs(t) do
         if v == item then
            return k
         end
      end
   end
end