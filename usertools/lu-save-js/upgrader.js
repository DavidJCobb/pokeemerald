import SaveFile from "./save-file.js";
import SaveFormatIndex from "./save-format-index.js";
import SaveSlotSummary from "./upgrader/save-slot-summary.js";
import { TranslationOperation } from "./data-format-translator.js";

function show_error(text) {
   let pi = Promise.withResolvers();
   
   let dialog = document.createElement("dialog");
   dialog.classList.add("error");
   dialog.innerHTML = `
      <header>
         <h1>Error</h1>
      </header>
      <div class="body"></div>
      <div class="buttons">
         <button data-action="dismiss">OK</button>
      </div>
   `.trim();
   dialog.querySelector(".body").textContent = text;
   document.body.append(dialog);
   dialog.querySelector("button[data-action='dismiss']").addEventListener("click", function(e) {
      dialog.remove();
      pi.resolve();
   });
   dialog.showModal();
   
   return pi.promise;
}

function friendly_save_version(/*IndexedSaveFormatInfo*/ info) {
   return info.name || `Unnamed savedata version ${info.version}`;
}

SaveFormatIndex.load().then(
   function done() {
      let node  = document.getElementById("target-version");
      let frag  = new DocumentFragment();
      let empty = true;
      for(const [version, info] of SaveFormatIndex.info) {
         empty = false;
         let opt = document.createElement("option");
         opt.value = version;
         opt.textContent = friendly_save_version(info);
         frag.append(opt);
      }
      node.replaceChildren(frag);
      if (!empty) {
         let section = document.getElementById("step-upload");
         let advance = section.querySelector("button[data-action='confirm']");
         advance.removeAttribute("disabled");
      }
      document.body.classList.add("loaded");
   },
   function fail() {
      show_error("Failed to load information about save file formats. Please check your Internet connection and refresh the page.");
   }
);

