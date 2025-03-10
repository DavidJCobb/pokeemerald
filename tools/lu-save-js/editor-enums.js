const EDITOR_ENUMS = (function() {
   let out = new Map();
   
   function _from_array(name, array) {
      let values = new Map();
      for(let i = 0; i < array.length; ++i) {
         values.set(array[i], i);
      }
      out[name] = values;
   }
   
   out["LanguageID"] = (function() {
      let values = new Map();
      values.set("Japanese", 1);
      values.set("English",  2);
      values.set("French",   3);
      values.set("Italian",  4);
      values.set("German",   5);
      values.set("Korean",   6);
      values.set("Spanish",  7);
      return values;
   })();
   
   return out;
})();