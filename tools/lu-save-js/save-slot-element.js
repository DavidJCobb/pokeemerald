
class SaveSlotElement extends HTMLElement {
   #shadow;
   #editor;
   #field_nodes = {
      counter: null,
      version: null,
      sectors: null,
   };
   #view;
   
   #slot_data = null;
   
   static TYPE_SUMMARY_FORMATTERS = {
      "Coords16": function(inst) {
         for(let key of ["x", "y"]) {
            if (inst.members[key] === null)
               return null;
            if (inst.members[key].value === null)
               return null;
         }
         return `(${inst.members.x.value}, ${inst.members.y.value})`;
      },
      "Time": function(inst) {
         for(let key of ["days", "hours", "minutes", "seconds"]) {
            if (inst.members[key] === null)
               return null;
            if (inst.members[key].value === null)
               return null;
         }
         return `${inst.members.days.value}d ${inst.members.hours.value}hr ${inst.members.minutes.value}m ${inst.members.seconds.value}s`;
      },
   };
   
   constructor() {
      super();
      this.#shadow = this.attachShadow({ mode: "open" });
      this.#shadow.innerHTML = `
<link rel="stylesheet" href="save-slot-element.css" />
<details>
   <summary>Save slot</summary>
   <div>
      <aside>
         <span class="field" data-field="version">
            <label>Version:</label>
            <span class="value">&mdash;</span>
         </span>
         <span class="field" data-field="counter">
            <label>Counter:</label>
            <span class="value">&mdash;</span>
         </span>
         <span class="field">
            <label>Sectors:</label>
            <ul class="sectors"></ul>
         </span>
      </aside>
      <c-view></c-view>
      <c-value-editor></c-value-editor>
   </div>
</details>
      `.trim();
      
      this.#editor = this.#shadow.querySelector("c-value-editor");
      this.#view   = this.#shadow.querySelector("c-view");
      for(let typename in SaveSlotElement.TYPE_SUMMARY_FORMATTERS) {
         this.#view.setTypeFormatter(typename, SaveSlotElement.TYPE_SUMMARY_FORMATTERS[typename]);
      }
      this.#view.allowSelection = true;
      this.#view.addEventListener("selection-changed", (function(e) {
         let item = e.detail.item;
         if (item instanceof CValueInstance)
            this.#editor.target = item;
         else
            this.#editor.target = null;
      }).bind(this));
      this.#editor.addEventListener("edit", this.#on_value_edited.bind(this));
      
      {
         let aside = this.#shadow.querySelector("aside");
         this.#field_nodes.counter = aside.querySelector("[data-field='counter'] .value");
         this.#field_nodes.version = aside.querySelector("[data-field='version'] .value");
         this.#field_nodes.sectors = aside.querySelector(".sectors");
      }
   }
   
   get title() { return this.#shadow.querySelector("summary").textContent; }
   set title(v) {
      this.#shadow.querySelector("summary").textContent = v;
   }
   
   get slotData() {
      return this.#slot_data;
   }
   set slotData(v) {
      if (this.#slot_data === v)
         return;
      if (v) {
         if (!(v instanceof SaveSlot))
            throw new TypeError("SaveSlot instance or falsy expected");
      }
      this.#slot_data = v;
      this.#view.scope = v;
      window.setTimeout(this.#view.repaint.bind(this.#view), 1);
      if (!v) {
         return;
      }
      
      let s_frag  = new DocumentFragment();
      for(let sector of v.sectors) {
         let s_item = document.createElement("li");
         s_frag.append(s_item);
         if (sector.signature == SAVE_SECTOR_SIGNATURE) {
            s_item.className   = sector.checksum_is_valid ? "good" : "bad";
            s_item.textContent = sector.sector_id;
         }
      }
      this.#field_nodes.sectors.replaceChildren(s_frag);
      
      let counter = v.counter;
      if (counter !== null) {
         this.#field_nodes.counter.textContent = counter;
      } else {
         this.#field_nodes.counter.innerHTML = "&mdash;";
      }
      
      let version = v.version;
      if (version !== null) {
         this.#field_nodes.version.textContent = version;
      } else {
         this.#field_nodes.version.innerHTML = "&mdash;";
      }
   }
   
   get scope() { return this.#view.scope; }
   set scope(v) { this.#view.scope = v; }
   
   #on_value_edited(e) {
      {  // BoxPokemon substructs checksum
         if (e.detail.subject.decl.name == "checksum") {
            return; // don't recalc the checksum if the user is editing it directly
         }
         let inst = get_enclosing_SerializedBoxPokemon(e.detail.subject);
         if (inst) {
            let sum = recalc_SerializedBoxPokemon_checksum(inst);
            inst.members.substructs.members.checksum.value = sum;
         }
      }
      this.#view.repaint();
   }
};
customElements.define("save-slot-element", SaveSlotElement);