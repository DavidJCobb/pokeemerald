//
// ugly quick-and-dirty JS
//
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
   
   SaveFormatIndex.load().then(function() {
      let node  = document.getElementById("target-version");
      let frag  = new DocumentFragment();
      let empty = true;
      for(const [version, info] of SaveFormatIndex.info) {
         empty = false;
         let opt = document.createElement("option");
         opt.value = version;
         opt.textContent = info.name || `Unnamed savedata version ${version}`;
         frag.append(opt);
      }
      node.replaceChildren(frag);
      if (!empty)
         steps[0].querySelector("button[data-action='confirm']").removeAttribute("disabled");
   });
   
   function back_to_step_one() {
      steps[0].removeAttribute("hidden");
      for(let i = 1; i < steps.length; ++i)
         steps[i].setAttribute("hidden", "hidden");
   }
   
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
   }
   
   function pokemon_info(/*CStructInstance<Pokemon>*/ inst) {
      let box = inst.members["box"];
      if (!box)
         return null;
      if (!box.members.hasSpecies?.value)
         return null;
      let species = box.members.substructs?.members.substruct0?.members.species;
      if (!species || species.value === null || species.value === 0)
         return null;
      
      let out = {
         name: {
            data:    null, // PokeString
            charset: "latin",
         },
         level:  null,
         is_egg: null,
         species: {
            id:   species.value,
            name: null,
         },
      };
      
      {
         let name = box.members.nickname?.value;
         if (name) {
            out.name.data = name;
            
            let lang = box.members.language?.value;
            out.name.charset = (lang == 1) ? "japanese" : "latin";
         }
      }
      
      out.level = inst.members.level?.value;
      if (out.level === undefined)
         out.level = null;
      
      out.is_egg = box.members.isEgg?.value || false;
      
      {
         let extra = state.save_format_info.extra_data;
         if (extra) {
            let enumeration = extra.enums.SPECIES;
            if (enumeration) {
               name = enumeration.members.by_value.get(species.value);
               if (name)
                  out.species.name = name;
            }
         }
      }
      
      return out;
   }
   
   const sav_picker    = steps[0].querySelector("input[type='file' i]");
   const target_picker = document.getElementById("target-version");
   
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
         await info.load();
      } catch (e) {
         show_error("We were unable to load information about this save file version. Please check your Internet connection.");
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
      
      {
         let container = document.getElementById("loaded-save-info");
         container.querySelectorAll("[data-field]").forEach(function(node) {
            let inst = slot.lookupCInstanceByPath(node.getAttribute("data-field"));
            if (!inst || !inst.decl) {
               node.textContent = "???";
               return;
            }
            if (inst.value instanceof PokeString) {
               let printer = new DOMPokeStringPrinter();
               printer.print(inst.value);
               node.replaceChildren(printer.result);
            } else {
               let format = node.getAttribute("data-format");
               if (format) {
                  format = document.querySelector("datalist[id='format-" + format + "']");
                  if (format) {
                     let v    = +inst.value;
                     let name = format.querySelector("option[value='" + v + "']");
                     if (name) {
                        name = name.textContent;
                        if (name) {
                           node.textContent = name;
                           return;
                        }
                     }
                  }
               }
               switch (inst.decl.type) {
                  case "boolean":
                     node.textContent = inst.value ? "yes" : "no";
                     break;
                  case "integer":
                     node.textContent = inst.value;
                     break;
                  case "pointer":
                     node.textContent = "0x" + inst.value.toString(16).toUpperCase().padStart(8, "0");
                     break;
                  default:
                     node.textContent = "...";
                     break;
               }
            }
         });
         let party = container.querySelector("[data-show='player party']");
         if (party) {
            let frag = new DocumentFragment();
            let base = slot.lookupCInstanceByPath("gSaveBlock1Ptr->playerParty");
            if (base instanceof CArrayInstance && base.values) {
               let any = false;
               for(let inst of base.values) {
                  let pokemon = pokemon_info(inst);
                  if (!pokemon)
                     continue;
                  any = true;
                  
                  let item = document.createElement("li");
                  frag.append(item);
                  if (pokemon.name.data) {
                     let printer = new DOMPokeStringPrinter();
                     printer.charset = pokemon.name.charset || "latin";
                     printer.print(pokemon.name.data);
                     item.append(printer.result);
                  }
                  if (pokemon.is_egg) {
                     item.append(" (egg)");
                  } else if (pokemon.level !== null) {
                     item.append(" (Lv. " + pokemon.level + ")");
                  }
               }
               if (any) {
                  party.classList.remove("empty");
               } else {
                  party.classList.add("empty");
                  let item = document.createElement("li");
                  frag.append(item);
                  item.textContent = "None. Your adventure is just beginning!";
               }
            } else {
               frag.append("???");
            }
            party.replaceChildren(frag);
         }
      }
      
      steps[1].removeAttribute("hidden");
      steps[0].setAttribute("hidden", "hidden");
   }
   
   function to_step_three() {
      let named_span = steps[2].querySelector(".if-version-named");
      if (state.save_format_info.name) {
         named_span.querySelector("span").textContent = state.save_format_info.name;
         named_span.removeAttribute("hidden");
      } else {
         named_span.setAttribute("hidden", "hidden");
      }
      steps[2].removeAttribute("hidden");
   }
   
   async function to_step_four() {
      let dst_version     = target_picker.value;
      let dst_format_info = SaveFormatIndex.get_format_info_immediate(dst_version);
      if (!dst_format_info) {
         show_error("Somehow, we were unable to load information for the desired version.");
         return;
      }
      await dst_format_info.load();
      
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
      
      steps[3].removeAttribute("hidden");
   }
   
   (function() { // step 0: upload
      steps[0].querySelector("[data-action='clear']").addEventListener("click", function() {
         sav_picker.value = null;
      });
      steps[0].querySelector("[data-action='confirm']").addEventListener("click", to_step_two);
   })();
   
   (function() { // step 1: verify
      steps[1].querySelector("[data-action='confirm']").addEventListener("click", to_step_three);
      steps[1].querySelector("[data-action='deny']").addEventListener("click", function() {
         show_error("Oh no! If your save file isn't showing up properly, then please contact the hack author, and be ready to send them your save file so they can inspect it.").then(back_to_step_one);
      });
   })();
   
   (function() { // step 2: pick version
      steps[2].querySelector("[data-action='confirm']").addEventListener("click", to_step_four);
   })();
   
})();