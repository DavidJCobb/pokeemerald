require "classes"
require "xml"
require "stringify-table"

function run_test(text)
   local parser  = xml.parser()
   local success = true
   xpcall(
      function() parser:parse(text) end,
      function(err)
         local trace = debug.traceback()
         success = false
         print("Error: " .. tostring(err))
         print(trace)
         print("Already parsed:")
         print("   " .. parser._all_consumed_text)
         print("Remaining (" .. parser._remaining .. "):")
         print("   " .. parser._all_remaining_text)
         print_table(parser)
      end
   )
   if success then
      print("Parsed:")
      print("   " .. text)
      print("Result:")
      print_table(parser.root)
   else
      error("Halting all testing.")
   end
end

run_test("<data><test /><test name='foo' /><test name='bar'><child /><child /></test></data>")

run_test("<data><test />Test!</data>")

run_test("<data><test />&quot;Test!&quot;</data>")
run_test("<data><test name='&quot;Test!&quot;' /></data>")

run_test("<data><!-- <test /> --><uncommented /></data>")

run_test("<data><![CDATA[This is some <text>.]]></data>")