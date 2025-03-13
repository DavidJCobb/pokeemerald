
// Given an aggregate CInstance, construct a DataView that mimics how its 
// data would be laid out in memory, accounting for nested aggregates. 
// Goal is to be able to recompute checksums calculated in C code.
//
// Throws if:
//
//  - Transformed values are present anywhere in the aggregate. We can't 
//    know how the transformation is done, and so can't reconstruct the 
//    original C in-memory layout.
//
//  - Any (non-top-level) value or aggregate lacks size/offset information.
//
//  - Size/offset information is illegal (e.g.an  array of bitfields).
//
class CBufferifier {
   static InstanceInfo = class InstanceInfo {
      constructor(/*CInstance*/ inst) {
         assert_type(inst instanceof CInstance);
         this.subject     = inst;
         this.offset      = null; // bytes for non-bitfields; bits for bitfields
         this.size        = null; // bytes for non-bitfields; bits for bitfields
         this.is_bitfield = false;
      }
   };
   
   constructor() {
      this.buffer = null; // ArrayBuffer
      this.view   = null; // DataView
      this.offset = 0;    // measured in bits (has to be that way for bitfields)
      this.watched_instances = new Map(); // Map<CInstance, Optional<InstanceInfo>>
   }
   
   #get_type(inst) {
      let type;
      if (inst instanceof CDeclInstance) {
         if (inst.decl.type == "transform") {
            throw new Error("Cannot bufferify a transformed value, or any aggregate that contains one. We have no way of knowing the original in-memory representation.");
         }
         return inst.decl.c_types.serialized.definition;
      } else if (inst instanceof CTypeInstance) {
         return inst.type;
      }
      return null;
   }
   
   #align(inst) {
      if (inst instanceof CValueInstance) {
         const info = inst.decl.bitfield_info;
         if (info) {
            //
            // GCC (and clang) packs bitfields into DWORDs. If the 
            // current DWORD can't fit the current bitfield, then 
            // it gets pushed to the next DWORD.
            //
            let consumed = this.offset % 32;
            if (consumed + info.size > 32) {
               this.offset += 32 - consumed;
            }
            return;
         }
      }
      let type = this.#get_type(inst);
      if (type === null || type.c_info.alignment === null) {
         throw new Error("Unable to bufferify this entity. Unknown size and alignment.");
      }
      //
      // First, align to the next whole byte; then, to the number of 
      // bytes to which `inst` must be aligned.
      //
      if (this.offset % 8) {
         this.offset += 8 - (this.offset % 8);
      }
      let align = type.c_info.alignment;
      let diff  = (this.offset / 8) % align;
      if (diff) {
         this.offset += (align - diff) * 8;
      }
   }
   
   #advance_by_bits(b) {
      this.offset += b;
   }
   #advance_by_bytes(b) {
      this.offset += b * 8;
   }
   
   get byte_offset() { return Math.floor(this.offset / 8); }
   
   #handle_watched_instance(inst) {
      if (!this.watched_instances.has(inst))
         return;
      let info = new CBufferifier.InstanceInfo(inst);
      info.offset = this.byte_offset;
      info.size   = this.#get_type(inst).c_info.size;
      if (inst instanceof CValueInstance) {
         const bitfield_info = inst.decl.bitfield_info;
         if (bitfield_info) {
            info.is_bitfield = true;
            info.offset      = this.offset;
            info.size        = bitfield_info.size;
         }
      }
      this.watched_instances.set(inst, info);
   }
   
   #handle_instance(inst) {
      if (inst instanceof CValueInstance) {
         let type = inst.decl.type;
         if (type == "omitted") {
            return;
         }
         if (inst.value === null) {
            console.log(inst);
            throw new Error("Cannot bufferify a missing value.");
         }
         
         this.#align(inst);
         this.#handle_watched_instance(inst);
         
         const bitfield_info = inst.decl.bitfield_info;
         if (bitfield_info) {
            let v;
            if (type == "boolean") {
               v = inst.value ? 1 : 0;
            } else if (type == "integer") {
               v = +inst.value;
            } else {
               throw new Error("Only booleans and integers can be C bitfields.");
            }
            
            let place = Math.floor(this.offset / 32) * 4;
            let shift = bitfield_info.offset % 32;
            v = v << shift;
            
            let unit = this.view.getUint32(place, true);
            unit |= v;
            this.view.setUint32(place, unit, true);
            
            this.#advance_by_bits(bitfield_info.size);
            return;
         }
         let size = this.#get_type(inst).c_info.size;
         switch (type) {
            case "boolean":
               {
                  this.view.setUint8(this.offset, inst.value ? 1 : 0);
                  for(let i = 1; i < size; ++i)
                     this.view.setUint8(this.offset + i, 0);
               }
               this.#advance_by_bytes(size);
               break;
            case "buffer":
               {
                  let i;
                  let max = inst.decl.options.bytecount;
                  let end = Math.min(inst.value.byteLength, max);
                  for(i = 0; i < end; ++i)
                     this.view.setUint8(this.byte_offset + i, inst.value.getUint8(i));
                  for(; i < max; ++i)
                     this.view.setUint8(this.byte_offset + i, 0);
                  this.#advance_by_bytes(max);
               }
               break;
            case "integer":
            case "pointer":
               switch (size) {
                  case 1:
                  case 2:
                  case 4:
                     this.view[`setUint${size * 8}`](this.byte_offset, inst.value, true);
                     break;
                  default:
                     throw new Error("Unsupported integral size.");
               }
               this.#advance_by_bytes(size);
               break;
            case "string":
               {
                  let i;
                  let max = inst.decl.options.length;
                  let end = Math.min(inst.value.length, max);
                  for(i = 0; i < end; ++i)
                     this.view.setUint8(this.byte_offset + i, inst.value.bytes[i]);
                  for(; i < max; ++i)
                     this.view.setUint8(this.byte_offset + i, CHARMAP.string_terminator);
                  this.#advance_by_bytes(max);
                  if (inst.decl.options.needs_terminator)
                     this.#advance_by_bytes(1);
               }
               break;
            case "transform":
               throw new Error("Cannot bufferify a transformed value, or any aggregate that contains one. We have no way of knowing the original in-memory representation.");
            default:
               throw new Error("Unimplemented value type " + type + ".");
         }
         return;
      } else {
         this.#handle_aggregate(inst);
      }
   }
   
   #handle_aggregate(inst) {
      this.#align(inst);
      this.#handle_watched_instance(inst);
      
      if (inst instanceof CArrayInstance) {
         for(let i = 0; i < inst.values.length; ++i) {
            this.#handle_instance(inst.values[i]);
         }
      } else if (inst instanceof CStructInstance) {
         let offset = this.offset;
         for(let name in inst.members) {
            this.#handle_instance(inst.members[name]);
         }
         {
            let distance = this.offset - offset;
            if (distance + 4*8 < this.#get_type(inst).c_info.size)
               console.warn("Under-wrote the struct?", inst, distance);
         }
         this.offset = offset + this.#get_type(inst).c_info.size * 8;
      } else if (inst instanceof CUnionInstance) {
         if (inst.value) {
            let offset = this.offset;
            this.#handle_instance(inst.value);
            this.offset = offset + this.#get_type(inst).c_info.size * 8;
         } else {
            let size = inst.type.c_info.size;
            if (!size && size !== 0) {
               throw new Error("Unable to bufferify an empty union. Unknown size.");
            }
            this.offset += size * 8;
         }
      }
   }
   
   run(/*CInstance*/ inst) {
      let type;
      if (inst instanceof CDeclInstance) {
         if (inst.decl.bitfield_info) {
            throw new Error("Cannot bufferify a bitfield.");
         }
         if (inst.decl.type == "transform") {
            throw new Error("Cannot bufferify a transformed value, or any aggregate that contains one. We have no way of knowing the original in-memory representation.");
         }
         type = inst.c_types.serialized.definition;
      } else if (inst instanceof CTypeInstance) {
         type = inst.type;
      }
      if (!type.c_info.size) {
         throw new Error("Unable to bufferify this entity. Unknown size.");
      }
      
      this.buffer = new ArrayBuffer(type.c_info.size);
      this.view   = new DataView(this.buffer);
      this.offset = 0;
      this.#handle_instance(inst);
   }
};