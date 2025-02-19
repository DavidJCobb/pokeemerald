
class CValueEditorElement extends HTMLElement {
   #body;
   #shadow;
   #target; // CValueInstance
   
   #input;   // input element
   #preview; // preview element
   
   #target_options = {};
   
   static #enum_definitions = {};
   static registerEnumDefinition(typename, values) {
      if (Array.isArray(values)) {
         let map = {};
         for(let i = 0; i < values.length; ++i)
            map[values[i]] = i;
         values = map;
      }
      this.#enum_definitions[typename] = values;
   }
   
   constructor() {
      super();
      this.#shadow = this.attachShadow({ mode: "open" });
      this.#shadow.innerHTML = `
         <link rel="stylesheet" href="c-value-editor-element.css" />
         <header></header>
         <div></div>
      `.trim();
      this.#body = this.#shadow.querySelector("div");
      
      // Count on the event bubbling, since we may replace/destroy our input element.
      this.#body.addEventListener("input", this.#on_change.bind(this));
      this.#body.addEventListener("change", this.#on_change.bind(this));
   }
   
   get target() { return this.#target; }
   set target(v) {
      if (v) {
         if (!(v instanceof CValueInstance))
            throw new TypeError("CValueInstance instance or falsy expected");
         if (this.#target == v)
            return;
         if (!v.decl)
            throw new Error("CValueInstance lacks a decl");
      } else {
         if (!this.#target)
            return;
      }
      this.#target = v || null;
      this.rebuild();
   }
   
   get value() {
      if (!this.#input || !this.#target)
         return null;
      const decl = this.#target.decl;
      
      let v = this.#input.value;
      switch (decl.type) {
         case "integer":
            if (this.#target_options.enumeration) {
               if (isNaN(+v)) {
                  let enum_val = this.#target_options.enumeration[v];
                  if (enum_val !== undefined)
                     return enum_val;
                  return null;
               }
            }
            v = +v;
            break;
         case "string":
            try {
               v = PokeString.from_text(v);
            } catch (e) {
               return null;
            }
            break;
         default:
            return null;
      }
      return v;
   }
   
   is_valid() {
      let v = this.value;
      if (v === null)
         return false;
      
      switch (this.#target.type) {
         case "integer":
            if (v < this.#target_options.min)
               return false;
            if (v > this.#target_options.max)
               return false;
            break;
         case "string":
            if (v.length > this.#target_options.max_length)
               return false;
            break;
      }
      return true;
   }
   
   #on_change(e) {
      if (!this.#input)
         return;
      if (!this.is_valid())
         return;
      this.update_preview();
      this.#target.value = this.value;
      
      let copy = new e.constructor(e.type, e);
      this.dispatchEvent(copy);
   }
   
   #update_target_options() {
      const decl = this.#target?.decl;
      if (!decl) {
         this.#target_options = {};
         return;
      }
      this.#target_options = {
         type: decl.type,
         src:  decl.options,
      };
      const o = decl.options;
      switch (decl.type) {
         case "integer":
            this.#target_options.enumeration = null;
            {
               let typename = decl.c_typenames.serialized;
               let enum_def = CValueEditorElement.#enum_definitions[typename];
               if (enum_def) {
                  this.#target_options.enumeration = enum_def;
               }
            }
            Object.assign(this.#target_options, this.#target.decl.compute_integer_bounds());
            break;
         case "string":
            this.#target_options.max_length = o.length;
            break;
      }
   }
   
   // Rebuild form controls, etc..
   rebuild() {
      this.#update_target_options();
      this.setAttribute("has-target", !!this.#target);
      this.#input   = null;
      this.#preview = null;
      if (!this.#target || !this.#target.decl) {
         this.#shadow.querySelector("header").textContent = "";
         this.#body.replaceChildren();
         return;
      }
      
      this.#shadow.querySelector("header").textContent = this.#target.build_path_string();
      const decl = this.#target.decl;
      this.setAttribute("data-type", decl.type);
      
      let frag = new DocumentFragment();
      switch (decl.type) {
         case "integer":
            if (this.#target_options.enumeration) {
               const input = this.#input = document.createElement("input");
               input.setAttribute("type", "text");
               
               let enum_def   = this.#target_options.enumeration;
               let datalist   = document.createElement("datalist");
               let value_name = null;
               for(let name in enum_def) {
                  let val = enum_def[name];
                  if (val == this.#target.value) {
                     value_name = name;
                  }
                  let opt = document.createElement("option");
                  opt.value       = val;
                  opt.textContent = name;
                  datalist.append(opt);
               }
               let now  = Date.now();
               let rand = Math.floor(Math.random() * 65535);
               let id = "c-value-editor-datalist-" + ((now << 16) | rand);
               datalist.setAttribute("id", id);
               input.setAttribute("list", id);
               frag.append(datalist);
               
               if (value_name !== null) {
                  input.value = value_name;
               } else {
                  input.value = this.#target.value;
               }
            } else {
               const input = this.#input = document.createElement("input");
               input.setAttribute("type", "number");
               input.setAttribute("size", "12");
               input.setAttribute("min", this.#target_options.min);
               input.setAttribute("max", this.#target_options.max);
               input.value = this.#target.value;
            }
            break;
         case "string":
            {
               const input = this.#input = document.createElement("textarea");
               //input.setAttribute("maxlength", this.#target_options.max_length); // fails for escape codes and XML entities
               
               let printer = new LiteralPokeStringPrinter();
               printer.print(this.#target.value);
               input.value = printer.result;
            }
            this.#preview = document.createElement("div");
            break;
      }
      if (this.#input)
         frag.append(this.#input);
      if (this.#preview) {
         this.#preview.classList.add("preview");
         frag.append(this.#preview);
      }
      this.#body.replaceChildren(frag);
      this.update_preview();
   }
   
   update_preview() {
      if (!this.#target || !this.#preview)
         return;
      const decl = this.#target.decl;
      if (decl.type == "string") {
         let printer = new DOMPokeStringPrinter();
         printer.print(this.#target.value);
         this.#preview.replaceChildren(printer.result);
      }
      
   }
};
customElements.define("c-value-editor", CValueEditorElement);