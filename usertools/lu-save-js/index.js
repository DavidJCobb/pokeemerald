import SaveFormat from "./savedata-classes/save-format.js";
import SaveFormatIndex from "./savedata-classes/save-format-index.js";
import { TranslationOperation } from "./savedata-classes/data-format-translator.js";
import EMERALD_DISPLAY_OVERRIDES from "./emerald-display-overrides.js";

import assess_sav_version from "./savedata-classes/assess-sav-version.js";

SaveFormatIndex.load().then(function() {
   let node  = document.getElementById("translate-to-version");
   let frag  = new DocumentFragment();
   let empty = true;
   for(const [version, info] of SaveFormatIndex.info) {
      empty = false;
      let opt = document.createElement("option");
      opt.value = version;
      opt.textContent = info.name || version;
      frag.append(opt);
   }
   node.replaceChildren(frag);
   if (!empty) {
      node.removeAttribute("disabled");
      //
      // Wait until all custom element scripts have loaded. (They're 
      // already referenced via <script/> tags, so the browser will 
      // be able to fetch them without waiting for us to reach this 
      // point; but we do want to make sure they're loaded before we 
      // proceed.)
      //
      Promise.allSettled([
         import("./custom-elements/c-view-element.js"),
         import("./custom-elements/c-value-editor-element.js"),
         import("./custom-elements/save-file-element.js"),
         import("./custom-elements/save-slot-element.js")
      ]).then(function() {
         document.querySelector("menu [data-action='import']").removeAttribute("disabled");
      });
   }
});

{
   const tab_view = document.getElementById("save-files");
   tab_view.addEventListener("tabchange", function(e) {
      let disable = !e.detail.tabBody;
      document.querySelectorAll("menu [data-acts-on-save-file]").forEach(function(node) {
         node[disable ? "setAttribute" : "removeAttribute"]("disabled", "disabled");
      });
   });
   
   // Define editor enums as necessary. This is a HACK for bespoke 
   // XML files supplied by the user.
   function preprocess_save_format(/*SaveFormat*/ format) {
      format.display_overrides = format.display_overrides.concat(EMERALD_DISPLAY_OVERRIDES);
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
         if (!file_node_sav.files.length) {
            alert("Error: no save file given.");
            return;
         }
         let format;
         let sav_buffer;
         {
            let sav = file_node_sav.files[0];
            sav_buffer = new DataView(await sav.arrayBuffer());
         }
         if (!file_node_xml.files.length) {
            let version = assess_sav_version(sav_buffer);
            if (version === null) {
               alert("Error: save file version could not be identified.");
               return;
            }
            let info = await SaveFormatIndex.get_format_info(version);
            if (!info) {
               alert("Error: save file version number " + version + " doesn't match any known version.");
               return;
            }
            try {
               await info.load();
            } catch (e) {
               alert("Failed to load information for save file version " + version + ".");
               throw e; // re-throw
            }
            format = info.save_format;
         } else {
            let xml    = file_node_xml.files[0];
            let parser = new DOMParser();
            let text   = await xml.text();
            let data   = parser.parseFromString(text, "text/xml");
            
            format = new SaveFormat();
            format.from_xml(data.documentElement);
            preprocess_save_format(format);
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
      
      let version_picker = modal.querySelector("#translate-to-version");
      let file_node_xml  = modal.querySelector("input.xml");
      
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
         let src_format  = src_file.slots[0].save_format;
         let src_version = src_file.version;
         
         let dst_format;
         let dst_version;
         {
            let type = modal.querySelector("input[name='translate-to-target']:checked");
            if (!type)
               return;
            type = type.value;
            if (type == "known-version") {
               let version = +version_picker.value;
               if (!version && version !== 0)
                  return;
               let info = await SaveFormatIndex.get_format_info(version);
               console.assert(!!info);
               try {
                  await info.load();
               } catch (e) {
                  alert("Error: Failed to load information for save file version " + version + ".");
                  throw e; // re-throw
               }
               dst_format  = info.save_format;
               dst_version = version;
            } else {
               if (!file_node_xml.files.length) {
                  alert("Error: no save file format given.");
                  return;
               }
               let xml    = file_node_xml.files[0];
               let parser = new DOMParser();
               let text   = await xml.text();
               let data   = parser.parseFromString(text, "text/xml");
               
               dst_format = new SaveFormat();
               dst_format.from_xml(data.documentElement);
               preprocess_save_format(dst_format);
            }
         }
         let dst_file = new SaveFile(dst_format);
         dst_file.special_sectors = structuredClone(src_file.special_sectors);
         if (src_file.rtc)
            dst_file.rtc = structuredClone(src_file.rtc);
         for(let i = 0; i < src_file.slots.length; ++i) {
            const src_slot = src_file.slots[i];
            const dst_slot = dst_file.slots[i];
            if (src_slot.version < src_version) {
               //
               // This can happen if the slot is empty, or in a situation where the 
               // user has a playthrough in an old version, and then starts a new 
               // game in a new version and saves one (and only one) time.
               //
               console.log(`Slot ${i + 1} is obsolete.`);
               continue;
            }
            dst_slot.sectors = structuredClone(src_slot.sectors);
            if (dst_version) {
               for(let header of dst_slot.sectors)
                  header.version = dst_version;
            }
            
            let present = false;
            for(let header of src_file.slots[i].sectors) {
               if (header.signature == SAVE_SECTOR_SIGNATURE) {
                  present = true;
                  break;
               }
            }
            if (present) {
               let operation = new TranslationOperation();
               //
               // Here is where you'd want to install any relevant user-defined 
               // translators, to handle any savedata format changes that can't be 
               // handled automatically.
               //
               if (src_version == 1 && dst_version == 2) {
                  let tran = new DebuggingTranslator1To2();
                  tran.install(operation);
               }
               //
               // And with the translators installed, the operation can now proceed.
               //
               try {
                  operation.translate(src_slot, dst_slot);
               } catch (ex) {
                  alert("A problem occurred while trying to translate the savedata.\n\n" + ex.message);
                  console.log(ex);
                  return;
               }
               console.log(`Slot ${i + 1} translated.`, dst_slot);
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
   {  // Export save file
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