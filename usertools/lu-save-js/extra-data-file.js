
// Parser for our *.dat files.
class ExtraDataFile {
   #bytestream;
   #buffer = null;
   #offset = 0;
   #view   = null;
   
   constructor(buffer) {
      this.found = {
         enums: new Map(), // Map<String prefix, ExtraEnumData>
         vars:  new Map(), // Map<String name, int vaulue>
      };
      
      this.#bytestream = new Bytestream(buffer);
      this.#parse();
   }
   
   #extract_next_subrecord() {
      if (this.#bytestream.remaining == 0) {
         return null;
      }
      let signature = this.#bytestream.readEightCC();
      let version   = this.#bytestream.readUint32();
      let size      = this.#bytestream.readUint32();
      let body      = this.#bytestream.bytestreamFromHere(size);
      this.#bytestream.skipBytes(size);
      return {
         signature: signature,
         version:   version,
         size:      size,
         body:      body,
      };
   }
   
   #parse() {
      let errors = [];
      let info;
      while (info = this.#extract_next_subrecord()) {
         switch (info.signature) {
            case "ENUMDATA":
               {
                  let data = new ExtraEnumData();
                  try {
                     data.parse(info.body);
                  } catch (ex) {
                     errors.push(ex);
                  }
                  if (data.prefix) {
                     this.found.enums.set(data.prefix, data);
                  }
               }
               break;
            case "VARIABLS":
               {
                  let data = new ExtraVariables();
                  try {
                     data.parse(info.body);
                  } catch (ex) {
                     errors.push(ex);
                  }
                  for(const [k, v] of data.variables) {
                     this.found.vars.set(k, v);
                  }
               }
               break;
         }
      }
      if (errors.length) {
         throw new AggregateError(errors, "Problems occurred while parsing an extra-data file.");
      }
   }
   
}