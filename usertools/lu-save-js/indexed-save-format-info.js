
class IndexedSaveFormatInfo {
   #extra_data  = null; // Optional<ExtraDataCollection>
   #save_format = null; // Optional<SaveFormat>
   
   constructor(v, info) {
      this.version = v;
      this.name    = info?.name || null;
      this.date    = info?.date || null;
      this.files = {
         owned:  info?.files  || [], // Array<String filename>
         shared: info?.shared || {}  // Map<String filename, int owning_version>
      };
   }
   
   get extra_data() { return this.#extra_data; }
   get save_format() { return this.#save_format; }
   
   static #extra_data_files = [
      "flags.dat",
      "game-stats.dat",
      "items.dat",
      "misc.dat",
      "moves.dat",
      "natures.dat",
      "species.dat",
      "vars.dat",
   ];
   
   async #load_extra_data() {
      if (this.#extra_data)
         return;
      
      let pi = Promise.withResolvers();
      this.#extra_data = new ExtraDataCollection(pi.promise);
      
      let promises = {};
      for(let name of IndexedSaveFormatInfo.#extra_data_files) {
         let path;
         if (this.files.owned.includes(name)) {
            path = "formats/" + this.version + "/";
         } else {
            let v = this.files.shared[name];
            if (v || v === 0) {
               path = "formats/" + v + "/";
            }
         }
         if (!path) {
            promises[name] = Promise.reject();
            continue;
         }
         promises[name] = fetch(path + name);
      }
      {  // Wait for all promises to settle, and log errors as appropriate.
         let list = await Promise.allSettled(Object.values(promises));
         
         let any_succeeded = false;
         let errors = [];
         for(let info of list) {
            if (info.status == "fulfilled") {
               any_succeeded = true;
               continue;
            }
            errors.push(info.reason);
         }
         if (errors.length) {
            let err = new AggregateError(errors,
               any_succeeded ?
                  "Some extra-data files for serialization version " + v + " could not be accessed."
               :
                  "None of the extra-data for serialization version " + v + " files could be accessed."
            );
            if (!any_succeeded) {
               pi.reject(err);
               return;
            }
            console.warn(err);
         }
      }
      //
      // Gather extra-data file data and feed it to the extra-data collection.
      //
      let errors = [];
      let files  = new Map();
      let any    = false;
      for(let name of IndexedSaveFormatInfo.#extra_data_files) {
         let response;
         try {
            response = await promises[name];
         } catch (e) {
            continue; // already handled above
         }
         let blob = await response.arrayBuffer();
         let file;
         try {
            file = new ExtraDataFile(blob);
         } catch (ex) {
            errors.push(ex);
            continue;
         }
         files.set(name, file);
         any = true;
      }
      this.#extra_data.finalize(files);
      if (any) {
         if (errors.length) {
            let err = new AggregateError(errors, "Some extra-data files for serialization version " + this.version + " failed to parse.");
            console.warn(err);
         }
         pi.resolve(this.#extra_data);
      } else {
         let err = new AggregateError(errors, "All extra-data files for serialization version " + this.version + " failed to parse.");
         pi.reject(err);
      }
   }
   
   async load(/*Optional<File>*/ xml_file) {
      if (this.#save_format) {
         return this.#save_format;
      }
      let data;
      {
         let parser = new DOMParser();
         if (xml_file) {
            let text = await xml_file.text();
            data = parser.parseFromString(text, "text/xml");
         } else {
            if (!this.version && this.version !== 0)
               throw new Error("An IndexedSaveFormatInfo doesn't know its version number, and so can't attempt to fetch its format XML.");
            
            let resp = await fetch("formats/" + this.version + "/format.xml");
            let text = await resp.text();
            data = parser.parseFromString(text, "text/xml");
         }
      }
      let format = this.#save_format = new SaveFormat();
      format.from_xml(data.documentElement);
      {
         let exists = false;
         try {
            EMERALD_DISPLAY_OVERRIDES;
            exists = true;
         } catch (e) {
            // Referencing a non-existent global fails with an error, yet globals 
            // don't make it into `globalThis` automatically.
         }
         if (exists) {
            format.display_overrides = format.display_overrides.concat(EMERALD_DISPLAY_OVERRIDES);
         }
      }
      
      if (this.version || this.version === 0) {
         this.#load_extra_data();
         this.#extra_data.readyPromise.then((function() {
            this.#extra_data.apply(this.#save_format);
         }).bind(this));
         await this.#extra_data.readyPromise;
      }
   }
};