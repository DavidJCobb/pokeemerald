import CArrayInstance from "./c-array-instance.js";
import CStructInstance from "./c-struct-instance.js";
import CUnionInstance from "./c-union-instance.js";

// For a loaded save slot.
export default class SaveSlot extends CStructInstance {
   #save_file;
   
   constructor(/*SaveFile*/ save_file) {
      //assert_type(save_file instanceof SaveFile);
      super(/*CStructDefinition*/ null);
      this.sectors    = [];
      this.#save_file = save_file;
      for(let tlv of this.save_format.top_level_values) {
         let value = tlv.make_instance_representation();
         value.is_member_of = this;
         this.members[tlv.name] = value;
      }
      for(let i = 0; i < SAVE_SECTORS_PER_SLOT; ++i) {
         this.sectors.push({
            version:   0xFFFFFFFF,
            sector_id: 0x0000,
            checksum:  0x0000,
            signature: 0xFFFFFFFF,
            counter:   0x00000000,
         });
      }
   }
   
   get save_file() {
      return this.#save_file;
   }
   get save_format() {
      return this.#save_file.save_format;
   }
   
   loadSectorMetadata(which, /*const DataView*/ sector_view) {
      console.assert(sector_view.byteLength == SAVE_SECTOR_FULL_SIZE);
      
      let offset = SAVE_SECTOR_FULL_SIZE - 16;
      let header = this.sectors[which];
      
      function read(bits) {
         let v = sector_view[`getUint${bits}`](offset, true);
         offset += bits / 8;
         return v;
      }
      header.version   = read(32);
      header.sector_id = read(16);
      header.checksum  = read(16);
      header.signature = read(32);
      header.counter   = read(32);
      
      let computed = checksumA32R16(sector_view, SAVE_SECTOR_DATA_SIZE);
      header.checksum_is_valid = computed == header.checksum;
      
      header.raw_sector = null;
      if (header.signature != SAVE_SECTOR_SIGNATURE) {
         //
         // HACK: Save the raw sector content for later export, so that 
         // round-tripped SAV files are byte-identical.
         //
         header.raw_sector = sector_view.buffer.slice(
            sector_view.byteOffset,
            sector_view.byteOffset + SAVE_SECTOR_FULL_SIZE
         );
      }
      
      return header;
   }
   
   get counter() {
      let counter = -1;
      for(let sector of this.sectors)
         if (sector.signature == SAVE_SECTOR_SIGNATURE)
            counter = Math.max(counter, sector.counter);
      return counter >= 0 ? counter : null;
   }
   get version() {
      let version = -1;
      for(let sector of this.sectors)
         if (sector.signature == SAVE_SECTOR_SIGNATURE)
            version = Math.max(version, sector.version);
      return version >= 0 ? version : null;
   }
   
   /*Optional<CInstance>*/ lookupCInstanceByPath(/*String*/ path) {
      if (!path)
         return;
      
      let value = this;
      let segm  = "";
      function _enter_segment() {
         if (!segm)
            return false;
         if (value instanceof CStructInstance) {
            value = value.members[segm];
            if (!value)
               return false;
         } else if (value instanceof CUnionInstance) {
            if (!value.value)
               return null;
            if (value.value.decl.name != segm)
               return null;
            value = value.value;
         } else {
            return false;
         }
         segm = "";
         return true;
      }
      
      let i = 0;
      {  // Handle leading segments of the form "(*foo)"
         let match = path.match(/^\((\**)([^\*\.\)]+)\)(->)?/);
         if (match) {
            i = match[0].length;
            
            let name  = match[2];
            let deref = match[1].length;
            if (match[3] == "->")
               ++deref;
            
            let memb = this.members[name];
            if (!memb)
               return null;
            let req = +memb.decl.dereference_count || 0;
            if (deref !== req)
               return null;
            value = memb;
         }
      }
      for(; i < path.length; ++i) {
         let c = path[i];
         if (c == '.') {
            let may_require_dereference = value === this;
            if (!_enter_segment())
               return null;
            if (may_require_dereference) {
               if (value.decl.dereference_count > 0)
                  return null;
            }
            continue;
         }
         if (c == "-" && path[i + 1] == ">") {
            if (value != this)
               return null;
            if (!_enter_segment())
               return null;
            if (value.decl.dereference_count != 1)
               return null;
            ++i; // since this token is two characters long, not just one
            continue;
         }
         if (c == "[") {
            if (segm) {
               if (!_enter_segment())
                  return null;
            }
            if (!value instanceof CArrayInstance)
               return null; // cannot index into a non-array aggregate
            let j = path.indexOf("]", i + 1);
            let k = path.indexOf(".", i + 1);
            if (k >= 0 && k < j)
               return null; // ill-formed path e.g. "abc[1.2]"
            if (j < 0)
               return null; // ill-formed path e.g. "abc["
            let index = path.substring(i + 1, j);
            if (index == "")
               return null; // ill-formed path e.g. "abc[]"
            index = +index;
            if (isNaN(index))
               return null; // ill-formed path e.g. "abc[foo]"
            if (index !== Math.floor(index) || index < 0)
               return null; // ill-formed path (not a positive or zero integer)
            value = value.values[index];
            if (!value)
               return null; // out-of-bounds index
            //
            // Skip to and past the "]".
            //
            i = j; // and we'll +1 as part of the loop
            if (path[i + 1] == '.') {
               ++i; // so "a[0].b" doesn't fail on '.', seeing nothing between ']' and '.'
            }
         } else {
            segm += c;
         }
      }
      if (segm)
         if (!_enter_segment())
            return null;
      
      return value;
   }
};