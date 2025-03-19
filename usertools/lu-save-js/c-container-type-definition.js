class CContainerTypeDefinition extends CTypeDefinition {
   constructor(format) {
      refuse_abstract_instantiation(CContainerTypeDefinition);
      super(format);
      this.members = []; // Array<CValue>
      
      this.categories  = []; // Array<String> // bitpacking categories
      this.annotations = []; // Array<String> // bitpacking misc annotations
   }
   
   begin_load(node) {
      super.begin_load(node);
      
      this.symbol = node.getAttribute("name");
      this.tag    = node.getAttribute("tag");
      
      this.c_info.alignment = node.getAttribute("c-alignment");
      this.c_info.size      = node.getAttribute("c-sizeof");
      if (this.c_info.alignment !== null)
         this.c_info.alignment = +this.c_info.alignment;
      if (this.c_info.size !== null)
         this.c_info.size = +this.c_info.size;
      
      for(let child of node.children) {
         if (child.nodeName == "category") {
            let name = child.getAttribute("name");
            if (name)
               this.categories.push(name);
         } else if (child.nodeName == "annotation") {
            let name = child.getAttribute("text");
            if (name)
               this.annotations.push(name);
         }
      }
   }
   
   /*Optional<CDeclDefinition>*/ member_by_name(name) /*const*/ {
      for(let memb of this.members)
         if (memb.name == name)
            return memb;
      return null;
   }
};