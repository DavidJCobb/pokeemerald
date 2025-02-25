
class CStruct extends CContainerTypeDefinition {
   constructor(format) {
      super(format);
      
      // Defined if the struct has a whole-struct serialization function.
      this.instructions = null; // Optional<RootInstructionNode>
   }
   
   // `node` should be a `struct` element
   from_xml(node) {
      super.from_xml(node);
      //
      // We don't load members here; we'll load them later on.
      //
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
            let member = new CValue(this.save_format);
            member.from_xml(item);
            this.members.push(member);
         }
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
};