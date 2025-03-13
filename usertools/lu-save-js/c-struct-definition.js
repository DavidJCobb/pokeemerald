class CStructDefinition extends CContainerTypeDefinition {
   constructor(format) {
      super(format);
      
      // Defined if the struct has a whole-struct serialization function.
      this.instructions = null; // Optional<RootInstructionNode>
   }
   
   begin_load(node) {
      super.begin_load(node);
      for(let child of node.children) {
         if (child.nodeName == "instructions") {
            this.instructions = new RootInstructionNode();
            this.instructions.from_xml(child);
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
         }
      }
   }
};