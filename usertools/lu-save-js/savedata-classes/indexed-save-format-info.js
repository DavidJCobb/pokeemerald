import EMERALD_DISPLAY_OVERRIDES from "../emerald-display-overrides.js";
import ExtraDataCollection from "../extra-data/collection.js";
import ExtraDataFile from "../extra-data/file.js";
import SaveFormat from "./save-format.js";

export default class IndexedSaveFormatInfo {
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
      "easy-chat.dat",
      "game-stats.dat",
      "items.dat",
      "maps.dat",
      "misc.dat",
      "moves.dat",
      "natures.dat",
      "species.dat",
      "trainers.dat",
      "vars.dat",
   ];
   static #extra_data_file_patterns = [
      "flags-#.dat",
   ];
   
   #fetch_extra_data_file(filename) {
      let path;
      if (this.files.owned.includes(filename)) {
         path = "formats/" + this.version + "/";
      } else {
         let v = this.files.shared[filename];
         if (v || v === 0) {
            path = "formats/" + v + "/";
         }
      }
      if (!path) {
         return null;
      }
      return fetch(path + filename);
   }
   
   #fetch_whole_extra_data_files() {
      let promises = {};
      for(let name of IndexedSaveFormatInfo.#extra_data_files) {
         let promise = this.#fetch_extra_data_file(name);
         if (!promise) {
            promises[name] = Promise.reject();
            continue;
         }
         promises[name] = promise;
      }
      return promises;
   }
   #fetch_sliced_extra_data_files() {
      let promises = {};
      for(let name of IndexedSaveFormatInfo.#extra_data_file_patterns) {
         if (name.indexOf("#") < 0)
            continue;
         let group = [];
         for(let i = 0; true; ++i) {
            let here    = name.replaceAll("#", i);
            let promise = this.#fetch_extra_data_file(here);
            if (!promise) {
               break;
            }
            group.push(promise);
         }
         promises[name] = Promise.all(group);
      }
      return promises;
   }
   
   async #load_extra_data() {
      if (this.#extra_data)
         return;
      
      let pi = Promise.withResolvers();
      this.#extra_data = new ExtraDataCollection(pi.promise);
      
      let promises_w = this.#fetch_whole_extra_data_files();
      let promises_s = this.#fetch_sliced_extra_data_files();
      {
         let list = await Promise.allSettled(Object.values(promises_w).concat(Object.values(promises_s)));
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
                  "Some extra-data files for serialization version " + this.version + " could not be accessed."
               :
                  "None of the extra-data for serialization version " + this.version + " files could be accessed."
            );
            if (!any_succeeded) {
               pi.reject(err);
               return;
            }
            console.warn(err);
         }
      }
      
      let any    = false;
      let errors = [];
      let files  = new Map();
      for(let name in promises_w) {
         let response;
         try {
            response = await promises_w[name];
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
      for(let name in promises_s) {
         let responses;
         try {
            responses = await promises_s[name];
         } catch (e) {
            continue; // already handled above
         }
         let coalesced = new ExtraDataFile();
         let failed    = false;
         for(let response of responses) {
            let blob = await response.arrayBuffer();
            try {
               coalesced.parse(blob);
            } catch (ex) {
               errors.push(ex);
               failed = true;
            }
         }
         files.set(name, coalesced);
         if (!failed)
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