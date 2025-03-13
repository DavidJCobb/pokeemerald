class CUnionDefinition extends CContainerTypeDefinition {
   constructor(/*SaveFormat*/ format) {
      super(format);
      
      this.internal_tag_name    = null; // Optional<String>
      this.members_by_tag_value = {};   // Map<int, CDeclDefinition>
   }
   
   // `node` should be a `union` element
   begin_load(node) {
      super.begin_load(node);
      for(let child of node.children) {
         switch (child.nodeName) {
            case "union-options":
               this.internal_tag_name = child.getAttribute("tag");
               break;
         }
      }
   }
   finish_load() {
      for(let child of this.node.children) {
         if (child.nodeName != "members")
            continue;
         for(let item of child.children) {
            let member = new CDeclDefinition(this.save_format);
            member.begin_load(item);
            member.finish_load();
            this.members.push(member);
            
            let tv = item.getAttribute("union-tag-value");
            if (tv !== null) {
               this.members_by_tag_value[tv] = member;
            }
         }
      }
   }
   
   /*Optional<CDeclDefinition>*/ member_by_tag_value(/*Variant<int, CValueInstance>*/ tag) {
      if (tag instanceof CValueInstance) {
         tag = tag.value;
      }
      tag = +tag;
      if (isNaN(tag))
         return null;
      return this.members_by_tag_value[tag] || null;
   }
};