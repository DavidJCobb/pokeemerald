class EnumInputElement extends HTMLElement {
   static formAssociated = true;
   
   #internals;
   #shadow;
   
   #datalist_id;
   #datalist;
   #input;
   #select;
   #shape_button;
   
   #prefer_showing_select = true;
   #showing_select = false;
   
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
         div {
            display: flex;
            gap:     1ch;
         }
         input, select {
            flex: 1 1 auto;
         }
      </style>
      <div>
         <input type="text" />
         <select hidden></select>
         <button disabled>Swap</button>
      </div>
      <datalist></datalist>
      `.trim();
      
      this.#datalist = this.#shadow.querySelector("datalist");
      this.#input    = this.#shadow.querySelector("input");
      this.#select   = this.#shadow.querySelector("select");
      
      this.#datalist_id = Math.floor(Math.random() * 65535) | (Date.now() << 16);
      let id = "enum-input-element-datalist-" + this.#datalist_id;
      this.#datalist.setAttribute("id", id);
      this.#input.setAttribute("list", id);
      
      this.#input.addEventListener("input", this.#on_text_edited.bind(this));
      this.#input.addEventListener("change", this.#on_text_edited.bind(this));
      this.#select.addEventListener("change", this.#on_select_edited.bind(this));
      
      this.#shape_button = this.#shadow.querySelector("button");
      this.#shape_button.addEventListener("click", this.#on_user_change_shape.bind(this));
   }
   
   #set_current_shape(show_select) {
      if (show_select == this.#showing_select)
         return;
      if (show_select) {
         this.#input.setAttribute("hidden", "hidden");
         this.#select.removeAttribute("hidden");
      } else {
         this.#select.setAttribute("hidden", "hidden");
         this.#input.removeAttribute("hidden");
      }
      this.#showing_select = show_select;
   }
   #on_user_change_shape() {
      let prior = this.#showing_select;
      this.#set_current_shape(!prior);
      this.#prefer_showing_select = this.#showing_select;
   }
   #update_shape(name_is_known) {
      if (name_is_known) {
         this.#shape_button.removeAttribute("disabled");
         if (!this.#prefer_showing_select)
            return;
      } else {
         this.#shape_button.setAttribute("disabled", "disabled");
      }
      this.#set_current_shape(name_is_known);
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
      return null;
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
      let disp;
      let raw;
      if (v+"" === v) {
         disp = v;
         raw  = this.#value_of_name(v);
         if (raw === null) {
            throw new Error("value is an unrecognized name");
         }
      } else {
         raw = +v;
         if (isNaN(raw) || raw !== Math.floor(raw)) {
            throw new TypeError("value is neither a name, an integer, nor convertible to an integer");
         }
         disp = this.#name_of_value(raw);
      }
      if (this.#value === raw)
         return;
      this.#value = raw;
      this.#internals.setFormValue(raw);
      if (disp)
         this.#input.value = disp;
      else
         this.#input.value = raw;
      this.#update_shape(!!disp);
   }
   
   get valueNames() { return this.#value_names; }
   set valueNames(v) {
      if (!v) {
         if (!this.#value_names)
            return;
         this.#value_names = null;
         this.#datalist.replaceChildren();
         this.#select.replaceChildren();
         //
         // If the text input was previously showing a value name, force it 
         // to show the value as an integer, since we no longer recognize 
         // that name.
         //
         this.#input.value = this.#value;
         this.#update_shape(false);
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
      let copy = frag.cloneNode(true);
      this.#datalist.replaceChildren(frag);
      this.#select.replaceChildren(copy);
      if (name)
         this.#input.value = name;
      this.#update_shape(!!name);
   }
   
   #push_value_to_textbox(and_update_shape = false) {
      let name = this.#name_of_value(this.#value);
      if (name)
         this.#input.value = name;
      else
         this.#input.value = this.#value;
      if (and_update_shape)
         this.#update_shape(!!name);
   }
   #push_value_to_select() {
      this.#select.value = this.#value;
   }
   
   #is_datalist_selection_event(e) {
      if (e.type != "input")
         return false;
      if (e.isComposing)
         return false;
      return e.inputType == "insertReplacementText";
   }
   
   #on_select_edited(e) {
      let value = +this.#select.value;
      if (isNaN(value))
         return;
      this.#value = value;
      this.#push_value_to_textbox();
   }
   #on_text_edited(e) {
      let raw   = this.#input.value;
      let value = +raw;
      if (!isNaN(value) && value == Math.floor(value)) {
         //
         // Handle the user typing in raw integers.
         //
         this.#value = value;
         this.#push_value_to_select();
         if (this.#value_names) {
            let name = this.#name_of_value(value);
            if (name) {
               //
               // If this is a change event, OR if it's an input event 
               // generated by a datalist selection, then force the 
               // input element to contain the value name.
               //
               if (e.type == "change" || this.#is_datalist_selection_event(e)) {
                  this.#input.value = name;
                  if (this.#shadow.activeElement == this.#input) {
                     //
                     // After choosing a datalist item in Firefox, the 
                     // cursor may be moved into the middle of the item 
                     // instead of, y'know, the end, where it'd make 
                     // sense. This happens after the `input` event is 
                     // dispatched and handled, so we need a timeout to 
                     // fix it up ASAP.
                     //
                     setTimeout(function() {
                        this.#input.setSelectionRange(name.length, name.length);
                     }.bind(this), 1);
                  }
               }
            }
            this.#update_shape(!!name);
         }
         return;
      }
      //
      // Handle the user typing in strings which might name a value.
      //
      value = this.#value_of_name(raw);
      if (value === null) {
         if (e.type == "change") {
            //
            // If the user typed in an unrecognized value name, then 
            // revert their input on a change event (as those will be 
            // fired when the user hits Enter or when focus leaves the 
            // editor).
            //
            this.#push_value_to_textbox(true);
         }
         return;
      }
      this.#internals.setFormValue(value);
      this.#value = value;
      this.#push_value_to_select();
      this.#update_shape(true);
   }
};
customElements.define("enum-input", EnumInputElement);