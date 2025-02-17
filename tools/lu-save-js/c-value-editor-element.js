
class CValueEditorElement extends HTMLElement {
   #body;
   #shadow;
   #value; // CValueInstance
   
   #input;   // input element
   #preview; // preview element
   
   constructor() {
      super();
      this.#shadow = this.attachShadow({ mode: "open" });
      this.#shadow.innerHTML = `
         <link rel="stylesheet" href="c-value-editor-element.css" />
         <div></div>
      `.trim();
      this.#body = this.#shadow.querySelector("div");
      
      // Count on the event bubbling, since we may replace/destroy our input element.
      this.#body.addEventListener("input", this.#on_change.bind(this));
      this.#body.addEventListener("change", this.#on_change.bind(this));
   }
   
   get target() { return this.#value; }
   set target(v) {
      if (v) {
         if (!(v instanceof CValueInstance))
            throw new TypeError("CValueInstance instance or falsy expected");
         if (this.#value == v)
            return;
      } else {
         if (!this.#value)
            return;
      }
      this.#value = v || null;
      this.rebuild();
   }
   
   #on_change(e) {
      if (!this.#input)
         return;
      if (!this.#input.willValidate) // TODO: doesn't work
         return;
      this.#value.value = this.#input.value;
      
      let copy = new e.constructor(e.type, e);
      this.dispatchEvent(copy);
   }
   
   // Rebuild form controls, etc..
   rebuild() {
      this.setAttribute("has-target", !!this.#value);
      this.#input   = null;
      this.#preview = null;
      if (!this.#value || !this.#value.decl) {
         this.#body.replaceChildren();
         return;
      }
      switch (this.#value.decl.type) {
         case "integer":
            this.#input = document.createElement("input");
            this.#input.setAttribute("type", "number");
            this.#input.setAttribute("size", "12");
            {
               let o = this.#value.decl.options;
               console.assert(o.bitcount !== undefined);
               
               let min = o.min;
               let max = o.max;
               let bcm  = (1 << o.bitcount) - 1;
               if (o.is_signed && min === null && max === null) {
                  bcm = (1 << (o.bitcount - 1)) - 1; // assume a sign bit
                  min = -(bcm + 1);
                  max = bcm;
               } else if (min === null) {
                  if (max === null) {
                     min = 0;
                     max = (1 << o.bitcount) - 1;
                  } else {
                     if (max > bcm)
                        min = max - bcm;
                     else
                        min = 0;
                  }
               } else if (max === null) {
                  max = min + bcm;
               }
               
               let node = this.#input;
               node.setAttribute("min", min);
               node.setAttribute("max", max);
            }
            this.#input.value = this.#value.value;
            break;
      }
      if (this.#input) {
         if (this.#preview) {
            this.#body.replaceChildren(this.#input, this.#preview);
         } else {
            this.#body.replaceChildren(this.#input);
         }
      } else {
         this.#body.replaceChildren();
      }
   }
};
customElements.define("c-value-editor", CValueEditorElement);