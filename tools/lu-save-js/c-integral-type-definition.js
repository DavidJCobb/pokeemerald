class CIntegralTypeDefinition extends CTypeDefinition {
   constructor(format) {
      super(format);
      this.is_signed = null;
      
      this.typedefs = [];
   }
   
   begin_load(node) {
      super.begin_load(node);
      this.symbol = node.getAttribute("name");
      
      this.is_signed = node.getAttribute("is-signed") == "true";
      {
         let v = node.getAttribute("alignment");
         if (v !== null)
            this.c_info.alignment = +v;
      }
      {
         let v = node.getAttribute("size");
         if (v !== null)
            this.c_info.size = +v;
      }
      
      for(let child of node.children) {
         if (child.nodeName == "typedef") {
            let name = child.getAttribute("name");
            if (name)
               this.typedefs.push(name);
         }
      }
   }
};