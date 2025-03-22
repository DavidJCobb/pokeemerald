import CDeclDefinition from "./c-decl-definition.js";
import CStructDefinition from "./c-struct-definition.js";
import CTypeInstance from "./c-type-instance.js";

export default class CStructInstance extends CTypeInstance {
   constructor(decl) {
      if (decl) {
         assert_type(decl instanceof CDeclDefinition);
         assert_logic(decl.serialized_type instanceof CStructDefinition);
      }
      super(decl);
      this.members = {}; // Array<CInstance>
      
      if (decl) {
         let type = decl.serialized_type;
         for(let /*CValue*/ src of type.members) {
            this.#rebuild_member(src);
         }
      }
   }
   
   copy_contents_of(/*const CStructInstance*/ other) {
      console.assert(other instanceof CStructInstance, "Can only copy the contents of a like instance.");
      for(let name in this.members) {
         let src = other.members[name];
         let dst = this.members[name];
         console.assert(!!src);
         dst.copy_contents_of(src);
      }
   }
   
   // Pass either a name or a decl.
   #rebuild_member(/*Variant<CDeclDefinition, String name>*/ decl) {
      assert_logic(!!this.type);
      if (decl+"" === decl) {
         let memb;
         for(let /*CDeclDefinition*/ src of this.type.members) {
            if (src.name == decl) {
               memb = src;
               break;
            }
         }
         assert_logic(!!memb);
         memb = decl;
      } else {
         assert_type(decl instanceof CDeclDefinition);
      }
      let dst = decl.make_instance_representation();
      dst.is_member_of = this;
      this.members[decl.name] = dst;
      if (decl.type == "union-external-tag") {
         let tag = this.members[decl.options.tag];
         dst.external_tag = tag;
         tag.is_tag_of_unions.push(dst);
      }
   }
};