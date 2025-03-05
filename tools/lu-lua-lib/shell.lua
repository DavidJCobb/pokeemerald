-- Helper for shell and syscalls.
shell = {
   cwd      = nil,
   cwd_real = nil,
}
do -- member functions
   function shell:cd(path)
      --
      -- We can't just use `os.execute` to run `cd` because that 
      -- spawns a separate shell/process. Instead, we have to 
      -- remember a preferred current working directory, so we 
      -- can prefix each command with a `cd` call as a one-liner.
      --
      if path then
         self.cwd      = nil
         self.cwd_real = self:run_and_read("readlink -f " .. path)
      else
         self.cwd_real = nil
      end
      self.cwd = path
   end
   
   function shell:resolve(path) -- resolve a relative path (e.g. ~/foo) to an absolute one
      local command = "readlink -f " .. path
      local stream  = assert(io.popen(command, "r"))
      local output  = stream:read("*all")
      stream:close()
      output = output:gsub("\n$", "")
      return output
   end
   
   function shell:exec(command) -- returns exit code, or nil if terminated
      if self.cwd then
         command = string.format("cd %s && %s", self.cwd, command)
      end
      local success, exit_type, exit_code = os.execute(command)
      if exit_type == "exit" then
         return exit_code
      end
      return nil
   end
   function shell:run(command) -- returns bool
      return self:exec(command) == 0
   end
   function shell:run_and_read(command) -- returns echo'd output
      if self.cwd then
         command = string.format("cd %s && %s", self.cwd, command)
      end
      local stream = assert(io.popen(command, "r"))
      local output = stream:read("*all")
      stream:close()
      output = output:gsub("\n$", "")
      return output
   end
   
   -- https://stackoverflow.com/a/40195356
   function shell:exists(name) -- file or folder
      return shell:run("stat " .. name .. " > /dev/null")
   end
   function shell:is_directory(path)
      return self:exists(path .. "/")
   end
   
   function shell:has_program(name) -- check if a program is installed
      return self:exec("command -v " .. name) == 0
   end
end