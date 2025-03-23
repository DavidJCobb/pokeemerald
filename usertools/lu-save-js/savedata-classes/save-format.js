import Bitstream from "../bitstream.js";
import CDeclDefinition from "../c/c-decl-definition.js";
import CIntegralTypeDefinition from "../c/c-integral-type-definition.js";
import CStructDefinition from "../c/c-struct-definition.js";
import CUnionDefinition from "../c/c-union-definition.js";
import InstructionsApplier from "../c/instructions-applier.js";
import { RootInstructionNode } from "../c/instructions.js";
import SaveFile from "./save-file.js";

export class SaveSector {
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

export default class SaveFormat {
   constructor() {
      this.enums   = new Map(); // Map<String, Map<String, int>>
      this.sectors = []; // Array<SaveSector>
      this.types   = {
         integrals: [], // Array<CIntegralTypeDefinition>
         structs:   [], // Array<CStructDefinition>
         unions:    [], // Array<CUnionDefinition>
      };
      this.top_level_values = []; // Array<CDeclDefinition>
      
      this.display_overrides = []; // Array<CInstanceDisplayOverrides>
   }
   
   // `node` should be the root `data` node
   from_xml(node) {
      node.querySelectorAll("c-types>integral").forEach((function(node) {
         let type = new CIntegralTypeDefinition(this);
         type.begin_load(node);
         this.types.integrals.push(type);
         for(let td of type.typedefs) {
            this.types.integrals.push(td);
         }
      }).bind(this));
      node.querySelectorAll("c-types>struct").forEach((function(node) {
         let type = new CStructDefinition(this);
         type.begin_load(node);
         this.types.structs.push(type);
      }).bind(this));
      node.querySelectorAll("c-types>union").forEach((function(node) {
         let type = new CUnionDefinition(this);
         type.begin_load(node);
         this.types.unions.push(type);
      }).bind(this));
      node.querySelectorAll("top-level-values>*").forEach((function(node) {
         let value = new CDeclDefinition(this);
         value.begin_load(node);
         
         value.dereference_count    = +(node.getAttribute("dereference-count") || 0);
         value.force_to_next_sector = node.getAttribute("force-to-next-sector") == "true";
         
         this.top_level_values.push(value);
      }).bind(this));
      node.querySelectorAll("sectors>sector").forEach((function(node) {
         let sector = new SaveSector();
         sector.from_xml(node);
         this.sectors.push(sector);
      }).bind(this));
      //
      // Now that all type definitions have been found, load member data 
      // for aggregate types. We want to load this later so that we can 
      // validate that if a member claims to be of a given type, we have 
      // a definition for that type available.
      //
      for(let type of this.types.structs)
         type.finish_load(type.node);
      for(let type of this.types.unions)
         type.finish_load(type.node);
   }
   
