
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