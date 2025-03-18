class CIntegralTypeDefinition extends CTypeDefinition {
   constructor(format) {
      super(format);
      this.is_signed = null;
      this.options   = null; // Optional<CBitpackOptions>
      
      this.is_typedef_of = null; // Optional<CIntegralTypeDefinition>
      this.typedefs      = [];   // Array<CIntegralTypeDefinition>
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
      
      {
         let v = node.getAttribute("bitcount");
         if (v !== null) {
            this.options = new CBitpackOptions();
            this.options.from_xml(node, true, "integer", this.is_typedef_of?.options);
         }
      }
      
      for(let child of node.children) {
         if (child.nodeName == "typedef") {
            let name = child.getAttribute("name");
            if (name) {
               let td = new CIntegralTypeDefinition(this.save_format);
               td.is_typedef_of = this;
               td.is_signed     = this;
               Object.assign(td.c_info, this.c_info);
               td.begin_load(child);
               this.typedefs.push(td);
            }
         }
      }
   }
};