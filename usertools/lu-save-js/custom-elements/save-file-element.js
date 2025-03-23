import LiteralPokeStringPrinter from "../poke-string/literal-printer.js";
import PokeString from "../poke-string/poke-string.js";
import SaveFile from "../savedata-classes/save-file.js";
import SaveFormatIndex from "../savedata-classes/save-format-index.js";

class SaveFileElement extends HTMLElement {
   #save_file = null;
   #save_slot_elements;
   #shadow;

   constructor() {
      super();
      this.#shadow = this.attachShadow({ mode: "open" });
      this.#shadow.innerHTML = `
         <style>
:host { display: block; }
*:first-child { margin-top: 0 }
*:last-child { margin-bottom: 0 }
         </style>
         <div>
            <save-slot-element></save-slot-element>
            <save-slot-element></save-slot-element>
         </div>
      `.trim();
      this.#save_slot_elements = Array.from(this.#shadow.querySelectorAll("save-slot-element"));
      for(let i = 0; i < this.#save_slot_elements.length; ++i)
         this.#save_slot_elements[i].title = `Save slot ${i + 1}`;
   }
   
   connectedCallback() {
      this.setAttribute("data-tab-is-closable", "");
      this.#update_title();
   }
   
   get saveFile() { return this.#save_file; }
   set saveFile(v) {
      if (v && !(v instanceof SaveFile))
         throw new TypeError("Must be a SaveFile.");
      if (this.#save_file === v)
         return;
      this.#save_file = v;
      if (v) {
         for(let i = 0; i < this.#save_slot_elements.length; ++i) {
            let node = this.#save_slot_elements[i];
            let slot = v.slots[i];
            node.slotData = slot;
         }
      }
      this.#update_title();
   }
   
   get version() {
      if (!this.#save_file)
         return null;
      let v = -1;
      for(let slot of this.#save_file.slots)
         v = Math.max(v, slot.version);
      return v;
   }
   
   #update_title() {
      if (!this.#save_file) {
         this.setAttribute("data-title", "[empty]");
         return;
      }
      let name    = "[unknown]";
      let version = null;
      {
         let slot;
         for(let s of this.#save_file.slots)
            if (!slot || slot.version < s.version)
               slot = s;
         version = slot.version;
         let ps = slot.members.gSaveBlock2Ptr?.members.playerName?.value;
         if (ps && ps instanceof PokeString) {
            let printer = new LiteralPokeStringPrinter();
            printer.print(ps);
            name = printer.result;
         }
      }
      if (version !== null) {
         let version_name = (function() {
            let info = SaveFormatIndex.get_format_info_immediate(version);
            if (!info || !info.name)
               return null;
            return info.name;
         }).call(this);
         this.setAttribute("data-title", name + " [version " + (version_name || version) + "]");
      } else {
         this.setAttribute("data-title", name);
      }
   }
};
customElements.define("save-file-element", SaveFileElement);