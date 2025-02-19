//
// Base class for user-defined translators.
//
class AbstractDataFormatTranslator {
   static RESULTS = {
      // This translator is opting not to handle a given destination value, 
      // preferring instead to defer to the default behavior (which may be 
      // to pass said value to another translator).
      PASS: new Symbol("PASS"),
   };
   
   constructor() {
   }
   
   // convenience accessors for subclasses
   get PASS() { return DataFormatTranslator.RESULTS.PASS; }
   
   //
   // Extension points:
   //
   
   // Translate data from `src` into `dst`. If there are any fields within 
   // `dst` that you don't want to manually and fully translate, you must 
   // overwrite them with `this.PASS`.
   //
   // These arguments are CInstances, so where the data is actually located 
   // will vary. If `src instanceof CValueInstance`, then you'd look in 
   // `src.value`, whereas if `src instanceof CStructInstance`, then you'd 
   // check each of `src.members`.
   //
   // These arguments are not guaranteed to have the same constructor. All 
   // you can be sure of is that they exist at the value path: if `src` is 
   // `a.b[0][3].c` within its respective save file, then `dst` will be 
   // `a.b[0][3].c` within its own save file as well; but `src` may be a 
   // CValueInstance while `dst` is a CStructInstance, for example.
   //
   /*void*/ translateInstance(/*const CInstance*/ src, /*CInstance*/ dst) {
   }
};

class TranslationOperation {
   constructor() {
      this.translators_by_src_typename = {};
   }
   
   // Returns `false` if any members were not initialized.
   /*bool*/ #default_init_dst(/*CInstance*/ dst) {
      if (dst instanceof CValueInstance) {
         if (dst.decl.type == "omitted") {
            return true; // skip omitted values
         }
         let dv = dst.decl.default_value;
         if (dv !== null) {
            dst.value = dv;
            return true;
         }
         return false;
      }
      if (dst instanceof CValueInstanceArray) {
         let all_present = true;
         for(let value of dst.values) {
            if (!this.#default_init_dst(value))
               all_present = false;
         }
         return all_present;
      }
      if (dst instanceof CUnionInstance) {
         if (!dst.external_tag)
            return false;
         let tag = dst.external_tag.value;
         if (!tag && tag !== 0)
            return false;
         let decl = dst.type.members_by_tag_value[+tag];
         if (decl) {
            let v = dst.get_or_emplace(decl.name);
            return this.#default_init_dst(v);
         }
         throw new Error(`Failed to translate destination union ${dst.build_path_string()}: tag value ${dst.external_tag.build_path_string()} had value ${+tag} which does not correspond to any union member.`);
      }
      if (dst instanceof CStructInstance) {
         let all_present = true;
         for(let member of dst.members)
            if (!this.#default_init_dst(member))
               all_present = false;
         return all_present;
      }
      console.assert(false, "unreachable");
   }
   
   // Recursively check whether a CInstance (and any members) has 
   // its value filled in.
   /*bool*/ #instance_is_filled(/*CInstance*/ inst, /*bool*/ allow_PASS) /*const*/ {
      const PASS = AbstractDataFormatTranslator.RESULTS.PASS;
      if (inst === PASS)
         return allow_PASS || false;
      
      if (inst instanceof CValueInstance) {
         if (inst.decl.type == "omitted") {
            //
            // Don't really care about omitted values.
            //
            return true;
         }
         if (allow_PASS) {
            if (inst.value === PASS)
               return true;
         }
         return inst.value !== null && inst.value !== PASS;
      }
      if (inst instanceof CValueInstanceArray) {
         for(let value of inst.values)
            if (!this.#instance_is_filled(value, allow_PASS))
               return false;
         return true;
      }
      if (inst instanceof CStructInstance) {
         for(let member of inst.members)
            if (!this.#instance_is_filled(member, allow_PASS))
               return false;
         return true;
      }
      if (inst instanceof CUnionInstance) {
         return this.#instance_is_filled(inst.value, allow_PASS);
      }
      console.assert(false, "unreachable");
   }
   
   /*[[noreturn]]*/ #report_failure_to_translate(src, dst, dst_member) /*const*/ {
      if (dst_member) {
         throw new Error(`Failed to translate destination value ${dst.build_path_string()} member ${dst_member} given source value ${src.build_path_string()}.`);
      }
      throw new Error(`Failed to translate destination value ${dst.build_path_string()} given source value ${src.build_path_string()}.`);
   }
   
   #verify_successful_user_defined_translation(/*CInstance*/ src, /*CInstance*/ dst, /*Optional<CInstance>*/ dst_active_union_member_decl) {
      if (dst instanceof CValueInstance) {
         if (dst.value === null) {
            this.#report_failure_to_translate(src, dst);
         }
         return true;
      }
      if (dst instanceof CValueInstance) {
         for(let i = 0; i < dst.values; ++i) {
            const dst_e = dst.values[i];
            if (!this.#instance_is_filled(dst_e, true)) {
               throw new Error(`Failed to translate destination array ${dst.build_path_string()} element ${i} given source value ${src.build_path_string()}.`);
            }
         }
         return true;
      }
      if (dst instanceof CStructInstance) {
         for(let member in dst.members) {
            let inst = dst.members[member];
            if (!this.#instance_is_filled(inst, true)) {
               this.#report_failure_tO_translate(src, dst, member);
            }
         }
         return true;
      }
      if (dst instanceof CUnionInstance) {
         if (!this.#instance_is_filled(dst.value, true)) {
            this.#report_failure_to_translate(src, dst);
         }
         if (dst.value !== PASS) {
            //
            // Verify that custom translators didn't violate any invariants 
            // with respect to the union tag.
            //
            if (dst.external_tag) {
               console.assert(!!dst_active_union_member_decl, "we should've successfully defaulted the union based on its external tag, so we should know what CValue we defaulted it to");
               if (dst.value.decl != dst_active_union_member_decl) {
                  throw new Error(`Failed to translate destination union ${dst.build_path_string()}: tag value ${dst.external_tag.build_path_string()} had value ${+tag} corresponding to union member ${dst_active_union_member_decl.name}, but a user-defined translation improperly changed the union to member ${dst.value.decl.name}, contradicting the tag.`);
               }
            }
         }
         return true;
      }
      return false;
   }
   
