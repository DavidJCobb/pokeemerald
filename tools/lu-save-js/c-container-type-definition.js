class CContainerTypeDefinition extends CTypeDefinition {
   constructor(format) {
      refuse_abstract_instantiation(CContainerTypeDefinition);
      super(format);
      this.members = []; // Array<CValue>
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
   }
   
   /*Optional<CDeclDefinition>*/ member_by_name(name) /*const*/ {
      for(let memb of this.members)
         if (memb.name == name)
            return memb;
      return null;
   }
};