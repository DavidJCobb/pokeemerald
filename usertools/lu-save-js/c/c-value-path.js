import CArrayInstance from "./c-array-instance.js";
import CDeclDefinition from "./c-decl-definition.js";
import CInstance from "./c-instance.js";
import CStructInstance from "./c-struct-instance.js";
import CTypeDefinition from "./c-type-definition.js";
import CUnionInstance from "./c-union-instance.js";

export class CValuePathSegment {
   constructor() {
      this.type     = null; // null (root segment), "array-access", "member-access"
      this.what     = null; // member name; array element index; array index var name
      this.resolved = null; // Optional<Variant<CValue, CStruct, CUnion>>
   }
};

// Value paths for instruction nodes. These assert that the paths are 
// well-formed.
export class InstructionNodeValuePath {
   constructor() {
      this.segments = [];   // Array<CValuePathSegment>
      
      this.head_segment_dereference_count = 0;
   }
   
   parse(str) {
      // Syntax examples:
      //    a
      //    a.b
      //    a[0]
      //    a[__var]
      //    (*a).b
      //    (*a)[0]
      //    (*a)[__var]
      //    a.b.c
      //    a.b[0]
      //    a.b[__var]
      // And so on.
      
      if (!str)
         return;
      let i = 0;
      {
         let match = str.match(/^\((\**)([^\*\)]+)\)/);
         if (match) {
            this.head_segment_dereference_count = match[1].length;
            let segm = new CValuePathSegment();
            segm.type = null;
            segm.what = match[2];
            this.segments.push(segm);
            i = match[0].length;
         } else {
            match = str.match(/^([^\[\.]+)/);
            console.assert(!!match);
            let segm = new CValuePathSegment();
            segm.type = null;
            segm.what = match[1];
            this.segments.push(segm);
            i = match[0].length;
         }
      }
      while (i < str.length) {
         let c = str[i];
         if (c == '.') {
            ++i;
            let match = str.substring(i).match(/^([^\[\.]+)/);
            console.assert(!!match);
            let segm = new CValuePathSegment();
            segm.type = "member-access";
            segm.what = match[1];
            this.segments.push(segm);
            i += match[0].length;
            continue;
         }
         if (c == '[') {
            ++i;
            let match = str.substring(i).match(/^([^\]]+)/);
            console.assert(!!match);
            let segm = new CValuePathSegment();
            segm.type = "array-access";
            segm.what = match[1];
            if (!isNaN(+match[1]))
               segm.what = +segm.what;
            this.segments.push(segm);
            i += match[0].length;
            i += 1; // ']'
            continue;
         }
         console.assert(false, "unreachable", str, i, c);
      }
   }
};

// Value paths for use on loaded data. These just exist as a convenience.
export default class CValuePath {
   static ANY_ARRAY_ELEMENT = Symbol("ANY ARRAY ELEMENT");
   
