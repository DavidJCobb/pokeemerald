{
   const tab_view = document.getElementById("save-files");
   tab_view.addEventListener("tabchange", function(e) {
      let disable = !e.detail.tabBody;
      document.querySelectorAll("menu [data-acts-on-save-file]").forEach(function(node) {
         node[disable ? "setAttribute" : "removeAttribute"]("disabled", "disabled");
      });
   });
   document.querySelectorAll("menu [data-acts-on-save-file]").forEach(function(node) {
      node.setAttribute("disabled", "disabled");
   });
   
   // Define editor enums as necessary. This is a HACK until we get 
   // the full build system in place for save editing.
   function preprocess_save_format(/*SaveFormat*/ format) {
      /*//
      for(let name in EDITOR_ENUMS) {
         format.enums[name] = EDITOR_ENUMS[name];
      }
      //*/
      format.display_overrides = format.display_overrides.concat(EMERALD_DISPLAY_OVERRIDES);
      (async function() {
         let coll = await ExtraDataIndexManager.load_version(1);
         await coll.readyPromise;
         coll.apply(format);
      })();
   }
   
   {  // Import save file
      let trigger = document.querySelector("menu [data-action='import']");
      let modal   = document.getElementById("dialog-import");
      trigger.addEventListener("click", function(e) {
         if (e.target.hasAttribute("disabled"))
            return;
         modal.showModal();
      });
      
      let file_node_xml = modal.querySelector("input.xml");
      let file_node_sav = modal.querySelector("input.sav");
      
      modal.querySelector("button[data-action='cancel']").addEventListener("click", function() {
         modal.close();
      });
      modal.querySelector("button[data-action='activate']").addEventListener("click", async function() {
         if (!file_node_xml.files.length) {
            alert("Error: no save file format given.");
            return;
         }
         if (!file_node_sav.files.length) {
            alert("Error: no save file given.");
            return;
         }
         let format;
         let sav_buffer;
         {
            let xml    = file_node_xml.files[0];
            let parser = new DOMParser();
            let text   = await xml.text();
            let data   = parser.parseFromString(text, "text/xml");
            
            format = new SaveFormat();
            format.from_xml(data.documentElement);
            preprocess_save_format(format);
         }
         {
            let sav = file_node_sav.files[0];
            sav_buffer = new DataView(await sav.arrayBuffer());
         }
         let file = format.load(sav_buffer);
         console.log("Save dump: ", file);
         
         let node = document.createElement("save-file-element");
         tab_view.append(node);
         node.saveFile = file;
         tab_view.selectedTabBody = node;
         
         modal.close();
      });
   }
   {  // Translate current save file
      let trigger = document.querySelector("menu [data-action='translate']");
      let modal   = document.getElementById("dialog-translate");
      trigger.addEventListener("click", function(e) {
         if (e.target.hasAttribute("disabled"))
            return;
         modal.showModal();
      });
      
      let file_node_xml = modal.querySelector("input.xml");
      
      modal.querySelector("button[data-action='cancel']").addEventListener("click", function() {
         modal.close();
      });
      modal.querySelector("button[data-action='activate']").addEventListener("click", async function() {
         let src_node = tab_view.selectedTabBody;
         if (!src_node || !(src_node instanceof SaveFileElement))
            return;
         let src_file = src_node.saveFile;
         if (!src_file)
            return;
         let src_format = src_file.slots[0].save_format;
         
         if (!file_node_xml.files.length) {
            alert("Error: no save file format given.");
            return;
         }
         let dst_format;
         {
            let xml    = file_node_xml.files[0];
            let parser = new DOMParser();
            let text   = await xml.text();
            let data   = parser.parseFromString(text, "text/xml");
            
            dst_format = new SaveFormat();
            dst_format.from_xml(data.documentElement);
            preprocess_save_format(dst_format);
         }
         let dst_file = new SaveFile(dst_format);
         dst_file.special_sectors = structuredClone(src_file.special_sectors);
         for(let i = 0; i < src_file.slots.length; ++i) {
            let slot = dst_file.slots[i];
            slot.sectors = structuredClone(src_file.slots[i].sectors);
            dst_file.slots.push(slot);
            
            let present = false;
            for(let header of src_file.slots[i].sectors) {
               if (header.signature == SAVE_SECTOR_SIGNATURE) {
                  present = true;
                  break;
               }
            }
            if (present) {
               let operation = new TranslationOperation();
/*//
{
   class TestPlayerNameROT13Translator extends AbstractDataFormatTranslator {
      translateInstance(src, dst) {
         this.pass(dst);
         //
         // We want to translate a specific field, so we `pass` the entire 
         // `dst` object and then write in just the values we want.
         //
         let src_name = src.members["playerName"].value;
         let dst_inst = dst.members["playerName"];
         let dst_name = dst_inst.value = new PokeString();
         for(let i = 0; i < src_name.length; ++i) {
            let byte = src_name.bytes[i];
            if (byte >= 0xBB && byte <= 0xEE) {
               let base   = 0;
               let letter = byte - 0xBB;
               if (letter >= 26) {
                  letter -= 26;
                  base   += 26;
               }
               letter = (letter + 13) % 26;
               byte = 0xBB + base + letter;
            }
            dst_name.bytes.push(byte);
         }
         //
         // Test some deliberate errors.
         //
         dst.members["playerGender"].value = "gril";
         dst.members["playTimeSeconds"].value = 99;
      }
   };
   let tran = new TestPlayerNameROT13Translator();
   operation.translators_for_top_level_values.add("dst", "gSaveBlock2Ptr", tran);
}
//*/
               operation.translate(src_file.slots[i], slot);
               console.log(`Slot ${i + 1} translated.`, slot);
            } else {
               console.log(`Slot ${i + 1} is absent.`);
            }
         }
         
         console.log("Translated save file: ", dst_file);

         let node = document.createElement("save-file-element");
         tab_view.append(node);
         node.saveFile = dst_file;
         tab_view.selectedTabBody = node;
         
         modal.close();
      });
   }
   {  // Import save file
      let trigger = document.querySelector("menu [data-action='export']");
      let modal   = document.getElementById("dialog-export");
      trigger.addEventListener("click", function(e) {
         if (e.target.hasAttribute("disabled"))
            return;
         let src_node = tab_view.selectedTabBody;
         if (!src_node || !(src_node instanceof SaveFileElement))
            return;
         let src_file = src_node.saveFile;
         if (!src_file)
            return;
         let src_format = src_file.slots[0].save_format;
         
         let link = modal.querySelector("a[download]");
         {
            let uri = link.href;
            if (uri)
               URL.revokeObjectURL(uri);
         }
         
         let data_view = src_format.save(src_file);
         let blob      = new Blob([data_view.buffer]);
         let uri       = URL.createObjectURL(blob);
         link.href = uri;
         
         modal.showModal();
      });
      
      let file_link = modal.querySelector("a[download]");
      
      modal.querySelector("button[data-action='cancel']").addEventListener("click", function() {
         modal.close();
      });
   }
}