class ChecksumRecalcHelper {
   static ChecksumInfo = class ChecksumInfo {
      static is_relevant_checksum(/*CInstance*/ inst) {
         if (!inst.is_member_of)
            return false;
         let decl = inst.decl;
         if (decl.bitfield_info || decl.array_extents.length)
            return false;
         {
            //
            // Currently, we require that the checksum be either the first 
            // or last member.
            //
            let type = inst.is_member_of.type;
            if (!type)
               return false;
            if (decl !== type.members[0])
               if (decl !== type.members[type.members.length - 1])
                  return false;
         }
         if (decl.type != "integer")
            return false;
         for(let name of decl.categories)
            if (name.match(/^checksum-(?:8|16|32|a(?:8|16|32)(?:u(?:8|16|32))?(?:r(?:8|16|32))?|crc\d+)$/))
               return true;
         return false;
      }
      
      constructor(/*CInstance*/ checksum_inst) {
         this.checksum  = checksum_inst;
         this.struct    = checksum_inst.is_member_of;
         this.is_suffix = false;
         this.params    = {
            accum:  null,
            unit:   null,
            result: null,
            crc:    null,
         };
         
         {
            let type = this.struct.type;
            assert_logic(!!type);
            if (checksum_inst.decl !== type.members[0]) {
               assert_logic(checksum_inst.decl === type.members[type.members.length - 1]);
               this.is_suffix = true;
            }
         }
         
         for(let name of checksum_inst.decl.categories) {
            if (!name.startsWith("checksum-"))
               continue;
            name = name.substring(("checksum-").length);
            {
               let v = +name;
               if (v) {
                  this.params.accum = this.params.unit = this.params.result = v;
                  break;
               }
            }
            if (name.startsWith("crc")) {
               let v = +name.substring(("crc").length);
               if (v) {
                  this.params.crc = v;
                  break;
               }
            }
            let match = name.match(/^a(\d+)(?:u(\d+))?(?:r(\d+))?$/);
            if (match) {
               let a = +match[1];
               this.params.accum  = a;
               this.params.unit   = +match[2] || a;
               this.params.result = +match[3] || a;
               break;
            }
         }
      }
      
      recalc(/*CBufferifier*/ bufferifier) {
         let checksum_info = bufferifier.watched_instances.get(this.checksum);
         let struct_info   = bufferifier.watched_instances.get(this.struct);
         assert_logic(!!checksum_info);
         assert_logic(!!struct_info);
         
         let view;
         if (this.is_suffix) {
            view = new DataView(
               bufferifier.buffer,
               struct_info.offset,
               checksum_info.offset - struct_info.offset
            );
         } else {
            view = new DataView(
               bufferifier.buffer,
               struct_info.offset + checksum_info.size,
               struct_info.size - checksum_info.size
            );
         }
         if (this.params.crc) {
            throw new Error("Not implemented!");
            return;
         }
         let value = checksum(
            this.params.accum,
            this.params.unit,
            this.params.result,
            view
         );
         this.checksum.value = value;
         for(let i = 0; i < checksum_info.size; ++i) {
            let byte = (value >> (i * 8)) & 0xFF;
            bufferifier.view.setUint8(checksum_info.offset + i, byte);
         }
      }
   };
   
   #subject;
   
   constructor() {
      this.bufferifier = new CBufferifier();
      this.#subject    = null; // CInstance
      this.checksums   = [];   // Array<ChecksumInfo>, sorted from innermost to outermost
   }
   
   get subject() { return this.#subject; }
   set subject(v) {
      assert_type(!v || v instanceof CInstance);
      this.bufferifier = new CBufferifier();
      if (!v) {
         this.#subject  = null;
         this.checksums = [];
         return;
      }
      this.#subject = v;
      this.bufferifier.watched_instances.set(v, null);
      
      let list = []; // Array<CInstance> of checksums, from innermost to outermost
      for(let inst = v.is_member_of; inst; inst = inst.is_member_of) {
         if (!inst.members)
            continue;
         let checksum = null;
         for(let name in inst.members) {
            let memb = inst.members[name];
            if (!ChecksumRecalcHelper.ChecksumInfo.is_relevant_checksum(memb, memb.decl))
               continue;
            checksum = memb;
            break;
         }
         if (!checksum)
            continue;
         list.push(checksum);
      }
      if (list.length) {
         let outermost_struct = list[list.length - 1].is_member_of;
         for(let item of list) {
            this.checksums.push(new ChecksumRecalcHelper.ChecksumInfo(item));
            this.bufferifier.watched_instances.set(item,              null);
            this.bufferifier.watched_instances.set(item.is_member_of, null);
         }
         this.bufferifier.run(outermost_struct);
      }
   }
   
   recalc() {
      if (!this.#subject || !this.checksums.length)
         return;
      assert_logic(this.bufferifier.watched_instances.has(this.#subject));
      {
         let info = this.bufferifier.watched_instances.get(this.#subject);
         assert_logic(!!info);
         if (info.is_bitfield) {
            let place = Math.floor(info.offset / 32) * 4;
            let shift = info.offset % 32;
            let mask  = (1 << info.size) - 1;
            if (info.size == 32) {
               mask = 0xFFFFFFFF;
            }
            let v = this.#subject.value;
            if (this.#subject.decl.type == "boolean")
               v = v ? 1 : 0;
            v = v << shift;
            
            let unit = this.bufferifier.view.getUint32(place, true);
            unit &= ~mask;
            unit |= v;
            this.bufferifier.view.setUint32(place, unit, true);
         } else {
            for(let i = 0; i < info.size; ++i) {
               let byte = (this.#subject.value >> (i * 8)) & 0xFF;
               this.bufferifier.view.setUint8(info.offset + i, byte);
            }
         }
      }
      for(let info of this.checksums)
         info.recalc(this.bufferifier);
   }
};