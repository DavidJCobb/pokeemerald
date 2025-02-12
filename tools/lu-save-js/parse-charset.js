function parse_charset(data) {
   let charset = {};
   
   data = data.split("\n");
   for(let line of data) {
      line = line.trim();
      if (line.startsWith("@"))
         continue;
      if (!line)
         continue;
      
      line = line.match(/^('='|'\s'|[^\s=]+)\s*=\s*([^\r\n]+)$/);
      if (!line)
         continue;
      let key = line[1];
      let val = line[2];
      
      if (key.startsWith("'"))
         key = key.charAt(1);
      
      {
         let i = val.indexOf("@");
         if (i >= 0)
            val = val.substring(0, i);
      }
      val = val.trim().split(" ").map(e => parseInt(e, 16));
      charset[key] = val;
   }
   return charset;
}