   lookup_type_by_name(name) {
      for(let type of this.types.integrals) {
         if (type.symbol == name)
            return type;
      }
      for(let type of this.types.structs) {
         if (type.tag == name)
            return type;
         if (type.symbol == name)
            return type;
      }
      for(let type of this.types.unions) {
         if (type.tag == name)
            return type;
         if (type.symbol == name)
            return type;
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
      
      out.data = new DataView(view.buffer, view.byteOffset, SAVE_SECTOR_DATA_SIZE);
      
      let offset = FLASH_SECTOR_SIZE - 16;
      
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
   
   /*SaveFile*/ load(/*DataView*/ sav) {
      if (sav.byteLength < FLASH_MEMORY_SIZE) {
         throw new Error("The SAV file is truncated.");
      }
      
      let out = new SaveFile(this);
      for(let i = 0; i < SAVE_SLOT_COUNT; ++i) {
         let slot = out.slots[i];
         
         for(let j = 0; j < SAVE_SECTORS_PER_SLOT; ++j) {
            let pos  = FLASH_SECTOR_SIZE * (SAVE_SECTORS_PER_SLOT * i + j);
            let blob = new DataView(sav.buffer, pos, FLASH_SECTOR_SIZE);
            
            let sector_header = slot.loadSectorMetadata(j, blob);
            let sector_data   = new DataView(sav.buffer, pos, SAVE_SECTOR_DATA_SIZE);
            
            let id = sector_header.sector_id;
            if (sector_header.signature == SAVE_SECTOR_SIGNATURE) {
               console.assert(id < SAVE_SECTORS_PER_SLOT);
            }
            
            if (id < this.sectors.length && this.sectors[id].instructions) {
               let applier = new InstructionsApplier();
               applier.save_format = this;
               applier.root_data   = slot;
               applier.bitstream   = new Bitstream(sector_data);
               applier.read(this.sectors[id].instructions);
            }
         }
      }
      {  // Special sectors
         out.special_sectors.hall_of_fame[0] = new DataView(sav.buffer, FLASH_SECTOR_SIZE*28, FLASH_SECTOR_SIZE);
         out.special_sectors.hall_of_fame[1] = new DataView(sav.buffer, FLASH_SECTOR_SIZE*29, FLASH_SECTOR_SIZE);
         out.special_sectors.trainer_hill = new DataView(sav.buffer, FLASH_SECTOR_SIZE*30, FLASH_SECTOR_SIZE);
         out.special_sectors.recorded_battle = new DataView(sav.buffer, FLASH_SECTOR_SIZE*31, FLASH_SECTOR_SIZE);
      }
      if (sav.byteLength >= FLASH_MEMORY_SIZE + RTC_DATA_SIZE) {
         //
         // Load RTC data.
         //
         let rtc  = [];
         let view = new DataView(sav.buffer, FLASH_MEMORY_SIZE, RTC_DATA_SIZE);
         for(let i = 0; i < RTC_DATA_SIZE; ++i) {
            rtc.push(view.getUint8(i));
         }
         out.rtc = rtc;
      }
      
      return out;
   }
   
   /*DataView*/ save(/*SaveFile*/ data) {
      let buffer = new ArrayBuffer(FLASH_MEMORY_SIZE + (data.rtc ? RTC_DATA_SIZE : 0));
      let view   = new DataView(buffer);
      
      function _copy_sector(src_view, n) {
         let dst_view = new DataView(view.buffer, FLASH_SECTOR_SIZE * n, FLASH_SECTOR_SIZE);
         memcpy(dst_view, src_view, dst_view.byteLength);
      }
      
      for(let i = 0; i < data.slots.length; ++i) {
         let slot = data.slots[i];
         for(let j = 0; j < slot.sectors.length; ++j) {
            let sector_view = new DataView(view.buffer, FLASH_SECTOR_SIZE * (SAVE_SECTORS_PER_SLOT * i + j), FLASH_SECTOR_SIZE);
            
            let header = slot.sectors[j];
            if (header.signature == SAVE_SECTOR_SIGNATURE) {
               if (header.sector_id < this.sectors.length) {
                  let instructions = this.sectors[header.sector_id].instructions;
                  if (instructions) {
                     let applier = new InstructionsApplier();
                     applier.save_format = this;
                     applier.root_data   = slot;
                     applier.bitstream   = new Bitstream(sector_view);
                     applier.save(instructions);
                     
                     header = structuredClone(header);
                     header.checksum = checksumA32R16(sector_view, SAVE_SECTOR_DATA_SIZE);
                  }
               }
            } else {
               if (header.raw_sector) {
                  memcpy(sector_view, header.raw_sector, SAVE_SECTOR_FULL_SIZE);
               }
               continue;
            }
            //
            // Serialize header.
            //
            {
               let offset = SAVE_SECTOR_FULL_SIZE - 16;
               
               sector_view.setUint32(offset, header.version, true);
               offset += 4;
               
               sector_view.setUint16(offset, header.sector_id, true);
               offset += 2;
               
               // TODO: Recompute checksum; write it at the end.
               sector_view.setUint16(offset, header.checksum, true);
               offset += 2;
               
               sector_view.setUint32(offset, header.signature, true);
               offset += 4;
               
               sector_view.setUint32(offset, header.counter, true);
               offset += 4;
            }
         }
      }
      //
      // Handle special sectors.
      //
      _copy_sector(data.special_sectors.hall_of_fame[0], 28);
      _copy_sector(data.special_sectors.hall_of_fame[1], 29);
      _copy_sector(data.special_sectors.trainer_hill, 30);
      _copy_sector(data.special_sectors.recorded_battle, 31);
      //
      if (data.rtc && data.rtc.length == RTC_DATA_SIZE) {
         let subview = new DataView(buffer, FLASH_MEMORY_SIZE, RTC_DATA_SIZE);
         for(let i = 0; i < RTC_DATA_SIZE; ++i) {
            subview.setUint8(i, data.rtc[i]);
         }
      }
      
      return view;
   }
   
   test_round_tripping(/*DataView*/ sav) {
      let decoded = this.load(sav);
      for(let i = 0; i < decoded.slots.length; ++i) {
         let slot = decoded.slots[i];
         for(let j = 0; j < SAVE_SECTORS_PER_SLOT; ++j) {
            let pos = FLASH_SECTOR_SIZE * (SAVE_SECTORS_PER_SLOT * i + j);
            let src = new DataView(sav.buffer, pos, FLASH_SECTOR_SIZE);
            
            let flash_sector = this.#decompose_flash_sector(src);
            if (flash_sector.header.signature != SAVE_SECTOR_SIGNATURE) {
               continue;
            }
            let id = flash_sector.header.sector_id;
            
            let dst      = new ArrayBuffer(FLASH_SECTOR_SIZE - 128);
            let dst_view = new DataView(dst);
            
            let applier = new InstructionsApplier();
            applier.save_format = this;
            applier.root_data   = slot;
            applier.bitstream   = new Bitstream(dst_view);
            applier.save(this.sectors[id].instructions);
            
            for(let k = 0; k < dst_view.byteLength; k += 4) {
               let a = src.getUint32(i);
               let b = dst_view.getUint32(i);
               if (a != b) {
                  console.log("Src view: ", src);
                  console.log("Dst view: ", dst_view);
                  console.assert(false, `Round-tripped data mismatched at slot ${i} sector ${j} (ID ${id}) offset 0x${k.toString(16).toUpperCase().padStart(4, '0')}!`);
               }
            }
         }
      }
      console.log("All good!");
   }
};