   constructor(/*Optional<String>*/ path) {
      this.segments = [];
      this.initial_dereferences = 0;
      
      if (!path)
         return;
      let i = 0;
      {  // Handle leading segments of the form "(*foo)"
         let match = path.match(/^\((\**)([^\*\.\)]+)\)(->|.)?/);
         if (match) {
            i = match[0].length;
            
            let name  = match[2];
            let deref = match[1].length;
            if (match[3] == "->")
               ++deref;
            
            this.initial_dereferences = deref;
            this.segments.push(name);
         }
      }
      let segm = "";
      for(; i < path.length; ++i) {
         let c = path[i];
         if (c == '.') {
            if (!segm)
               throw new Error("Invalid path. (Unexpected '.' character.)");
            this.segments.push(segm);
            segm = "";
            continue;
         }
         if (c == '-' && path[i + 1] == '>') {
            if (this.segments.length > 0)
               throw new Error("Invalid path. (You can only dereference the first path element.)");
            if (!segm)
               throw new Error("Invalid path. (Unexpected \"->\" token.)");
            ++this.initial_dereferences;
            this.segments.push(segm);
            segm = "";
            ++i; // since the token is two characters long, not one
            continue;
         }
         if (c == '[') {
            if (segm) {
               this.segments.push(segm);
               segm = "";
            }
            let j = path.indexOf(']', i + 1);
            let k = path.indexOf('.', i + 1);
            if (k >= 0 && k < j)
               throw new Error("Invalid path. (Unexpected '.' character within an array index.)");
            if (j < 0)
               throw new Error("Invalid path. (Unterminated array index.)");
            let index = path.substring(i + 1, j);
            if (index === "")
               throw new Error("Invalid path. (Empty array index.)");
            if (index === '*') {
               this.segments.push(CValuePath.ANY_ARRAY_ELEMENT);
            } else {
               index = +index;
               if (isNaN(index))
                  throw new Error("Invalid path. (Non-integer array index.)");
               if (index !== Math.floor(index) || index < 0)
                  throw new Error("Invalid path. (Array index is negative or not an integer.)");
               this.segments.push(index);
            }
            
            i = j;
            if (path[i + 1] == '.') {
               ++i;
            }
            continue;
         }
         segm += c;
      }
      if (segm)
         this.segments.push(segm);
   }
   
   // Apply a given path to a given CInstance. The CInstance itself is not 
   // part of the path.
   /*Optional<CInstance>*/ apply(/*CInstance*/ base) {
      assert_type(!base || base instanceof CInstance);
      if (!base)
         return null;
      if (!this.segments.length)
         return null;
      let inst = base;
      for(let i = 0; i < this.segments.length; ++i) {
         const segm = this.segments[i];
         if (segm === CValuePath.ANY_ARRAY_ELEMENT) {
            if (!(inst instanceof CArrayInstance))
               return null;
            inst = inst.values[0];
            if (!inst)
               return null;
         } else if (segm === +segm) {
            if (!(inst instanceof CArrayInstance))
               return null;
            inst = inst.values[segm];
            if (!inst)
               return null;
         } else {
            if (inst instanceof CStructInstance) {
               inst = inst.members[segm];
            } else if (inst instanceof CUnionInstance) {
               let v = inst.value;
               if (v && v.decl.name == segm)
                  inst = v;
               else
                  return null;
            } else {
               return null;
            }
            if (i === 0 && inst && inst.decl) {
               if (this.initial_dereferences !== inst.decl.dereference_count)
                  return null;
            }
         }
      }
      return inst;
   }
   
   // Check whether a given CInstance is referred to by this path. Paths are 
   // relative to the CInstance's containing save slot (if any) by default; 
   // if you supply a `base`, then paths are relative to that base. Note that 
   // the base object itself is not part of the path.
   /*bool*/ matches(/*CInstance*/ inst, /*Variant<CInstance, Function(CInstance)>*/ base) {
      if (!base) {
         let slot = inst.save_slot;
         if (!slot)
            return null;
         return this.apply(slot) === inst;
      }
      let subject = inst;
      for(let i = this.segments.length - 1; i >= 0; --i) {
         let segm = this.segments[i];
         if (segm === CValuePath.ANY_ARRAY_ELEMENT) {
            let imo = subject.is_member_of;
            if (!imo)
               return false;
            if (!(imo instanceof CArrayInstance))
               return false;
         } else if (+segm === segm) {
            let imo = subject.is_member_of;
            if (!imo)
               return false;
            if (!(imo instanceof CArrayInstance))
               return false;
            if (subject !== imo.values[segm])
               return false;
         } else {
            if (subject.decl.name !== segm)
               return false;
         }
         subject = subject.is_member_of;
         if (!subject)
            return false;
      }
      if (base) {
         if (base instanceof CInstance) {
            return subject === base;
         } else if (base instanceof CDeclDefinition) {
            return subject.decl === base;
         } else if (base instanceof CTypeDefinition) {
            if (subject instanceof CTypeInstance) {
               return subject.type === base;
            } else {
               if (subject instanceof CArrayInstance)
                  return false;
               return subject.decl.c_types.serialized_type === base;
            }
         } else if (base instanceof Function) {
            return (base)(subject);
         }
      }
      return true;
      
   }
};