import IndexedSaveFormatInfo from "./indexed-save-format-info.js";

const SaveFormatIndex = new (class SaveFormatIndex {
   #load_promise = null;
   
   constructor() {
      this.data = null; // formats/index.json
      this.info = new Map(); // Map<int, IndexedSaveFormatInfo>
   }
   
   async #load_impl() {
      let resp = await fetch("formats/index.json");
      this.data = await resp.json();
      if (this.data.formats) {
         for(let version in this.data.formats) {
            version = +version;
            let src = this.data.formats[version];
            if (isNaN(version) || !src)
               continue;
            let info = new IndexedSaveFormatInfo(version, src);
            this.info.set(version, info);
         }
      }
   }
   /*Promise*/ load() {
      if (this.#load_promise)
         return this.#load_promise;
      this.#load_promise = this.#load_impl();
      return this.#load_promise;
   }
   
   /*Optional<IndexedSaveFormatInfo>*/ async get_format_info(/*int*/ version) {
      await this.load();
      return this.info.get(+version)
   }
   /*Optional<IndexedSaveFormatInfo>*/ get_format_info_immediate(/*int*/ version) {
      return this.info.get(+version)
   }
   
   /*Optional<IndexedSaveFormatInfo>*/ find_by_format(/*SaveFormat*/ format) {
      let match = null;
      this.info.forEach(function(v, k) {
         if (v.save_format === format) {
            match = v;
         }
      });
      return match;
   }
})();
export default SaveFormatIndex;