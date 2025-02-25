
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
   constructor() {
      this.buffer = null;
      this.views  = []; // Array<DataView>
   }
   
   #get_metrics_of(inst) {
      if (inst instanceof CDeclInstance) {
         let info = inst.decl.field_info;
         if (!info)
            return null;
         let out = {
            is_bitfield: info.is_bitfield,
            offset:      info.offset,
            size:        info.size,
         };
         if (inst instanceof CValueInstanceArray) {
            if (out.is_bitfield) {
               throw new Error("Arrays of C bitfields are illegal.");
            }
            let extents = inst.decl.array_ranks;
            let count   = extents[0];
            for(let i = 1; i < inst.rank; ++i) {
               count *= extents[i];
            }
            out.offset += out.size * count;
            out.size   *= extents[inst.rank];
         }
         return out;
      } else if (inst instanceof CTypeInstance) {
         let sizeof = inst.type.c_info.size;
         let offset = inst.decl?.field_info?.offset;
         if (!sizeof) {
            throw new Error("Unable to ascertain the size of this object.");
         }
         return {
            is_bitfield: false,
            offset:      offset, // can be null/undefined e.g. if this is a top-level struct
            size:        sizeof,
         };
      }
      return null;
   }
   
   #handle_instance(inst, override_offset = null) {
      if (inst instanceof CValueInstance) {
         let info = inst.decl.field_info;
         if (!info) {
            //
            // TODO: This fails for top-level non-struct values e.g. ints or arrays thereof, 
            //       since the XML doesn't report this information for them.
            //
            throw new Error("Unable to ascertain the C size or offset of the data we wish to bufferify.");
         }
         if (override_offset || override_offset === 0) {
            info = Object.assign({ offset: override_offset }, info);
         }
         if (!this.buffer) {
            if (info.is_bitfield) {
               throw new Error("Cannot bufferify a lone C bitfield.");
            }
            this.buffer = new ArrayBuffer(info.size);
            this.views.push(new DataView(this.buffer));
         }
         //
         // Attempt to write into the buffer.
         //
         let view = this.views[this.views.length - 1];
         let type = inst.decl.type;
         if (type == "omitted") {
            return;
         }
         if (inst.value === null) {
            throw new Error("Cannot bufferify a missing value.");
         }
         if (info.is_bitfield) {
            let v;
            if (type == "boolean") {
               v = inst.value ? 1 : 0;
            } else if (type == "integer") {
               v = +inst.value;
            } else {
               throw new Error("Only booleans and integers can be C bitfields.");
            }
            
            // HACK: use Bitstream
            let stream = new Bitstream(view);
            stream.skip_bits(info.offset || 0);
            stream.write_unsigned(info.size, v);
            return;
         }
         switch (type) {
            case "boolean":
               {
                  view.setUint8(info.offset, inst.value ? 1 : 0);
                  for(let i = 1; i < info.size; ++i)
                     view.setUint8(info.offset + i, 0);
               }
               break;
            case "buffer":
               {
                  let i;
                  let end = Math.min(inst.value.byteLength, info.size);
                  for(i = 0; i < end; ++i)
                     view.setUint8(info.offset + i, inst.value.getUint8(i));
                  for(; i < info.size; ++i)
                     view.setUint8(info.offset + i, 0);
               }
               break;
            case "integer":
            case "pointer":
               switch (info.size) {
                  case 1:
                  case 2:
                  case 4:
                     view[`setUint${info.size * 8}`](info.offset, inst.value, true);
                     break;
                  default:
                     throw new Error("Unsupported integral size.");
               }
               break;
            case "string":
               {
                  let i;
                  let end = Math.min(inst.value.length, info.size);
                  for(i = 0; i < end; ++i)
                     view.setUint8(info.offset + i, inst.value.bytes[i]);
                  for(; i < info.size; ++i)
                     view.setUint8(info.offset + i, CHARSET_CONTROL_CODES.chars_to_bytes["\0"]);
               }
               break;
            default:
               throw new Error("Unimplemented value type " + type + ".");
         }
         return;
      } else {
         this.#handle_aggregate(inst);
      }
   }
   
   #handle_aggregate(inst) {
      let view;
      let metrics = this.#get_metrics_of(inst);
      if (!metrics || !metrics.size) {
         throw new Error("Unable to ascertain the C size of the data we wish to bufferify.");
      }
      if (metrics.is_bitfield) {
         throw new Error("Illegal C bitfield (entity is an aggregate).");
      }
      if (this.buffer) {
         if (!metrics.offset && metrics.offset !== 0) {
            throw new Error("Unable to ascertain the C offset of the data we wish to bufferify.");
         }
         let back = this.views[this.views.length - 1];
         view = new DataView(this.buffer, back.byteOffset + metrics.offset, metrics.size);
         this.views.push(view);
      } else {
         if (metrics.is_bitfield) {
            throw new Error("Cannot bufferify a bitfield.");
         }
         let offset = metrics.offset || 0;
         this.buffer = new ArrayBuffer(metrics.size + offset);
         view = new DataView(this.buffer, offset);
         this.views.push(view);
      }
      
      //
      // Handle the aggregate.
      //
      
      if (inst instanceof CValueInstanceArray) {
         for(let i = 0; i < inst.values.length; ++i) {
            let member = inst.values[i];
            let offset = metrics.offset + metrics.size * i;
            this.#handle_instance(member, offset, i);
         }
      } else if (inst instanceof CStructInstance) {
         for(let name in inst.members) {
            let memb = inst.members[name];
            this.#handle_instance(memb);
         }
      } else if (inst instanceof CUnionInstance) {
         if (inst.value) {
            this.#handle_instance(inst.value);
         }
      }
      
      this.views.pop();
   }
   
   run(/*CInstance*/ inst) {
      this.buffer = null;
      this.views  = [];
      this.#handle_instance(inst);
   }
};