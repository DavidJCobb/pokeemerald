class BitsetInputElement extends HTMLElement {
   static formAssociated = true;
   
   #bitcount = 0;
   #labels   = null; // Map<size_t bit_index, String bit_name>
   #value    = 0;
   
   #checkboxes = []; // Array<HTMLInputElement>
   #internals;
   #shadow;
   
   constructor() {
      super();
      this.#internals = this.attachInternals();
      this.#shadow = this.attachShadow({ mode: "open" });
      this.#shadow.innerHTML = `
<style>
:where(:host) {
   display: block;
}
ul {
   list-style: none;
   padding:    0;
   margin:     0;
}
label {
   display:     flex;
   flex-flow:   row nowrap;
   align-items: start;
}
</style>
<ul></ul>
      `.trim();
      
      this.#shadow.querySelector("ul").addEventListener("input", this.#on_checkbox_edited.bind(this));
   }
   
   get bitcount() { return this.#bitcount; }
   set bitcount(v) {
      v = +v;
      if (isNaN(v) || v < 0 || v !== Math.floor(v))
         throw new Error("Bitcount must be a non-negative integer.");
      if (v > 32)
         throw new Error("JavaScript doesn't support bitwise operations on values wider than 32 bits.");
      if (this.#bitcount === v)
         return;
      this.#bitcount = v;
      this.#on_bitcount_changed();
   }
   
   get bitMax() {
      if (this.#bitcount == 32)
         return 0xFFFFFFFF;
      return (1 << this.#bitcount) - 1;
   }
   
   get labels() {
      return this.#labels;
   }
   set labels(v) {
      if (!v) {
         if (!this.#labels)
            return;
         this.#labels = null;
      } else if (v instanceof Map) {
         this.#labels = v;
      } else if (Array.isArray(v)) {
         this.#labels = new Map();
         for(let i = 0; i < v.length; ++i)
            this.#labels.set(i, v[i]);
      } else {
         throw new TypeError("Invalid type.");
      }
      this.#on_labels_changed();
   }
   
   get value() { return this.#value; }
   set value(v) {
      v = +v;
      if (isNaN(v))
         throw new TypeError("Value must be a 32-bit integer.");
      if (v < 0) {
         v += 0xFFFFFFFF + 1;
         if (v < 0)
            throw new Error("The given value is too wide.");
      }
      if (v > this.bitMax) {
         throw new Error("The given value is too wide.");
      }
      if (v === this.#value)
         return;
      this.#value = v;
      this.#internals.setFormValue(v);
      for(let i = 0; i < this.#checkboxes.length; ++i) {
         let checked = v & (1 << i);
         this.#checkboxes[i].checked = checked;
      }
   }
   
   //
   
   #on_bitcount_changed() {
      let size = this.#checkboxes.length;
      if (this.#bitcount === size)
         return;
      if (this.#bitcount > size) {
         let list = this.#shadow.querySelector("ul");
         let frag = new DocumentFragment();
         for(let i = size; i < this.#bitcount; ++i) {
            let item  = document.createElement("li");
            let label = document.createElement("label");
            let node  = document.createElement("input");
            node.setAttribute("type", "checkbox");
            node.value = 1 << i;
            label.append(node);
            {
               let text;
               if (this.#labels)
                  text = this.#labels.get(i);
               if (!text)
                  text = "Bit " + i;
               label.append(text);
            }
            item.append(label);
            frag.append(item);
            
            this.#checkboxes.push(node);
            
            if (this.#value & (1 << i))
               node.checked = true;
         }
         list.append(frag);
      } else {
         for(let i = this.#bitcount; i < size; ++i) {
            let node = this.#checkboxes[i];
            let item = node.parentNode;
            item.remove();
         }
         this.#checkboxes.splice(this.#bitcount, size - this.#bitcount);
         if (this.#bitcount < 32) {
            let mask = (1 << this.#bitcount) - 1;
            this.#value &= mask;
         }
      }
   }
   
   #on_labels_changed() {
      if (!this.#labels) {
         for(let i = 0; i < this.#checkboxes.length; ++i) {
            let node  = this.#checkboxes[i];
            let label = node.nextSibling;
            if (label) {
               label.data = "Bit " + i;
            } else {
               node.parentNode.append("Bit " + i);
            }
         }
         return;
      }
      for(let i = 0; i < this.#checkboxes.length; ++i) {
         let node  = this.#checkboxes[i];
         let label = node.nextSibling;
         let text  = this.#labels.get(i);
         if (text === undefined)
            text = "Bit " + i;
         if (label) {
            label.data = text;
         } else {
            node.parentNode.append(text);
         }
      }
   }
   
   #on_checkbox_edited(e) {
      let node = e.target.closest("input");
      if (node.checked) {
         this.#value |= node.value;
      } else {
         this.#value &= ~node.value;
      }
      this.#internals.setFormValue(this.#value);
   }
}
customElements.define("bitset-input", BitsetInputElement);