   #clear_pass_state(/*CInstance*/ dst) {
      const PASS = AbstractDataFormatTranslator.RESULTS.PASS;
      
      if (dst instanceof CValueInstance) {
         if (dst.value === PASS)
            dst.value = null;
         return;
      }
      if (dst instanceof CValueInstanceArray) {
         for(let i = 0; i < dst.values.length; ++i) {
            if (dst.values[i] === PASS) {
               //
               // A user-defined translator chose to use the default translation 
               // for this element. Fair enough, but in making that choice, it 
               // clobbered the CInstance for the element. Rebuild it.
               //
               dst.rebuild_element(i);
            } else {
               this.#clear_pass_state(dst.values[i]);
            }
         }
         return;
      }
      if (dst instanceof CStructInstance) {
         for(let name in dst.members) {
            let inst = dst.members[name];
            if (inst === PASS) {
               //
               // A user-defined translator chose to use the default translation 
               // for this member. Fair enough, but in making that choice, it 
               // clobbered the CInstance for the member. Rebuild it.
               //
               inst = dst.rebuild_member(name);
            } else {
               this.#clear_pass_state(inst);
            }
         }
         return;
      }
      if (dst instanceof CUnionInstance) {
         let inst = dst.value;
         if (inst === PASS) {
            //
            // A user-defined translator chose to use the default translation for 
            // the union's active member. Fair enough. Can we rebuild said member?
            //
            dst.value = null;
            if (dst.external_tag) {
               this.#default_init_dst(dst);
               console.assert(!!dst.value);
            }
         } else if (inst) {
            this.#clear_pass_state(inst);
         }
         return;
      }
   }
   
   translate(/*CInstance*/ src, /*CInstance*/ dst) {
      const PASS = AbstractDataFormatTranslator.RESULTS.PASS;
      
      if (dst instanceof CDeclInstance) {
         if (dst.decl.type == "omitted")
            return;
         if (dst instanceof CValueInstance && dst.value !== null && dst.value !== PASS) {
            //
            // Something already translated this value for us. Skip default 
            // processing.
            //
            return;
         }
      }
      this.#default_init_dst(dst);
      
      let src_typename;
      if (src instanceof CDeclInstance) {
         src_typename = src.decl.c_typenames.serialized;
      } else if (src instanceof CTypeInstance) {
         src_typename = src.type.tag || src.type.symbol;
      }
      if (src_typename) {
         let translator = this.translators_by_src_typename[src_typename];
         if (translator) {
            let union_member_decl;
            if (dst instanceof CUnionInstance) {
               union_member_decl = dst.value?.decl;
            }
            
            translator.translateInstance(src, dst);
            //
            // The user-defined translator has run.
            //
            // Verify that the user-defined translator actually filled in 
            // all fields in the destination (either with full values or 
            // by intentionally PASSing on them), and error if it didn't. 
            // Also error if it violated any other invariants (e.g. if it 
            // emplaced a union member contrary to the value of the union 
            // tag CInstance).
            //
            this.#verify_successful_user_defined_translation(src, dst, union_member_decl);
            //
            // Clear out any PASS values, both so they don't confuse us 
            // when we translate nested values, and so they aren't left 
            // in should we throw an error.
            //
            this.#clear_pass_state(dst);
         }
      }
      //
      // User-defined translations have run. Handle any 
      // default translations, including recursing into 
      // aggregates to translate their members.
      //
      if (dst instanceof CValueInstance && dst.decl.type == "omitted") {
         return;
      }
      if (this.#instance_is_filled(dst)) {
         //
         // Don't clobber anything that's been wholly filled in by a 
         // user-defined translator.
         //
         return;
      }
      if (src.constructor != dst.constructor) {
         //
         // We don't currently support interconverting across types as 
         // a default translation; for example, there's no way to turn 
         // a struct into a union or vice versa.
         //
         this.#report_failure_to_translate(src, dst);
      }
      //
      // Specific logic for default translations, by CInstance class:
      //
      if (dst instanceof CValueInstance) {
         if (dst.decl.type == "omitted") {
            //
            // Don't really care about omitted values.
            //
            return;
         }
         this.#default_value_translation(src, dst);
         if (dst.value === null) {
            this.#report_failure_to_translate(src, dst);
         }
         return;
      } else if (dst instanceof CValueInstanceArray) {
         for(let i = 0; i < dst.values.length; ++i) {
            let src_e = src.values[i];
            let dst_e = dst.values[i];
            if (src_e === undefined) {
               //
               // The destination array is longer than the source array. Default 
               // the element if we can, and fail otherwise.
               //
               if (!this.#default_init_dst(dst_e)) {
                  throw new Error(`Failed to translate destination value ${dst.build_path_string()} element ${i} given source value ${src.build_path_string()}. The source array is only length ${src.values.length}, and the destination value could not be fully defaulted.`);
               }
               continue;
            }
            this.translate(src_e, dst_e);
         }
         return;
      } else if (dst instanceof CUnionInstance) {
         if (!src.value) {
            //
            // The source union has no value to default-translate.
            //
            this.#report_failure_to_translate(src, dst);
         }
         if (dst.external_tag) {
            console.assert(!!dst.value, "the destination union should have a value: we default-create it before running a translator, and the translator should either properly update it or set it to PASS, which we then default-create again in #clear_pass_state");
         } else {
            //
            // `dst` is an internally tagged union.
            //
            if (!dst.value) {
               //
               // The destination union has no active union member. We need to 
               // see if we can emplace it based on the source union.
               //
               // Let's translate over the common members-of-members within the 
               // destination union; then, once we have the tag, we'll know what 
               // type to emplace.
               //
               console.assert(dst.type.members.length > 0);
               
               let tag_name = dst.type.internal_tag_name;
               dst.emplace(dst.type.members[0].name);
               for(let name in dst.value.members) {
                  let src_inst = src.value.members[name];
                  let dst_inst = dst.value.members[name];
                  if (!src_inst) {
                     throw new Error(`Failed to translate destination value ${dst.build_path_string()}.<any>.${name} given source union ${src.build_path_string()}. The source union's current value does not have a member named ${name}.`);
                  }
                  this.translate(src_inst, dst_inst);
                  if (name == tag_name)
                     break;
               }
               let tag_value = dst.value.members[tag_name].value;
               console.assert(tag_value !== null);
               dst.get_or_emplace(dst.type.members_by_tag_value[+tag_value]);
               //
               // Fall through, to translate the remaining members of the newly-
               // emplaced destination union member.
               //
            }
            //
            // Fall through.
            //
         }
         this.translate(src.value, dst.value);
         return;
      } else if (dst instanceof CStructInstance) {
         for(let member_name in dst.members) {
            let src_e = src.members[member_name];
            let dst_e = dst.members[member_name];
            if (this.#instance_is_filled(dst_e)) {
               //
               // This destination element was already filled by a user-defined 
               // translator. Don't clobber the filled-in value with a default.
               //
               continue;
            }
            if (!src_e) {
               //
               // `dst_e` represents a newly-added member in this type, not 
               // present in the source save format.
               //
               if (!this.#default_init_dst(dst_e)) {
                  throw new Error(`Failed to translate destination value ${dst_e.build_path_string()}. There was no corresponding value ${src.build_path_string()}.${member_name} to translate anything from.`);
               }
               continue;
            }
            this.translate(src_e, dst_e);
         }
         return;
      }
   }
   
   #default_value_translation(/*CValueInstance*/ src, /*CValueInstance*/ dst) {
      console.assert(src instanceof CValueInstance);
      console.assert(dst instanceof CValueInstance);
      if (src.decl.type == "omitted" || src.value === null || dst.decl.type == "omitted") {
         return;
      }
      const src_options = src.decl.options;
      const dst_options = dst.decl.options;
      switch (dst.decl.type) {
         // Allow implicit conversion between booleans and integers.
         case "boolean":
            {
               let src_value = null;
               switch (src.decl.type) {
                  case "boolean":
                     src_value = src.value;
                     break;
                  case "integer":
                     if (src_options.min > 0 || src_options.max < 0)
                        //
                        // We can only meaningfully coerce to a bool if 
                        // zero and a non-zero value are representable 
                        // in this integer.
                        //
                        break;
                     src_value = src.value != 0;
                     break;
               }
               if (src_value === null)
                  break;
               dst.value = src_value;
            }
         
         // Allow implicit conversion between booleans and integers.
         case "integer":
            {
               let src_value = null;
               switch (src.decl.type) {
                  case "boolean": src_value = src.value ? 1 : 0; break;
                  case "integer": src_value = src.value; break;
               }
               if (src_value === null)
                  break;
               //
               // TODO: Validate that the value can fit!
               //
               dst.value = src_value;
            }
            break;
            
         // Allow implicit conversion between buffers and strings.
         case "buffer":
            {
               let read_func = null;
               let src_size  = 0;
               switch (src.decl.type) {
                  case "buffer":
                     read_func = src.value.getUint8.bind(src.value);
                     src_size  = src.value.byteLength;
                     break;
                  case "string":
                     read_func = function(i) { return src.value.bytes[i]; };
                     src_size  = src.length;
                     break;
               }
               if (read_func === null)
                  break;
               if (src_size > dst_options.bytecount)
                  break;
               let buffer = new ArrayBuffer(dst_options.bytecount);
               dst.value = new DataView(buffer);
               for(let i = 0; i < src_size; ++i)
                  dst.value.setUint8(i, read_func(i));
            }
            break;
            
         // Allow implicit conversion between buffers and strings.
         case "string":
            {
               let src_bytes = [];
               switch (src.decl.type) {
                  case "buffer":
                     for(let i = 0; i < src.value.byteLength; ++i)
                        src_bytes.push(src.value.getUint8(i));
                     break;
                  case "string":
                     src_bytes = src.value.bytes;
                     break;
                  default:
                     src_bytes = null;
                     break;
               }
               if (src_bytes === null)
                  break;
               if (src_bytes.length > dst_options.max_length)
                  break;
               dst.value = new PokeString();
               dst.value.bytes = src_bytes;
            }
            break;
            
         case "pointer":
            if (src.decl.type != dst.decl.type)
               break;
            dst.value = src.value;
            break;
      }
   }
};