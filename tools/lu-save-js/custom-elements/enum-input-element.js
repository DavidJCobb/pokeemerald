class EnumInputElement extends HTMLElement {
   static formAssociated = true;
   
   #internals;
   #shadow;
   
   #datalist_id;
   #datalist;
   #input;
   
   #value_names = null;
   #value = 0;
   
   constructor() {
      super();
      this.#internals = this.attachInternals();
      this.#shadow = this.attachShadow({ mode: "open" });
      this.#shadow.innerHTML = `
      <style>
         :where(:host) {
            display: block;
         }
         input {
            box-sizing: border-box;
            width: 100%;
         }
      </style>
      <input type="text" />
      <datalist></datalist>
      `.trim();
      
      this.#datalist = this.#shadow.querySelector("datalist");
      this.#input    = this.#shadow.querySelector("input");
      
      this.#datalist_id = Math.floor(Math.random() * 65535) | (Date.now() << 16);
      let id = "enum-input-element-datalist-" + this.#datalist_id;
      this.#datalist.setAttribute("id", id);
      this.#input.setAttribute("list", id);
      
      this.#input.addEventListener("input", this.#on_edited.bind(this));
      this.#input.addEventListener("change", this.#on_edited.bind(this));
   }
   
   #name_of_value(v) {
      const names = this.#value_names;
      if (names) {
         if (Array.isArray(names)) {
            if (names.length > v) {
               let i = names[v];
               if (i || i === 0)
                  return i;
            }
         } else if (names instanceof Map) { // assumed: Map<String, int>
            for(const [name, val] of names)
               if (val === v && name)
                  return name;
         }
      }
      return +v;
   }
   #value_of_name(name) {
      const names = this.#value_names;
      if (!names)
         return null;
      if (Array.isArray(names)) {
         let i = names.indexOf(name);
         if (i >= 0)
            return i;
         return null;
      }
      if (names instanceof Map) {
         let i = names.get(name);
         if (!i && i !== 0)
            return null;
         return i;
      }
      return null;
   }
   
   get value() { return this.#value; }
   set value(v) {
      if (v+"" === v) {
         v = this.#value_of_name(v);
      } else {
         v = +v;
      }
      if (isNaN(v) || v !== Math.floor(v))
         throw new TypeError("value must be an integer or a known name");
      if (this.#value === v)
         return;
      this.#value = v;
      this.#internals.setFormValue(v);
      this.#input.value = this.#name_of_value(v);
   }
   
   get valueNames() { return this.#value_names; }
   set valueNames(v) {
      if (!v) {
         if (!this.#value_names)
            return;
         this.#value_names = null;
         this.#datalist.replaceChildren();
         //
         // If the text input was previously showing a value name, force it 
         // to show the value as an integer, since we no longer recognize 
         // that name.
         //
         this.#input.value = this.#value;
         return;
      }
      if (!Array.isArray(v) && !(v instanceof Map))
         throw new TypeError("value name list must be null, an Array, or a Map");
      this.#value_names = v;
      
      let frag = new DocumentFragment();
      let name = null;
      if (Array.isArray(v)) {
         for(let i = 0; i < v.length; ++i) {
            let opt = document.createElement("option");
            opt.value       = i;
            opt.textContent = v[i];
            frag.append(opt);
         }
         name = v[this.#value];
      } else {
         for(const [v_name, val] of this.#value_names) {
            if (val == this.#value) {
               name = v_name;
            }
            let opt = document.createElement("option");
            opt.value       = val;
            opt.textContent = v_name;
            frag.append(opt);
         }
      }
      this.#datalist.replaceChildren(frag);
      if (name)
         this.#input.value = name;
   }
   
   #on_edited(e) {
      let raw   = this.#input.value;
      let value = +raw;
      if (!isNaN(value)) {
         this.#value = value;
         if (e.type == "change" && this.#value_names) {
            let name = this.#name_of_value(value);
            if (name)
               this.#input.value = name;
         }
         return;
      }
      value = this.#value_of_name(raw);
      if (value === null)
         return;
      this.#internals.setFormValue(value);
      this.#value = value;
   }
};
customElements.define("enum-input", EnumInputElement);