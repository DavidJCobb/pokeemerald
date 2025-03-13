
require "classes"
require "filesystem"
require "stringify-table"

do
   print("Preferred path separator: " .. filesystem.path.preferred_separator)
end

do
   local cwd
   local success = true
   xpcall(
      function()
         cwd = filesystem.current_path()
      end,
      function(err)
         print("Error: " .. tostring(err))
         print(debug.traceback())
         print_table(cwd)
         success = false
      end
   )
   if not success then
      error("Halting tests.")
   end
   print("CWD: " .. tostring(cwd))
   
   local subject = cwd:lexically_normal()
   print("CWD normalized: " .. tostring(subject))
   print("Is relative: " .. tostring(subject:is_relative()))
   print("CWD filename: " .. subject:filename())
   print("CWD extension: " .. subject:extension())
   print("CWD stem: " .. subject:stem())
   subject = subject:parent_path()
   print("...and parent: " .. tostring(subject))
   
   local rel = cwd:lexically_normal():lexically_relative(subject)
   print("Relative to parent: " .. tostring(rel))
   print(" - Is relative: " .. tostring(rel:is_relative()))
   
   do
      local test = filesystem.path(".gitignore")
      print("Test path: " .. tostring(test))
      print(" - Is relative: " .. tostring(test:is_relative()))
      print(" - Filename: " .. test:filename())
      print(" - Stem: " .. test:stem())
      print(" - Extension: " .. test:extension())
   end
   do
      local test = filesystem.path("hello.txt")
      print("Test path: " .. tostring(test))
      print(" - Is relative: " .. tostring(test:is_relative()))
      print(" - Filename: " .. test:filename())
      print(" - Stem: " .. test:stem())
      print(" - Extension: " .. test:extension())
   end
   
   filesystem.current_path("../../")
   do
      local test = filesystem.current_path()
      print("Test path: " .. tostring(test))
      print(" - Is relative: " .. tostring(test:is_relative()))
      print(" - Filename: " .. test:filename())
      print(" - Stem: " .. test:stem())
      print(" - Extension: " .. test:extension())
   end
   do
      local test = filesystem.current_path() / "lu-lua-lib"
      print("Test path: " .. tostring(test))
      print(" - Is relative: " .. tostring(test:is_relative()))
      print(" - Filename: " .. test:filename())
      print(" - Stem: " .. test:stem())
      print(" - Extension: " .. test:extension())
   end
end