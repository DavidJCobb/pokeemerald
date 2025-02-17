
class CStruct {
   constructor() {
      this.node = null;
      
      this.symbol  = null; // typedef struct {} NAME
      this.tag     = null; // struct NAME
      this.members = [];   // Array<CValue>
      
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
      this.members     = {}; // Array<Variant<CStructInstance, CValueInstance, CValueInstanceArray, CUnionInstance>>
      
      if (type) {
         for(let /*CValue*/ src of type.members) {
            let dst = src.make_instance_representation(this.save_format);
            this.members[src.name] = dst;
            if (src.type == "union-external-tag") {
               dst.external_tag = this.members[src.options.tag];
            }
         }
      }
   }
   
   // Returns a list of any instance-objects that couldn't be filled in.
   fill_in_defaults() {
      let unfilled = [];
      for(let member of this.members) {
         if (member instanceof CValueInstance) {
            let dv = member.base?.default_value;
            if (dv === undefined) {
               unfilled.push(member);
            } else {
               member.value = dv;
            }
            continue;
         }
         unfilled = unfilled.concat(member.fill_in_defaults());
      }
      return unfilled;
   }
};