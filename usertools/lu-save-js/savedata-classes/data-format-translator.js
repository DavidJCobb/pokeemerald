import CInstance from "../c/c-instance.js";
import CArrayInstance from "../c/c-array-instance.js";
import CDeclInstance from "../c/c-decl-instance.js";
import CStructInstance from "../c/c-struct-instance.js";
import CUnionInstance from "../c/c-union-instance.js";
import CValueInstance from "../c/c-value-instance.js";

import PokeString from "../poke-string/poke-string.js";
import SaveSlot from "./save-slot.js";

//
// Base class for user-defined translators.
//
export class AbstractDataFormatTranslator {
   static RESULTS = {
      // This translator is opting not to handle a given destination value, 
      // preferring instead to defer to the default behavior (which may be 
      // to pass said value to another translator).
      PASS: Symbol("PASS"),
   };
   
   constructor() {
   }
   
   /*void*/ pass(/*CInstance*/ inst) {
      const PASS = AbstractDataFormatTranslator.RESULTS.PASS;
      if (inst instanceof CValueInstance) {
         if (inst.value === null) // don't clobber an already-filled value
            inst.value = PASS;
         return;
      }
      if (inst instanceof CArrayInstance) {
         for(let value of inst.values)
            this.pass(value);
         return;
      }
      if (inst instanceof CUnionInstance) {
         if (inst.value && inst.value !== PASS) {
            this.pass(inst.value);
         } else {
            console.assert(!inst.external_tag, "a CUnionInstance may have a null value during translation if it's internally tagged, but should never have one if it's externally tagged");
            inst.value = PASS;
         }
         return;
      }
      if (inst instanceof CStructInstance) {
         for(let name in inst.members) {
            let memb = inst.members[name];
            console.assert(memb instanceof CInstance);
            this.pass(memb);
         }
         return;
      }
      unreachable();
   }
   
   //
   // Extension points:
   //
   
   // Translate data from `src` into `dst`. If there are any fields within 
   // `dst` that you don't want to manually and fully translate, you must 
   // call `this.pass(field)` on them.
   //
   // These arguments are CInstances, so where the data is actually located 
   // will vary. If `src instanceof CValueInstance`, then you'd look in 
   // `src.value`, whereas if `src instanceof CStructInstance`, then you'd 
   // check each of `src.members`.
   //
   // Note also that a previously executed user-defined translator may 
   // already have filled in some or all data within `dst`.
   //
   /*void*/ translateInstance(/*const CInstance*/ src, /*CInstance*/ dst) {
      purecall();
   }
};

export class TranslationOperation {
   static TranslatorMapPair = class TranslatorMapPair {
      constructor() {
         this.src = new Map(); // Map<String typename, Array<AbstractDataFormatTranslator>>
         this.dst = new Map(); // Map<String typename, Array<AbstractDataFormatTranslator>>
      }
      add(which, criterion, translator) {
         const map = this[which];
         if (!map)
            throw new Error("You must specify whether this is a \"src\" or \"dst\" translator.");
         let list = map.get(criterion);
         if (!list) {
            map.set(criterion, list = []);
         }
         list.push(translator);
      }
   };
   
   constructor() {
      this.translators_by_typename          = new TranslationOperation.TranslatorMapPair();
      this.translators_for_top_level_values = new TranslationOperation.TranslatorMapPair();
   }
   
