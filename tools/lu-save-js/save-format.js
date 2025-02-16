
class SaveSector {
   constructor() {
      this.node = null;
      this.instructions = null; // Optional<RootInstructionNode>
   }
   from_xml(node) {
      this.node = node;
      for(let child of node.children) {
         if (child.nodeName == "instructions") {
            this.instructions = new RootInstructionNode();
            this.instructions.from_xml(child);
         }
      }
   }
};

class SaveFormat {
   constructor() {
      this.sectors = []; // Array<SaveSector>
      this.structs = []; // Array<CStruct>
      this.unions  = []; // Array<CUnion>
      this.top_level_values = []; // Array<CValue>
   }
   
   // `node` should be the root `data` node
   from_xml(node) {
      node.querySelectorAll("c-types>struct").forEach((function(node) {
         let type = new CStruct();
         type.from_xml(node);
         this.structs.push(type);
      }).bind(this));
      node.querySelectorAll("c-types>union").forEach((function(node) {
         let type = new CUnion();
         type.from_xml(node);
         this.structs.push(type);
      }).bind(this));
      node.querySelectorAll("top-level-values>*").forEach((function(node) {
         let value = new CValue();
         value.from_xml(node);
         
         value.dereference_count    = +(node.getAttribute("dereference-count") || 0);
         value.force_to_next_sector = node.getAttribute("force-to-next-sector") == "true";
         
         this.top_level_values.push(value);
      }).bind(this));
      node.querySelectorAll("sectors>sector").forEach((function(node) {
         let sector = new SaveSector();
         sector.from_xml(node);
         this.sectors.push(sector);
      }).bind(this));
   }
   
   lookup_type_by_name(name) {
      for(let s of this.structs) {
         if (s.tag == name)
            return s;
         if (s.symbol == name)
            return s;
      }
      for(let s of this.unions) {
         if (s.tag == name)
            return s;
         if (s.symbol == name)
            return s;
      }
      return null;
   }
   
   #decompose_flash_sector(/*DataView*/ view) {
      const ENDIANNESS = true; // little-endian
      
      let out = {
         header: {
            version:   null,
            sector_id: null,
            checksum:  null,
            signature: null,
            counter:   null,
         },
         data: null,
      };
      
      out.data = new DataView(view.buffer, view.byteOffset, (0x1000 - 128));
      
      let offset = 0x1000 - 16;
      
      out.header.version = view.getUint32(offset, ENDIANNESS);
      offset += 4;
      
      out.header.sector_id = view.getUint16(offset, ENDIANNESS);
      offset += 2;
      
      out.header.checksum = view.getUint16(offset, ENDIANNESS);
      offset += 2;
      
      out.header.signature = view.getUint32(offset, ENDIANNESS);
      offset += 4;
      
      out.header.counter = view.getUint32(offset, ENDIANNESS);
      offset += 4;
      
      return out;
   }
   
   load(/*DataView*/ sav) {
      const SLOTS_PER_SAV    =  2;
      const SECTORS_PER_SLOT = 14;
      const SECTOR_SIGNATURE = 0x8012025;
      
      if (sav.byteLength < 0x1000 * 32) {
         throw new Error("The SAV file is truncated.");
      }
      
      let out = {
         slots: [],
         special_sectors: {
            hall_of_fame:    [ null, null ],
            trainer_hill:    null,
            recorded_battle: null,
         }
      };
      for(let i = 0; i < SLOTS_PER_SAV; ++i) {
         let slot = new CStructInstance(this, null);
         out.slots.push(slot);
         
         for(let tlv of this.top_level_values) {
            let value = tlv.make_instance_representation(this);
            slot.members[tlv.name] = value;
            slot.sectors = [];
         }
         
         for(let j = 0; j < SECTORS_PER_SLOT; ++j) {
            let pos  = 0x1000 * (SECTORS_PER_SLOT * i + j);
            let blob = new DataView(sav.buffer, pos, 0x1000);
            
            let flash_sector = this.#decompose_flash_sector(blob);
            let id = flash_sector.header.sector_id;
            if (flash_sector.header.signature == SECTOR_SIGNATURE) {
               console.assert(id < SECTORS_PER_SLOT);
            }
            
            slot.sectors.push(flash_sector.header);
            {
               let checksum = 0;
               for(let k = 0; k < 0x1000 - 128; k += 4) {
                  checksum += blob.getUint32(k, true);
                  checksum |= 0;
               }
               checksum = ((checksum >> 16) + checksum) & 0xFFFF;
               if (checksum == flash_sector.header.checksum)
                  flash_sector.header._checksum_is_valid = true;
               else
                  flash_sector.header._checksum_is_valid = false;
            }
            
            if (id < this.sectors.length && this.sectors[id].instructions) {
               let applier = new InstructionsApplier();
               applier.save_format = this;
               applier.root_data   = slot;
               applier.bitstream   = new Bitstream(blob);
               applier.apply(this.sectors[id].instructions);
            }
         }
      }
      {  // Special sectors
         out.special_sectors.hall_of_fame[0] = new DataView(sav.buffer, 0x1000*28, 0x1000);
         out.special_sectors.hall_of_fame[1] = new DataView(sav.buffer, 0x1000*29, 0x1000);
         out.special_sectors.trainer_hill = new DataView(sav.buffer, 0x1000*30, 0x1000);
         out.special_sectors.recorded_battle = new DataView(sav.buffer, 0x1000*30, 0x1000);
      }
      
      return out;
   }
};