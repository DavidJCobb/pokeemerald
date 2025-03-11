require "classes"
require "stringify-table"
require "json"

function test_encode(data)
   local text
   local success = true
   xpcall(
      function()
         text = json.to(data, { pretty_print = true, indent = 3 })
      end,
      function(err)
         local trace = debug.traceback()
         success = false
         print("Error: " .. tostring(err))
         print(trace)
      end
   )
   if success then
      print("Source:")
      if type(data) == "table" then
         print_table(data, "   ")
      else
         print("   " .. tostring(data))
      end
      print("Result:")
      print("   " .. text)
      print("")
   else
      error("Halting all testing.")
   end
end

test_encode(nil)
test_encode(false)
test_encode(true)
test_encode(0)
test_encode("foo")
test_encode({ bar = 5 })
test_encode({ baz = { hello = "world" } })
test_encode({ "A", "B", "C", "D", "E" })
test_encode({ "A", "B", {"C", "D", "E"}, "F", "G" })

print("")
print("TESTING DECODE")
print("")

function test_decode(text)
   local data
   local success = true
   xpcall(
      function()
         data = json.from(text)
      end,
      function(err)
         local trace = debug.traceback()
         success = false
         print("Error: " .. tostring(err))
         print(trace)
      end
   )
   if success then
      print("Source:")
      print("   " .. text)
      print("Result:")
      if type(data) == "table" then
         print_table(data, "   ")
      else
         print("   " .. tostring(data))
      end
      print("")
   else
      error("Halting all testing.")
   end
end

test_decode("null")
test_decode("false")
test_decode("true")
test_decode("5")
test_decode("-5")
test_decode("-5.5")
test_decode("-5.5e5")
test_decode('"hello, world!"')
test_decode("{}")
test_decode('{"a":5}')
test_decode('{ "a" : 5 }') -- same as previous, but with spaces
test_decode('{"a":1,"b":2}')
test_decode('{ "a" : 1 , "b" : 2 }') -- same as previous, but with spaces
test_decode('[ 1, 2, 3, 4, 5 ]')
test_decode('[ "a", "b", "c" ]')
test_decode('[ "a", [ "b" ], "c" ]')
test_decode(' [ "a" , [ "b" ] , "c" ] ') -- same as previous, but with more spaces