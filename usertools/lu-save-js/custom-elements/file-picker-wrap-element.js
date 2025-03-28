customElements.define("file-picker", class _ extends HTMLElement {
   #shadow;
   #clear_button;
   
   constructor() {
      super();
      (this.#shadow = this.attachShadow({ mode: "open" }))
         .innerHTML = '<style>div{display:flex;align-items:center}</style><div><slot></slot><button part="clear-button" disabled>Clear</button></div>';
      
      (this.#clear_button = this.#shadow.querySelector("button"))
         .addEventListener("click", this.#on_clear_click.bind(this));
      
      this.addEventListener("change", this.#on_change.bind(this));
   }
   
   get #input() {
      return this.querySelector("input")||{};
   }
   
   #on_clear_click() {
      this.#input.value = "";
      this.#clear_button.disabled = true;
   }
   
   #on_change() {
      this.#clear_button.disabled = !this.#input.files?.length;
   }
});