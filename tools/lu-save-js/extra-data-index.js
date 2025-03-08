
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
         try {
            await Promise.all(Object.values(promises));
         } catch (e) {
            // network error or we are running on file:///
            pi.reject(e);
            return coll;
         }
         
         let errors = [];
         let files  = new Map();
         let any    = false;
         for(let name of filenames) {
            let blob = await (await promises[name]).arrayBuffer();
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
               let err = new AggregateError(errors, "Some extra-data files in a collection failed to load.");
               console.warn(err);
            }
            pi.resolve(coll);
         } else {
            let err = new AggregateError(errors, "All extra-data files in a collection failed to load.");
            pi.reject(err);
         }
      })();
         
      return coll;
   }
})();