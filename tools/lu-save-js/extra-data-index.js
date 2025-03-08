
let ExtraDataIndexManager = new (class ExtraDataIndexManager {
   constructor() {
      this.collections_by_serialization_version = new Map(); // Map<int, ExtraDataCollection>
   }
   async load_version(v) {
      let coll = this.collections_by_serialization_version.get(v);
      if (coll)
         return coll;
      
      let pi = Promise.withResolvers();
      coll = new ExtraDataCollection(pi.promise);
      this.collections_by_serialization_version.set(v, coll);
         
      (async function() {
         let base_path = `./formats/${v}/`;
         
         let filenames = [
            "flags.dat",
            "game-stats.dat",
            "items.dat",
            "misc.dat",
            "moves.dat",
            "natures.dat",
            "species.dat",
            "vars.dat",
         ];
         let promises = {};
         for(let name of filenames) {
            promises[name] = fetch(`${base_path}${name}`);
         }
         {
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
         
         let errors = [];
         let files  = new Map();
         let any    = false;
         for(let name of filenames) {
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
         
         coll.finalize(files);
         if (any) {
            if (errors.length) {
               let err = new AggregateError(errors, "Some extra-data files for serialization version " + v + " failed to parse.");
               console.warn(err);
            }
            pi.resolve(coll);
         } else {
            let err = new AggregateError(errors, "All extra-data filess for serialization version " + v + " failed to parse.");
            pi.reject(err);
         }
      })();
         
      return coll;
   }
})();