//
// ugly quick-and-dirty JS
//
document.querySelectorAll(".filepicker").forEach(function(wrap) {
   let input = wrap.querySelector("input[type='file' i]");
   let clear = wrap.querySelector("button[data-action='clear']");
   clear.addEventListener("click", function() {
      input.value = null;
      clear.setAttribute("disabled", "disabled");
   });
   input.addEventListener("change", function() {
      clear.removeAttribute("disabled");
   });
   if (!input.value) {
      clear.setAttribute("disabled", "disabled");
   }
});
(function() {
   
   const steps = [
      document.getElementById("step-upload"),
      document.getElementById("step-verify"),
      document.getElementById("step-translate"),
      document.getElementById("step-download"),
   ];
   
   const state = {
      save_format:      null,
      save_format_info: null,
      save_file:        null,
   };
   
   
   function back_to_step_one() {
      steps[0].removeAttribute("hidden");
      for(let i = 1; i < steps.length; ++i)
         steps[i].setAttribute("hidden", "hidden");
   }
   
   
   const sav_picker    = document.getElementById("step-upload").querySelector("input[type='file' i]");
   const target_picker = document.getElementById("target-version");
   
   function show_loading_modal(text) {
      const MODAL_TIME_THRESHOLD = 100;
      
      let controller = new AbortController();
      let signal = controller.signal;
      window.setTimeout(function() {
         if (signal.aborted)
            return;
         let dialog = document.createElement("dialog");
         dialog.classList.add("loading");
         dialog.innerHTML = `
            <header>
               <h1>Loading...</h1>
            </header>
            <div class="body"></div>
         `.trim();
         dialog.querySelector(".body").textContent = text;
         document.body.append(dialog);
         dialog.showModal();
         signal.addEventListener("abort", function(e) {
            dialog.remove();
         });
      }, MODAL_TIME_THRESHOLD);
      return controller;
   }
   async function load_format_info(/*IndexedSaveFormatInfo*/ info) {
      let signal  = show_loading_modal("Loading information for save file version " + friendly_save_version(info) + "...");
      let promise = info.load();
      promise.then(signal.abort.bind(signal), signal.abort.bind(signal));
      await promise;
   }
   
   async function to_step_two() {
      if (!sav_picker.files.length) {
         show_error("You need to pick a save file (*.sav) first!");
         return;
      }
      let format;
      let sav_buffer;
      {
         let sav = sav_picker.files[0];
         sav_buffer = new DataView(await sav.arrayBuffer());
      }
      let version = assess_sav_version(sav_buffer);
      if (version === null) {
         show_error("We couldn't figure out what version number your save file is using.");
         return;
      }
      let info = await SaveFormatIndex.get_format_info(version);
      if (!info) {
         show_error("Your save file version doesn't match any known version. (The raw serialization version number is " + version + ".)");
         return;
      }
      try {
         await load_format_info(info);
      } catch (e) {
         show_error("We were unable to load information about this save file version. Please check your Internet connection and try again.");
         return;
      }
      format = info.save_format;
      
      state.save_format      = format;
      state.save_format_info = info;
      
      let file = format.load(sav_buffer);
      state.save_file = file;
      let slot = file.current_slot;
      if (!slot) {
         show_error("This save file appears to be empty.");
         return;
      }
      
      {  // display loaded save summary
         let container = document.getElementById("loaded-save-info");
         let summary   = new SaveSlotSummary(info, slot);
         
         {  // player name
            let node = container.querySelector("[data-field='player-name']");
            let data = summary.player.name;
            if (data.empty) {
               node.textContent = "???";
            } else {
               node.replaceChildren(data.to_dom());
            }
         }
         {  // player gender
            let node = container.querySelector("[data-field='player-gender']");
            let data = summary.player.gender;
            if (data === null) {
               node.textContent = "?";
            } else {
               let text = ["♂", "♀"][data];
               if (!text)
                  text = data+"";
               node.textContent = text;
            }
         }
         {  // player trainer ID
            let node = container.querySelector("[data-field='trainer-id']");
            let data = summary.player.trainer_id;
            node.textContent = data.visible || "?????";
         }
         {
            let list = container.querySelector("[data-field='player-party']");
            let frag = new DocumentFragment();
            if (!summary.party.length) {
               list.classList.add("empty");
               let item = document.createElement("li");
               frag.append(item);
               item.textContent = "None. Your adventure is just beginning!";
            } else {
               for(let pokemon of summary.party) {
                  let item = document.createElement("li");
                  frag.append(item);
                  if (pokemon.species.id) {
                     item.setAttribute("data-species-id", pokemon.species.id);
                  }
                  if (pokemon.nickname.empty) {
                     if (pokemon.species.name) {
                        item.append(pokemon.species.name);
                     } else {
                        item.append("?");
                     }
                  } else {
                     item.append(pokemon.nickname.to_dom());
                  }
                  if (pokemon.is_egg) {
                     item.append(" (egg)");
                  } else if (pokemon.level !== null) {
                     item.append(" (Lv. " + pokemon.level);
                     if (pokemon.species.name && !pokemon.nickname.empty) {
                        item.append(" " + pokemon.species.name);
                     }
                     item.append(")");
                  }
               }
            }
            list.replaceChildren(frag);
         }
         {  // player badges
            let node = container.querySelector("[data-field='player-badges']");
            let list = node.querySelector("ul");
            let data = summary.player.badges;
            if (data === null) {
               list.replaceChildren();
               node.setAttribute("hidden", "hidden");
            } else {
               let frag = new DocumentFragment();
               for(let b of data) {
                  let item = document.createElement("li");
                  item.setAttribute("data-status", b);
                  if (b === null) {
                     item.textContent = "[unknown]";
                  } else if (b) {
                     item.textContent = "[obtained]";
                  } else {
                     item.textContent = "[not obtained]";
                  }
                  frag.append(item);
               }
               list.replaceChildren(frag);
               node.removeAttribute("hidden");
            }
         }
      }
      
      document.getElementById("step-verify").removeAttribute("hidden");
   }
   
   function to_step_three() {
      let section = document.getElementById("step-translate");
      {  // Show version we're upgrading from.
         let named_span = section.querySelector(".if-version-named");
         if (state.save_format_info.name) {
            named_span.querySelector("span").textContent = state.save_format_info.name;
            named_span.removeAttribute("hidden");
         } else {
            named_span.setAttribute("hidden", "hidden");
         }
      }
      {  // Hide current and previous versions from picker
         let any_future = false;
         for(let option of target_picker.children) {
            if (option.nodeName != "option")
               continue;
            let version = +option.value;
            if (version <= state.save_format_info.version) {
               option.setAttribute("hidden", "hidden");
            } else {
               option.removeAttribute("hidden");
               any_future = true;
            }
         }
         if (any_future) {
            section.setAttribute("data-condition", "can-upgrade");
            section.classList.remove("final");
         } else {
            section.setAttribute("data-condition", "already-upgraded");
            section.classList.add("final");
         }
      }
      
      section.removeAttribute("hidden");
   }
   
   async function to_step_four() {
      let dst_version     = target_picker.value;
      let dst_format_info = SaveFormatIndex.get_format_info_immediate(dst_version);
      if (!dst_format_info) {
         show_error("Somehow, we were unable to load information for the desired version.");
         return;
      }
      try {
         await load_format_info(dst_format_info);
      } catch (e) {
         show_error("We were unable to load information about the save file version you want to convert to. Please check your Internet connection and try again.");
         return;
      }
      
      let dst_file = new SaveFile(dst_format_info.save_format);
      dst_file.special_sectors = structuredClone(state.save_file.special_sectors);
      if (state.save_file.rtc)
         dst_file.rtc = structuredClone(state.save_file.rtc);
      for(let i = 0; i < state.save_file.slots.length; ++i) {
         const src_slot = state.save_file.slots[i];
         const dst_slot = dst_file.slots[i];
         if (src_slot.version < state.save_format_info.version) {
            //
            // This can happen if the slot is empty, or in a situation where the 
            // user has a playthrough in an old version, and then starts a new 
            // game in a new version and saves one (and only one) time.
            //
            continue;
         }
         dst_slot.sectors = structuredClone(src_slot.sectors);
         if (dst_version) {
            for(let header of dst_slot.sectors)
               header.version = dst_version;
         }
         
         let present = false;
         for(let header of state.save_file.slots[i].sectors) {
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
            ;
            //
            // And with the translators installed, the operation can now proceed.
            //
            try {
               operation.translate(src_slot, dst_slot);
            } catch (ex) {
               show_error("A problem occurred while trying to translate the savedata.\n\n" + ex.message);
               console.log(ex);
               return;
            }
         }
      }
      
      let link = document.getElementById("download-link");
      {
         let uri = link.href;
         if (uri)
            URL.revokeObjectURL(uri);
      }
      let data_view = dst_format_info.save_format.save(dst_file);
      let blob      = new Blob([data_view.buffer]);
      let uri       = URL.createObjectURL(blob);
      link.href = uri;
      link.setAttribute("download", "converted-save-file.sav");
      
      document.getElementById("step-download").removeAttribute("hidden");
   }
   
   (function() { // step 0: upload
      let section = document.getElementById("step-upload");
      section.querySelector("[data-action='confirm']").addEventListener("click", to_step_two);
   })();
   
   (function() { // step 1: verify
      let section = document.getElementById("step-verify");
      section.querySelector("[data-action='confirm']").addEventListener("click", to_step_three);
      section.querySelector("[data-action='deny']").addEventListener("click", function() {
         show_error("Oh no! If your save file isn't showing up properly, then please contact the hack author, and be ready to send them your save file so they can inspect it.").then(back_to_step_one);
      });
   })();
   
   (function() { // step 2: pick version
      let section = document.getElementById("step-translate");
      section.querySelector("[data-action='confirm']").addEventListener("click", to_step_four);
   })();
   
})();