   #emplace_appropriate_externally_tagged_union_member(/*CUnionInstance*/ dst) {
      console.assert(dst instanceof CUnionInstance);
      if (!dst.external_tag)
         return;
      let decl = dst.type.member_by_tag_value(dst.external_tag);
      if (decl) {
         let v = dst.get_or_emplace(decl);
         return this.#default_init_dst(v);
      }
      throw new Error(`Failed to translate destination union ${dst.build_path_string()}: tag value ${dst.external_tag.build_path_string()} had value ${+tag} which does not correspond to any union member.`);
   }
   
   #should_zero_fill_if_new(/*CInstance*/ dst) {
      if (dst instanceof SaveSlot) { // SaveSlot subclasses CStructInstance as a dirty HACK, but lacks a `decl`
         return false;
      }
      for(let anno of dst.decl.annotations)
         if (anno == "zero-fill-if-new")
            return true;
      let type;
      if (dst instanceof CDeclInstance) {
         type = dst.type || dst.decl.serialized_type;
      } else if (dst instanceof CStructInstance || dst instanceof CUnionInstance) {
         type = dst.type;
      }
      if (type && type.annotations) {
         for(let anno of type.annotations)
            if (anno == "zero-fill-if-new")
               return true;
      }
      return false;
   }
   
   // Zero-fills `dst` (and any members/elements therein) that don't have 
   // default values. You should generally also call `#default_init_dst` 
   // after this.
   #zero_fill_new_inst(/*CInstance*/ dst) {
      if (dst instanceof CArrayInstance) {
         for(let v of dst.values)
            this.#zero_fill_new_inst(v);
      } else if (dst instanceof CValueInstance) {
         const PASS = AbstractDataFormatTranslator.RESULTS.PASS;
         if (dst.value !== null && dst.value !== PASS) {
            return;
         }
         if (dst.decl.default_value !== null) {
            return;
         }
         switch (dst.decl.type) {
            case "omitted":
               dst.value = null;
               break;
            case "boolean":
               dst.value = false;
               break;
            case "buffer":
               {
                  let buffer = new ArrayBuffer(dst.decl.options.bytecount);
                  dst.value = new DataView(buffer);
               }
               break;
            case "integer":
            case "pointer":
               dst.value = 0;
               break;
            case "string":
               {
                  dst.value = new PokeString();
                  for(let i = 0; i < dst.decl.options.length; ++i)
                     dst.value.bytes[i] = 0x00;
               }
               break;
         }
      } else if (dst instanceof CUnionInstance) {
         if (!dst.value) {
            if (dst.external_tag) {
               this.#emplace_appropriate_externally_tagged_union_member(dst);
            } else {
               let decl = dst.type.member_by_tag_value(0);
               if (!decl) {
                  throw new Error(`Failed to translate destination union ${dst.build_path_string()}: the union was marked as "zero-fill-if-empty", but it's internally tagged, and tag value 0 doesn't correspond to any of the union's elements.`);
               }
               dst.get_or_emplace(decl);
            }
         }
         this.#zero_fill_new_inst(dst.value);
      } else if (dst instanceof CStructInstance) {
         for(let name in dst.members) {
            this.#zero_fill_new_inst(dst.members[name]);
         }
      }
   }
   
   // Default-initialize all data in `dst` that we are capable of defaulting. 
   // Returns false if any data fails to initialize.
   //
   // Used for default translation operations, e.g. if a destination array is 
   // longer than the source array.
   /*bool*/ #default_init_dst(/*CInstance*/ dst) {
      if (dst instanceof CValueInstance) {
         if (dst.decl.type == "omitted") {
            return true; // skip omitted values
         }
         if (dst.value !== null) {
            return true;
         }
         return dst.set_to_default();
      }
      if (dst instanceof CArrayInstance) {
         let all_present = true;
         for(let value of dst.values) {
            if (!this.#default_init_dst(value))
               all_present = false;
         }
         return all_present;
      }
      if (dst instanceof CUnionInstance) {
         this.#emplace_appropriate_externally_tagged_union_member(dst);
      }
      if (dst instanceof CStructInstance) {
         let all_present = true;
         for(let name in dst.members) {
            let memb = dst.members[name];
            if (!this.#default_init_dst(memb))
               all_present = false;
         }
         return all_present;
      }
      console.assert(false, "unreachable");
   }
   
   // Recursively check whether a CInstance (and any members) has 
   // its value filled in.
   /*bool*/ #instance_is_filled(/*CInstance*/ inst, /*bool*/ allow_PASS) /*const*/ {
      console.assert(inst instanceof CInstance);
      
      const PASS = AbstractDataFormatTranslator.RESULTS.PASS;
      
      if (inst instanceof CValueInstance) {
         if (inst.decl.type == "omitted") {
            //
            // Don't really care about omitted values.
            //
            return true;
         }
         if (inst.value === PASS)
            return !!allow_PASS;
         return inst.value !== null;
      }
      if (inst instanceof CArrayInstance) {
         for(let value of inst.values)
            if (!this.#instance_is_filled(value, allow_PASS))
               return false;
         return true;
      }
      if (inst instanceof CStructInstance) {
         for(let name in inst.members) {
            let memb = inst.members[name];
            if (!this.#instance_is_filled(memb, allow_PASS))
               return false;
         }
         return true;
      }
      if (inst instanceof CUnionInstance) {
         if (inst.value === PASS) {
            return allow_PASS;
         }
         if (!inst.value) {
            return false;
         }
         return this.#instance_is_filled(inst.value, allow_PASS);
      }
      unreachable();
   }
   
   /*[[noreturn]]*/ #report_failure_to_translate(src, dst, dst_member) /*const*/ {
      if (!src) {
         throw new Error(`Failed to translate destination value ${dst.build_path_string()}: no value with the same path exists in the source format, and the destination value could not be fully defaulted.`);
      }
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
      
      function _member_is_intact(memb) {
         if (!(memb instanceof CInstance))
            return false;
         if (memb.is_member_of != dst)
            return false;
         return true;
      }
      
      if (dst instanceof CArrayInstance) {
         for(let i = 0; i < dst.values; ++i) {
            const dst_e = dst.values[i];
            if (!_member_is_intact(dst_e)) {
               throw new Error(`Failed to translate destination array ${dst.build_path_string()} element ${i} given source value ${src.build_path_string()}. The user-defined translator overwrote the destination CInstance with something else, instead of modifying the data in the CInstance.`);
            }
            if (!this.#instance_is_filled(dst_e, true)) {
               throw new Error(`Failed to translate destination array ${dst.build_path_string()} element ${i} given source value ${src.build_path_string()}.`);
            }
         }
         return true;
      }
      if (dst instanceof CStructInstance) {
         for(let member in dst.members) {
            let inst = dst.members[member];
            if (!_member_is_intact(inst)) {
               throw new Error(`Failed to translate destination value ${dst.build_path_string()}.${member} given source value ${src.build_path_string()}. The user-defined translator overwrote the destination CInstance with something else, instead of modifying the data in the CInstance.`);
            }
            if (!this.#instance_is_filled(inst, true)) {
               this.#report_failure_to_translate(src, dst, member);
            }
         }
         return true;
      }
      if (dst instanceof CUnionInstance) {
         if (dst.value === null) {
            this.#report_failure_to_translate(src, dst);
         }
         if (dst.value === PASS && dst.external_tag) {
            throw new Error(`Failed to translate destination union ${dst.build_path_string()} given source value ${src.build_path_string()}. The user-defined translator overwrote the CInstance for the destination union's value, instead of modifying the data in the CInstance.`);
         }
         if (dst.value !== PASS) {
            if (!_member_is_intact(dst.value)) {
               throw new Error(`Failed to translate destination union ${dst.build_path_string()} given source value ${src.build_path_string()}. The user-defined translator did not properly emplace the destination union's value.`);
            }
            if (!this.#instance_is_filled(dst.value, true)) {
               this.#report_failure_to_translate(src, dst);
            }
            //
            // Verify that custom translators didn't violate any invariants 
            // with respect to the union tag.
            //
            if (dst.external_tag) {
               let decl = dst.type.member_by_tag_value(dst.external_tag);
               if (!decl) {
                  throw new Error(`Failed to translate destination union ${dst.build_path_string()} given source value ${src.build_path_string()}. The destination union's external tag ${dst.external_tag.build_path_string()} has a value which doesn't match any of the union type's members; either an error internal to the translation system has occurred, or a user-defined translation improperly altered the value of the external tag.`);
               }
               if (dst.value.decl != decl) {
                  throw new Error(`Failed to translate destination union ${dst.build_path_string()} given source value ${src.build_path_string()}. The destination union's active member is ${dst.value.decl.name}, but based on the current value ${dst.external_tag.value} of its external tag ${dst.external_tag.build_path_string()}, the union's active member should be ${decl.name}. A user-defined translation improperly altered either the value of the external tag, or the union's active member, such that the two no longer match.`);
               }
            }
         }
         return true;
      }
      unreachable();
   }
   
   #clear_pass_state(/*CInstance*/ dst) {
      console.assert(dst instanceof CInstance);
      
      const PASS = AbstractDataFormatTranslator.RESULTS.PASS;
      
      if (dst instanceof CValueInstance) {
         if (dst.value === PASS)
            dst.value = null;
         return;
      }
      if (dst instanceof CArrayInstance) {
         for(let i = 0; i < dst.values.length; ++i)
            this.#clear_pass_state(dst.values[i]);
         return;
      }
      if (dst instanceof CStructInstance) {
         for(let name in dst.members) {
            let memb = dst.members[name];
            this.#clear_pass_state(memb);
         }
         return;
      }
      if (dst instanceof CUnionInstance) {
         let inst = dst.value;
         if (inst) {
            if (inst === PASS)
               dst.value = null;
            else
               this.#clear_pass_state(inst);
         }
         return;
      }
      unreachable();
   }
   
   #get_user_defined_translators_for(/*const CInstance*/ src, /*CInstance*/ dst) {
      if (src instanceof SaveSlot) {
         return [];
      }
      let list = [];
      
      function _gather(pair, func) {
         let key = (func)(src);
         if (key) {
            let subset = pair.src.get(key);
            if (subset)
               list = list.concat(subset);
         }
         key = (func)(dst);
         if (key) {
            let subset = pair.dst.get(key);
            if (subset)
               list = list.concat(subset);
         }
      }
      
      _gather(
         this.translators_by_typename,
         function(inst) {
            if (inst instanceof CDeclInstance)
               return inst.decl.c_types.serialized.name;
            else if (inst instanceof CTypeInstance)
               return inst.type.tag || inst.type.symbol;
         }
      );
      _gather(
         this.translators_for_top_level_values,
         function(inst) {
            if (!(inst.is_member_of instanceof SaveSlot))
               return;
            return inst.decl.name;
         }
      );
      
      return list;
   }
   
   #translate_impl(/*const CInstance*/ src, /*CInstance*/ dst) {
      if (dst instanceof CDeclInstance) {
         if (dst.decl.type == "omitted")
            return;
         if (dst instanceof CValueInstance && dst.value !== null) {
            //
            // Something already translated this value for us. Skip default 
            // processing.
            //
            return;
         }
      }
      if (dst instanceof CUnionInstance) {
         this.#emplace_appropriate_externally_tagged_union_member(dst);
      }
      
      let translators = this.#get_user_defined_translators_for(src, dst);
      for(let translator of translators) {
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
         this.#verify_successful_user_defined_translation(src, dst);
         //
         // Clear out any PASS values, both so they don't confuse us 
         // when we translate nested values, and so they aren't left 
         // in should we throw an error.
         //
         this.#clear_pass_state(dst);
      }
      //
      // User-defined translations have run. Handle any default translations, 
      // including recursing into aggregates to translate their members.
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
      if (!src) {
         let zero_fill = this.#should_zero_fill_if_new(dst);
         if (zero_fill) {
            this.#zero_fill_new_inst(dst);
         }
         if (!this.#default_init_dst(dst) && !zero_fill) {
            this.#report_failure_to_translate(src, dst);
         }
      } else if (src.constructor != dst.constructor) {
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
      } else if (dst instanceof CArrayInstance) {
         let zero_fill_dst = this.#should_zero_fill_if_new(dst);
         for(let i = 0; i < dst.values.length; ++i) {
            let src_e = src.values[i];
            let dst_e = dst.values[i];
            if (src_e === undefined) {
               //
               // The destination array is longer than the source array. Default 
               // the element if we can, and fail otherwise.
               //
               let zero_fill = zero_fill_dst || this.#should_zero_fill_if_new(dst_e);
               if (zero_fill) {
                  this.#zero_fill_new_inst(dst_e);
               }
               if (!this.#default_init_dst(dst_e) && !zero_fill) {
                  throw new Error(`Failed to translate destination value ${dst.build_path_string()} element ${i} given source value ${src.build_path_string()}. The source array is only length ${src.values.length}, and the destination value could not be fully defaulted.`);
               }
               continue;
            }
            this.#translate_impl(src_e, dst_e);
         }
         return;
      } else if (dst instanceof CUnionInstance) {
         if (!src.value) {
            //
            // The source union has no value to default-translate.
            //
            if (!dst.value || !this.#instance_is_filled(dst.value, false))
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
               
               let tag_name  = dst.type.internal_tag_name;
               let tag_value = src.value.members[tag_name];
               if (!tag_value || tag_value.value === null) {
                  //
                  // The source union's current value doesn't have a member with 
                  // the same name as the union tag, or it has such a member but 
                  // somehow the member isn't filled. Check if the destination-
                  // side union tag has a default value, and if so, use that.
                  //
                  let memb = dst.type.members[0].member_by_name(tag_name);
                  console.assert(!!memb);
                  if (memb.default_value === null || memb.default_value === undefined) {
                     //
                     // No default. Fail.
                     //
                     this.#report_failure_to_translate(src, dst);
                  }
                  tag_value = memb.default_value;
               } else {
                  tag_value = +tag_value.value;
               }
               dst.emplace(dst.type.members_by_tag_value[tag_value]);
               dst.value.members[tag_name].value = tag_value;
               //
               // Fall through, to translate the remaining members of the newly-
               // emplaced destination union member.
               //
            }
            //
            // Fall through.
            //
         }
         if (!src.value) {
            return;
         }
         this.#translate_impl(src.value, dst.value);
         return;
      } else if (dst instanceof CStructInstance) {
         let zero_fill_dst = this.#should_zero_fill_if_new(dst);
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
               let zero_fill = zero_fill_dst || this.#should_zero_fill_if_new(dst_e);
               if (zero_fill) {
                  this.#zero_fill_new_inst(dst_e);
               }
               if (!this.#default_init_dst(dst_e) && !zero_fill) {
                  throw new Error(`Failed to translate destination value ${dst_e.build_path_string()}. There was no corresponding value ${src.build_path_string()}.${member_name} to translate anything from.`);
               }
               continue;
            }
            this.#translate_impl(src_e, dst_e);
         }
         return;
      }
   }
   
   #default_value_translation(/*CValueInstance*/ src, /*CValueInstance*/ dst) {
      console.assert(src instanceof CValueInstance);
      console.assert(dst instanceof CValueInstance);
      if (src.decl.type == "omitted" || src.value === null || dst.decl.type == "omitted") {
         if (this.#default_init_dst(dst))
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
            break;
         
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
               {
                  let computed_dst = dst.decl.compute_integer_bounds();
                  if (src_value < computed_dst.min || src_value > computed_dst.max)
                     //
                     // The source value can't fit in the destination.
                     //
                     break;
               }
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
      
      if (dst.value === null) {
         this.#default_init_dst(dst);
      }
   }
   
   translate(/*const CInstance*/ src, /*CInstance*/ dst) {
      this.#translate_impl(src, dst);
      //
      // Verify that no type-mismatched values were improperly written in by a 
      // user-defined translator. Doing this check after translation means we 
      // can't report the problem ASAP after it happens, but it also means we 
      // don't have to recursively check everything that a translator could 
      // theoretically touch after each individual CInstance is handed to a 
      // translator.
      //
      let errors = [];
      function _typecheck(/*const CInstance*/ inst) {
         if (inst instanceof CStructInstance) {
            for(let name in inst.members)
               _typecheck(inst.members[name]);
            return;
         }
         if (inst instanceof CArrayInstance) {
            if (!inst.values) {
               errors.push(new Error(`Value ${inst.build_value_path()} is missing its values array.`));
               return;
            }
            if (!Array.isArray(inst.values)) {
               errors.push(new Error(`Value ${inst.build_value_path()} had its values array replaced with something that is not an array.`));
               return;
            }
            if (inst.values.length != inst.declared_length) {
               errors.push(new Error(`Value ${inst.build_value_path()} has ${inst.values.length} values in its values array, when there should be ${inst.declared_length} values.`));
               //
               // Fall through: check what values remain.
               //
            }
            for(let memb of inst.values)
               _typecheck(memb);
            return;
         }
         if (inst instanceof CUnionInstance) {
            if (inst.value)
               _typecheck(inst.value);
            return;
         }
         console.assert(inst instanceof CValueInstance);
         console.assert(!!inst.decl);
         let   valid = true;
         const value = inst.value;
         let   note  = null;
         switch (inst.decl.type) {
            case "omitted":
               break;
            case "boolean":
               valid = (value === true || value === false || +value === value);
               break;
            case "buffer":
               valid = value instanceof DataView;
               if (valid) {
                  if (value.byteLength > inst.decl.options.bytecount) {
                     valid = false;
                     note  = `the value is ${value.byteLength} bytes long, and cannot fit in a ${inst.decl.options.bytecount}-byte buffer`;
                  }
               }
               break;
            case "integer":
               valid = (value || value === 0) && !isNaN(+value);
               if (valid) {
                  let v = +value;
                  if (v < 0) {
                     let type = inst.decl.serialized_type;
                     if (type && type instanceof CIntegralTypeDefinition) {
                        if (!type.is_signed) {
                           //
                           // Simulate integer underflow for unsigned types.
                           //
                           let size = type.c_info?.size;
                           if (size == 4) {
                              v += 0xFFFFFFFF + 1;
                           } else if (size < 4) {
                              v += 1 << (size * 8);
                           }
                           inst.value = v;
                        }
                     }
                  }
                  let bounds = inst.decl.compute_integer_bounds();
                  if (v < bounds.min || v > bounds.max) {
                     valid = false;
                     note  = `the value lies outside the valid range [${bounds.min}, ${bounds.max}]`;
                  }
               }
               break;
            case "pointer":
               if (!value && value !== 0) {
                  valid = false;
               } else  {
                  let p = +value;
                  if (isNaN(p)) {
                     valid = false;
                  } else if (p > 0xFFFFFFFF) {
                     valid = false;
                     note  = "the value is outside the address range representable by a 32-bit pointer";
                  } else if (p < 0) {
                     //
                     // Correct potential problems that may arise from JS bitwise 
                     // operators, which coerce the result to an int32_t.
                     //
                     p += 0xFFFFFFFF + 1;
                     if (p < 0) {
                        //
                        // Value was too negative and isn't representable inside 
                        // of a uint32_t.
                        //
                        valid = false;
                        note  = "the value is outside the address range representable by a 32-bit pointer";
                     }
                  }
               }
               break;
            case "string":
               valid = value instanceof PokeString;
               if (valid) {
                  if (value.length > inst.decl.options.length) {
                     //
                     // TODO: If terminating nulls are what's causing us to overrun 
                     // the space limit, then just truncate them instead of erroring.
                     //
                     valid = false;
                     note  = `the string is ${value.length} bytes long, but only ${inst.decl.options.length} bytes are available with which to store it`;
                  }
               } else if (!valid && value+"" === value) {
                  note = "values intended to represent strings must be instances of PokeString, not bare string literals or native String objects";
               }
               break;
         }
         if (!valid) {
            let stringified = ""+value;
            if (!(value instanceof Object)) {
               try {
                  stringified = JSON.stringify(value);
               } catch (e) {}
            }
            
            let text = `Value ${inst.build_path_string()} was improperly set to ${stringified}.`;
            if (note) {
               text += ` (Note: ${note}.)`;
            } else {
               text += ` (Value is of the wrong type.)`;
            }
            errors.push(new Error(text));
         }
      }
      _typecheck(dst);
      if (errors.length) {
         throw new AggregateError(errors, "Failed to translate savedata to a different format; see associated errors.");
      }
   }
};