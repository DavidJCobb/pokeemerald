function parseBBCode(str) {
   str+="";
   
   let out   = [];
   let stack = [];
   
   const TEXT      = 0;
   const TAG_START = 1;
   const TAG_NAME  = 2;
   const TAG_DATA  = 3;
   const TAG_END   = 4;
   
   let text     = "";
   let where    = TEXT;
   let tag_name = "";
   let tag_data = "";
   
   function _commit_tag(name, data) {
      let parent = out;
      if (stack.length)
         parent = stack[stack.length - 1].children;
      
      name = name.toLowerCase();
      let tag = { name: name, data: data };
      if (text) {
         parent.push(text);
         text = "";
      }
      parent.push(tag);
      switch (name) {
         case "b":
         case "i":
         case "u":
         case "color":
         case "style":
            tag.children = [];
            stack.push(tag);
            break;
      }
      tag_name = "";
      tag_data = "";
   }
   function _close_tag(name) {
      tag_name = "";
      tag_data = "";
      if (!stack.length)
         return;
      let back = stack[stack.length - 1];
      if (back.name == name.toLowerCase()) {
         if (text) {
            back.children.push(text);
            text = "";
         }
         stack.pop();
      }
   }
   
   for(let i = 0; i < str.length; ++i) {
      let c = str[i];
      if (where == TAG_START) {
         if (c == '/') {
            where = TAG_END;
            continue;
         }
         where    = TAG_NAME;
         tag_name = c;
         continue;
      }
      switch (where) {
         case TAG_NAME:
            if (c == '=') {
               where = TAG_DATA;
               break;
            } else if (c == ']') {
               where = TEXT;
               _commit_tag(tag_name, null);
               break;
            }
            tag_name += c;
            break;
         case TAG_DATA:
            if (c == ']') {
               where = TEXT;
               _commit_tag(tag_name, tag_data);
               break;
            }
            tag_data += c;
            continue;
         case TAG_END:
            if (c == ']') {
               where = TEXT;
               _close_tag(tag_name);
               break;
            }
            tag_name += c;
            break;
         case TEXT:
            if (c == '[') {
               where = TAG_START;
               break;
            }
            text += c;
            break;
      }
   }
   if (text) {
      let parent = out;
      if (stack.length)
         parent = stack[stack.length - 1].children;
      parent.push(text);
   }
   return out;
}