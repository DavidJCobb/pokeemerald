
class CStruct extends CContainerTypeDefinition {
   constructor() {
      super();
      
      // Defined if the struct has a whole-struct serialization function.
      this.instructions = null; // Optional<RootInstructionNode>
   }
   
   // `node` should be a `struct` element
   from_xml(node) {
      super.from_xml(node);
      
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

class CStructInstance extends CTypeInstance {
   constructor(/*SaveFormat*/ format, /*CStruct*/ type, /*CValue*/ decl) {
      super(format, type, decl);
      this.members = {}; // Array<CInstance>
      
      if (type) {
         console.assert(type instanceof CStruct);
         for(let /*CValue*/ src of type.members) {
            this.rebuild_member(null, src);
         }
      }
   }
   
   copy_contents_of(/*const CStructInstance*/ other) {
      console.assert(other instanceof CStructInstance);
      for(let name in this.members) {
         let src = other.members[name];
         let dst = this.members[name];
         console.assert(!!src);
         dst.copy_contents_of(src);
      }
   }
   
   // Pass either a name or a decl.
   rebuild_member(/*Optional<String>*/ name, /*Optional<CValue>*/ decl) {
      console.assert(!!this.type);
      if (!decl) {
         for(let /*CValue*/ src of this.type.members) {
            if (src.name == name) {
               decl = src;
               break;
            }
         }
         console.assert(!!decl);
      } else {
         console.assert(!name || decl.name == name);
      }
      let dst = decl.make_instance_representation(this.save_format);
      dst.is_member_of = this;
      this.members[decl.name] = dst;
      if (decl.type == "union-external-tag") {
         dst.external_tag = this.members[decl.options.tag];
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