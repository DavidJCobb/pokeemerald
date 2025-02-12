
class CStruct {
   constructor() {
      this.node = null;
      
      this.symbol  = null; // typedef struct {} <name>
      this.tag     = null; // struct <name>
      this.members = [];
      
      // Defined if the struct has a whole-struct serialization function.
      this.instructions = null; // Optional<RootInstructionNode>
   }
   
   // `node` should be a `struct` element
   from_xml(node) {
      this.node   = node;
      this.symbol = node.getAttribute("name");
      this.tag    = node.getAttribute("tag");
      
      for(let child of node.children) {
         if (child.nodeName == "members") {
            this.members_from_xml(child);
         } else if (child.nodeName == "instructions") {
            this.instructions = new RootInstructionNode();
            this.instructions.from_xml(child);
         }
      }
   }
   members_from_xml(node) { // `node` should be a `members` element
      for(let item of node.children) {
         let member = new CValue();
         member.from_xml(item);
         this.members.push(member);
      }
   }
};

class CStructInstance {
   constructor(/*SaveFormat*/ format, /*CStruct*/ type) {
      this.save_format = format;
      this.type        = type; // CStruct
      this.members     = {};
      
      if (type) {
         for(let src of type.members) {
            let dst = new CValueInstance(format, src);
            this.members[src.name] = dst;
         }
      }
   }
};