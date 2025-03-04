
class CValueEditorElement extends HTMLElement {
   #body;
   #shadow;
   #target; // CValueInstance
   
   #input;   // input element
   #preview; // preview element
   
   #target_options = {};
   
   #checksum_recalc_helper = null; // ChecksumRecalcHelper
   
   static #base_path = "";
   static {
      if (document.currentScript) {
         let src = (function() {
            try {
               return new URL(".", document.currentScript.src);
            } catch (e) {
               return "";
            }
         })();
         if (src) {
            this.#base_path = src + "/";
         }
      }
   }
   
   constructor() {
      super();
      this.#shadow = this.attachShadow({ mode: "open" });
      this.#shadow.innerHTML = `
         <link rel="stylesheet" href="${CValueEditorElement.#base_path}c-value-editor-element.css" />
         <header></header>
         <div></div>
      `.trim();
      this.#body = this.#shadow.querySelector("div");
      
      // Count on the event bubbling, since we may replace/destroy our input element.
      this.#body.addEventListener("input", this.#on_change.bind(this));
      //this.#body.addEventListener("change", this.#on_change.bind(this));
      
      if (globalThis.ChecksumRecalcHelper) {
         this.#checksum_recalc_helper = new ChecksumRecalcHelper();
      }
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
      if (this.#checksum_recalc_helper)
         this.#checksum_recalc_helper.subject = v;
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
   
   #update_unions_tagged_by_target() {
      if (this.#target.value === null) {
         return;
      }
      for(let u of this.#target.is_tag_of_unions) {
         //
         // Update externally-tagged union.
         //
         u.emplace();
      }
      //
      // Are we inside of a union?
      //
      let u = this.#target.is_member_of?.is_member_of;
      if (u instanceof CUnionInstance && !u.external_tag && u.type.internal_tag_name == this.#target.decl.name) {
         let decl = u.type.members_by_tag_value[+this.#target.value];
         let old  = u.value;
         u.emplace(decl);
         
         this.#target = u.value.members[this.#target.decl.name];
         this.dispatchEvent(new CustomEvent("edit-emplaced-containing-union", {
            detail: {
               subject:   this.#target,
               value:     this.#target.value,
               union:     u,
               emplaced:  u.value,
               discarded: old,
            },
         }));
      }
   }
   
   #on_change(e) {
      if (!this.#input)
         return;
      if (!this.is_valid())
         return;
      this.#target.value = this.value;
      this.#update_unions_tagged_by_target();
      this.update_preview();
      if (this.#checksum_recalc_helper)
         this.#checksum_recalc_helper.recalc();
      
      this.dispatchEvent(new CustomEvent("edit", {
         detail: {
            subject: this.#target,
            value:   this.#target.value,
         },
      }));
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
               let typename = decl.c_types.serialized.name;
               let format   = this.#target?.save_format;
               let enum_def = format.enums[typename];
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
   
   #update_displayed_path() {
      let node = this.#shadow.querySelector("header");
      if (!this.#target) {
         node.textContent = "";
         return;
      }
      let path = this.#target.build_path_string();
      path = path.replaceAll(".", "\u200B.");
      node.textContent = path;
   }
   
   // Rebuild form controls, etc..
   rebuild() {
      this.#update_target_options();
      this.setAttribute("has-target", !!this.#target);
      this.#input   = null;
      this.#preview = null;
      this.#update_displayed_path();
      if (!this.#target || !this.#target.decl) {
         this.#body.replaceChildren();
         return;
      }
      const decl = this.#target.decl;
      this.setAttribute("data-type", decl.type);
      
      {
         let format  = decl.save_format;
         let display = null;
         for(let item of format.display_overrides) {
            if (!item.overrides.make_editor_element)
               continue;
            if (item.matches(this.#target)) {
               display = item;
               break;
            }
         }
         if (display) {
            this.#input = display.overrides.make_editor_element(this.#target);
            this.#input.value = this.#target.value;
            this.#body.replaceChildren(this.#input);
            this.update_preview();
            return;
         }
      }
      
      let frag = new DocumentFragment();
      let type = decl.type;
      if (type == "integer") {
         if (decl.options.bitcount == 1)
            type = "boolean";
      }
      switch (type) {
         case "boolean":
            {
               const input = this.#input = document.createElement("select");
               {
                  let opt = document.createElement("option");
                  opt.value = 0;
                  opt.textContent = "false";
                  input.append(opt);
               }
               {
                  let opt = document.createElement("option");
                  opt.value = 1;
                  opt.textContent = "true";
                  input.append(opt);
               }
               input.value = this.#target.value;
            }
            break;
         case "integer":
            if (this.#target_options.enumeration) {
               const input = this.#input = document.createElement("enum-input");
               input.valueNames = this.#target_options.enumeration;
               input.value      = this.#target.value;
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
               if (this.#target.value) {
                  let printer = new LiteralPokeStringPrinter();
                  printer.print(this.#target.value);
                  input.value = printer.result;
               }
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
         try {
            printer.print(this.#target.value);
         } catch (e) {
            //
            // Invalid control code, entity name, etc.. The user may still be 
            // in the middle of typing it.
            //
            return;
         }
         this.#preview.replaceChildren(printer.result);
      }
      
   }
};
customElements.define("c-value-editor", CValueEditorElement);