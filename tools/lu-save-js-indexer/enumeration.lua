local function constructor(self, name)
   self.name  = name
   self.data  = {} -- self.data[value_name] == value
   self.cache = {
      sparse  = false,
      lowest  = nil,
      highest = nil,
      count   = nil,
   }
   self.unused_ranges = {}
end

local instance_members = {}

enumeration = make_class({
   constructor      = constructor,
   instance_members = instance_members
})

do -- methods
   function instance_members:get(name)
      return self.data[name]
   end
   function instance_members:set(name, value)
      self.data[name] = value
      self.cache = {
         sparse  = false,
         lowest  = nil,
         highest = nil,
         count   = nil,
      }
   end
   function instance_members:mark_value_unused(value)
      local size = #self.unused_ranges
      for i = 1, size do
         local range = self.unused_ranges[i]
         if range.first == value + 1 then
            range.first = value
            if i > 1 then
               local prev = self.unused_ranges[i - 1]
               if prev.last >= range.first then
                  prev.last = range.last
                  table.remove(self.unused_ranges, i)
               end
            end
            return
         end
         if range.last == value - 1 then
            range.last = value
            if i < size then
               local nexx = self.unused_ranges[i + 1]
               if nexx.first <= range.last then
                  range.last = nexx.last
                  table.remove(self.unuesd_ranges, i + 1)
               end
            end
            return
         end
         if value >= range.first and value <= range.last then
            return
         end
      end
      self.unused_ranges[size + 1] = { first = value, last = value }
      if size > 0 then
         table.sort(self.unused_ranges, function(a, b)
            return a.first < b.first
         end)
      end
   end

   -- Returns an array of name/value pairs. If an enum value 
   -- has multiple names, only the first we encounter is used.
   function instance_members:to_sorted_pairs()
      local list = {}
      local seen = {}
      for k, v in pairs(self.data) do
         if not seen[v] then
            seen[v] = true
            local pair = { k, v }
            list[#list + 1] = pair
         end
      end
      table.sort(list, function(a, b)
         return a[2] < b[2]
      end)
      return list
   end
   
   function instance_members:update_cache()
      local count  = 0
      local lo     = nil
      local hi     = nil
      local values = {}
      for k, v in pairs(self.data) do
         if not values[v] then
            values[v] = true
            count = count + 1
         end
         if not hi or v > hi then
            hi = v
         end
         if not lo or v < lo then
            lo = v
         end
      end
      
      local cache = self.cache
      if not lo then
         cache.sparse = false
         return
      end
      cache.lowest  = lo
      cache.highest = hi
      if lo == hi + count - 1 then
         cache.sparse = true
      else
         cache.sparse = false
         for i = lo, hi do
            if not values[i] then
               cache.sparse = true
               break
            end
         end
      end
   end
   
   function instance_members:print()
      io.write(self.name)
      io.write(":\n")
      for k, v in pairs(self.data) do
         io.write("   ")
         io.write(k)
         io.write(" == ")
         io.write(tostring(v))
         io.write("\n")
      end
   end
end