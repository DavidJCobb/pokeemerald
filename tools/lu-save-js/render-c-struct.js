
function instance_tree_item_to_node(item) {
   let node = null;
   if (item instanceof CStructInstance) {
      node = document.createElement("c-struct");
      node.value = item;
   } else if (item instanceof CUnionInstance) {
      node = document.createElement("c-union");
      node.value = item;
   } else if (item instanceof CValueInstanceArray) {
      node = document.createElement("c-value-array");
      node.value = item;
   } else if (item instanceof CValueInstance) {
      node = document.createElement("c-value");
      node.value = item;
   }
   return node;
}

class CStructInstanceElement extends HTMLElement {
   #shadow;
   #name  = "";
   #value = null;
   
   constructor() {
      super();
      this.#shadow = this.attachShadow({ mode: "open" });
      this.#shadow.innerHTML = `
         <link rel="stylesheet" href="render-c-struct.css" />
         <details>
            <summary><span class="name">[unnamed]</span></summary>
            <div>
               <slot></slot>
            </div>
         </details>
      `.trim();
   }
   
   get name() { return this.#name; }
   set name(v) {
      this.#name = v + "";
      this.#shadow.querySelector(".name").innerText = this.#name;
   }
   
   get value() { return this.#value; }
   set value(v) {
      if (!v) {
         this.#value = null;
         this.rebuild_descendants();
         return;
      }
      if (!(v instanceof CStructInstance))
         throw new TypeError("must be a CStructInstance");
      if (this.#value == v)
         return;
      this.#value = v;
      this.rebuild_descendants();
   }
   
   rebuild_descendants() {
      if (!this.#value) {
         this.replaceChildren();
         return;
      }
      let frag = new DocumentFragment();
      for(let key in this.#value.members) {
         let val  = this.#value.members[key];
         let node = instance_tree_item_to_node(val);
         node.name = key;
         frag.append(node);
      }
      this.replaceChildren(frag);
   }
};
customElements.define("c-struct", CStructInstanceElement);

class CUnionInstanceElement extends HTMLElement {
   #shadow;
   #name  = "";
   #value = null;
   
   constructor() {
      super();
      this.#shadow = this.attachShadow({ mode: "open" });
      this.#shadow.innerHTML = `
         <link rel="stylesheet" href="render-c-struct.css" />
         <details>
            <summary><span class="name">[unnamed]</span></summary>
            <div>
               <slot></slot>
            </div>
         </details>
      `.trim();
   }
   
   get name() { return this.#name; }
   set name(v) {
      this.#name = v + "";
      this.#shadow.querySelector(".name").innerText = this.#name;
   }
   
   get value() { return this.#value; }
   set value(v) {
      if (!v) {
         this.#value = null;
         this.rebuild_descendants();
         return;
      }
      if (!(v instanceof CUnionInstance))
         throw new TypeError("must be a CUnionInstance");
      if (this.#value == v)
         return;
      this.#value = v;
      this.rebuild_descendants();
   }
   
   rebuild_descendants() {
      if (!this.#value) {
         this.replaceChildren();
         return;
      }
      let val  = this.#value.value;
      let node = instance_tree_item_to_node(val);
      if (node) {
         let name = "[unnamed]";
         for(let memb of this.#value.type.members) {
            if (memb == this.#value.base) {
               name = memb.name;
               break;
            }
         }
         node.name = name;
         this.replaceChildren(node);
      } else {
         this.replaceChildren("[empty/unknown]");
      }
   }
};
customElements.define("c-union", CUnionInstanceElement);

class CValueInstanceArrayElement extends HTMLElement {
   #shadow;
   #name  = "";
   #value = null;
   
   constructor() {
      super();
      this.#shadow = this.attachShadow({ mode: "open" });
      this.#shadow.innerHTML = `
         <link rel="stylesheet" href="render-c-struct.css" />
         <details>
            <summary><span class="name">[unnamed]</span></summary>
            <div>
               <slot></slot>
            </div>
         </details>
      `.trim();
   }
   
   get name() { return this.#name; }
   set name(v) {
      this.#name = v + "";
      this.#shadow.querySelector(".name").innerText = this.#name;
   }
   
   get value() { return this.#value; }
   set value(v) {
      if (!v) {
         this.#value = null;
         this.rebuild_descendants();
         return;
      }
      if (!(v instanceof CValueInstanceArray))
         throw new TypeError("must be a CValueInstanceArray");
      if (this.#value == v)
         return;
      this.#value = v;
      this.rebuild_descendants();
   }
   
   rebuild_descendants() {
      if (!this.#value) {
         this.replaceChildren();
         return;
      }
      let frag = new DocumentFragment();
      let list = this.#value.values;
      for(let i = 0; i < list.length; ++i) {
         let val = list[i];
         let node = instance_tree_item_to_node(val);
         node.name = `[${i}]`;
         frag.append(node);
      }
      this.replaceChildren(frag);
   }
};
customElements.define("c-value-array", CValueInstanceArrayElement);

class CValueInstanceElement extends HTMLElement {
   #shadow;
   #name  = "";
   #value = null;
   
   constructor() {
      super();
      this.#shadow = this.attachShadow({ mode: "open" });
      this.#shadow.innerHTML = `
         <link rel="stylesheet" href="render-c-struct.css" />
         <details>
            <summary><span class="name">[unnamed]</span></summary>
            <div>
               <slot></slot>
            </div>
         </details>
      `.trim();
   }
   
   get name() { return this.#name; }
   set name(v) {
      this.#name = v + "";
      this.#shadow.querySelector(".name").innerText = this.#name;
   }
   
   get value() { return this.#value; }
   set value(v) {
      if (!v) {
         this.#value = null;
         this.rebuild_descendants();
         return;
      }
      if (!(v instanceof CValueInstance))
         throw new TypeError("must be a CValueInstance");
      if (this.#value == v)
         return;
      this.#value = v;
      this.rebuild_descendants();
   }
   
   rebuild_descendants() {
      if (!this.#value) {
         this.removeAttribute("data-type");
         this.replaceChildren();
         return;
      }
      let base = this.#value.base;
      let val  = this.#value.value;
      this.setAttribute("data-type", base.type);
      if (val === null) {
         this.replaceChildren("[none]");
         return;
      }
      switch (base.type) {
         case "boolean":
         case "integer":
            this.replaceChildren(val);
            break;
         case "pointer":
            this.replaceChildren("0x" + val.toString(16).padStart(8, '0'));
            break;
            
         case "string":
            {
               let frag = new DocumentFragment();
               for(let i = 0; i < val.length; ++i) {
                  let cc = val[i];
                  let ch = CHARSET_ENGLISH.bytes_to_chars[cc];
                  if (!ch) {
                     ch = CHARSET_CONTROL_CODES.bytes_to_chars[cc];
                  }
                  
                  let span = document.createElement("span");
                  span.classList.add("char");
                  span.setAttribute("data-byte", val[i]);
                  if (ch) {
                     span.textContent = ch;
                  }
                  frag.append(span);
               }
               this.replaceChildren(frag);
            }
            break;
            
         case "buffer":
            {
               let frag = new DocumentFragment();
               for(let i = 0; i < val.byteLength; ++i) {
                  let e = val.getUint8(i);
                  
                  let span = document.createElement("span");
                  span.classList.add("byte");
                  span.textContent = e.toString(16).padStart(2, '0');
                  frag.append(span);
               }
               this.replaceChildren(frag);
            }
            break;
            
         case "omitted":
            this.replaceChildren("[omitted]");
            break;
      }
   }
};
customElements.define("c-value", CValueInstanceElement);