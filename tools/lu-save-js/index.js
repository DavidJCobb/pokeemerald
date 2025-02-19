{
   async function _get_save_format() {
      let data_x = null;
      {
         let node_x = document.getElementById("file-xml");
         if (!node_x.files.length) {
            alert("Error: no save file format given");
            return;
         }
         let file = node_x.files[0];
         
         let parser = new DOMParser();
         let text   = await file.text();
         data_x = parser.parseFromString(text, "text/xml");
      }
      let format = new SaveFormat();
      format.from_xml(data_x.documentElement);
      console.log("Save format: ", format);
      return format;
   };
   
   document.querySelector(".form button[data-action='load']").addEventListener("click", async function() {
      let data_s = null;
      {
         let node_s = document.getElementById("file-sav");
         if (!node_s.files.length) {
            alert("Error: no save file given");
            return;
         }
         let file = node_s.files[0];
         data_s = new DataView(await file.arrayBuffer());
      }
      
      let format  = await _get_save_format();
      let decoded = format.load(data_s);
      console.log("Save dump: ", decoded);
      
      let section = document.querySelector("section.slots");
      let frag    = new DocumentFragment();
      for(let i = 0; i < decoded.slots.length; ++i) {
         let slot = decoded.slots[i];
         let element = document.createElement("save-slot-element");
         element.title = `Save slot ${i + 1}`;
         frag.append(element);
         element.slotData = slot;
      }
      section.replaceChildren(frag);
   });
   
   document.querySelector(".form button[data-action='roundtrip']").addEventListener("click", async function() {
      let data_s = null;
      {
         let node_s = document.getElementById("file-sav");
         if (!node_s.files.length) {
            alert("Error: no save file given");
            return;
         }
         let file = node_s.files[0];
         data_s = new DataView(await file.arrayBuffer());
      }
      
      let format = await _get_save_format();
      format.test_round_tripping(data_s);
   });
   
   document.querySelector(".form button[data-action='translate']").addEventListener("click", async function() {
      const USE_NEW_TRANSLATION = true;
      
      let data_s = null;
      {
         let node_s = document.getElementById("file-sav");
         if (!node_s.files.length) {
            alert("Error: no save file given");
            return;
         }
         let file = node_s.files[0];
         data_s = new DataView(await file.arrayBuffer());
      }
      
      let src_format = await _get_save_format();
      let dst_format;
      {
         let node_x = document.getElementById("file-xml");
         if (!node_x.files.length) {
            alert("Error: no save file format given");
            return;
         }
         let file = node_x.files[0];
         
         let parser = new DOMParser();
         let text   = await file.text();
         let data_x = parser.parseFromString(text, "text/xml");
         
         dst_format = new SaveFormat();
         dst_format.from_xml(data_x.documentElement);
      }
      
      let loaded   = src_format.load(data_s);
      let dst_data = {
         slots: [],
         special_sectors: loaded.special_sectors,
      };
      for(let i = 0; i < loaded.slots.length; ++i) {
         let slot = new SaveSlot(dst_format);
         slot.sectors = structuredClone(loaded.slots[i].sectors);
         dst_data.slots.push(slot);
         
         let present = false;
         for(let header of loaded.slots[i].sectors) {
            if (header.signature == SAVE_SECTOR_SIGNATURE) {
               present = true;
               break;
            }
         }
         if (present) {
            if (USE_NEW_TRANSLATION) {
               let operation = new TranslationOperation();
               operation.translate(loaded.slots[i], slot);
               console.log(`Slot ${i + 1} translated.`, slot);
            } else {
               let info = translate_value_instance_tree(src_format, loaded.slots[i], dst_format, slot);
               console.log(`Translation result for slot ${i + 1}`, info);
            }
         } else {
            console.log(`Slot ${i + 1} is absent.`);
         }
      }
      let repacked = dst_format.save(dst_data);
      console.log("Repacked: ", repacked);
      
      let roundtripped = dst_format.load(repacked);
      console.log("Round-tripped: ", roundtripped);
      
      let section = document.querySelector("section.slots");
      let frag    = new DocumentFragment();
      for(let i = 0; i < roundtripped.slots.length; ++i) {
         let slot = roundtripped.slots[i];
         let element = document.createElement("save-slot-element");
         element.title = `Save slot ${i + 1} after translation and reload`;
         frag.append(element);
         element.slotData = slot;
      }
      section.replaceChildren(frag);
   });
}