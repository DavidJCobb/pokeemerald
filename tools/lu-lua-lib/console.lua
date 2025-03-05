-- Helper for formatted text output.
console = {
   stream = io.output()
}
do
   function _stringify_formatting(options)
      if not options then
         return "\x1b[0m"
      end
      local out = "\x1b["
      
      local empty = true
      function _write(s)
         if not empty then
            out = out .. ";"
         end
         out = out .. s
         empty = false
      end
      
      if options.reset then
         _write("0")
      end
      if options.bold then
         _write("1")
      end
      if options.back_color then
         _write(string.format(
            "48;2;%u;%u;%u",
            options.back_color.r or options.back_color[1] or 0,
            options.back_color.g or options.back_color[2] or 0,
            options.back_color.b or options.back_color[3] or 0
         ))
      end
      if options.text_color then
         _write(string.format(
            "38;2;%u;%u;%u",
            options.text_color.r or options.text_color[1] or 0,
            options.text_color.g or options.text_color[2] or 0,
            options.text_color.b or options.text_color[3] or 0
         ))
      end
      out = out .. "m"
      return out
   end

   function console:format(options)
      self:print(_stringify_formatting(options))
   end
   function console:print(text)
      self.stream:write(text)
   end
   
   local RED   = { 255,  64,  64 }
   local WHITE = { 240, 240, 240 }
   local BLUE  = {   0,   0, 128 }
   
   function _make_icon(glyph, back_color, glyph_color)
      local out = ""
      out = out .. _stringify_formatting({ text_color = back_color })
      out = out .. utf8.char(0x2590)
      out = out .. _stringify_formatting({ back_color = back_color, text_color = glyph_color })
      out = out .. glyph
      out = out .. _stringify_formatting({ reset = true, text_color = back_color })
      out = out .. utf8.char(0x258C)
      out = out .. _stringify_formatting()
      return out
   end
   
   local ERROR_ICON   = _make_icon("!", RED, WHITE)
   local ERROR_FORMAT = _stringify_formatting({ reset = true, text_color = RED })
   
   local STEP_ICON   = _make_icon("i", WHITE, BLUE)
   local STEP_FORMAT = _stringify_formatting({ reset = true, bold = true })
   
   local HIGHLIGHT_FORMAT = _stringify_formatting({ text_color = { 0, 255, 255 } })
   
   local RESET_FORMAT = _stringify_formatting()
   
   function console:report_error(text)
      self:print("\n")
      self:print(ERROR_ICON)
      self:print(ERROR_FORMAT)
      text = text:gsub("<h>",  HIGHLIGHT_FORMAT)
      text = text:gsub("</h>", ERROR_FORMAT)
      self:print(text)
      self:print(RESET_FORMAT)
      self:print("\n\n")
   end
   function console:report_fatal_error(text)
      self:report_error(text)
      os.exit(1)
   end
   function console:report_step(text)
      self:print("\n")
      self:print(STEP_ICON)
      self:print(STEP_FORMAT)
      text = text:gsub("<h>",  HIGHLIGHT_FORMAT)
      text = text:gsub("</h>", STEP_FORMAT)
      self:print(text)
      self:print(RESET_FORMAT)
      self:print("\n")
      -- Unlike errors, we don't print a second line break after because some 
      -- places may want to print additional information paired with this output.
   end
   
   function console:set_window_title(text)
      self.stream:write("\x1b]0;" .. text .. "\x1b\x5C")
   end
end