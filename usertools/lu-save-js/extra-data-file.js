
// Parser for our *.dat files.
class ExtraDataFile {
   #bytestream;
   
   constructor(buffer) {
      this.found = {
         enums: new Map(), // Map<String prefix, ExtraEnumData>
         vars:  new Map(), // Map<String name, int vaulue>
      };
      
      if (buffer)
         this.parse(buffer);
   }
   
   #extract_next_subrecord() {
      if (this.#bytestream.remaining == 0) {
         return null;
      }
      let signature = this.#bytestream.readEightCC();
      let version   = this.#bytestream.readUint32();
      let size      = this.#bytestream.readUint32();
      let body;
      try {
         body = this.#bytestream.bytestreamFromHere(size);
      } catch (e) {
         throw new Error(
            `Truncated or invalid subrecord (${signature} v${version} at offset 0x${this.#bytestream.position.toString(16).toUpperCase().padStart(8, '0')} with length 0x${size.toString(16).toUpperCase()}, in a file of length 0x${this.#bytestream.length.toString(16).toUpperCase().padStart(8, '0')}).`,
            { cause: e }
         );
      }
      this.#bytestream.skipBytes(size);
      return {
         signature: signature,
         version:   version,
         size:      size,
         body:      body,
      };
   }
   
   parse(buffer) {
      this.#bytestream = new Bytestream(buffer);
      let errors = [];
      let info;
      while (info = this.#extract_next_subrecord()) {
         switch (info.signature) {
            case "ENUMDATA":
            case "ENUNUSED":
               {
                  let name;
                  try {
                     name = ExtraEnumData.peek_enum_name(info.signature, info.body);
                     info.body.position = 0;
                  } catch (ex) {
                     errors.push(ex);
                     break;
                  }
                  let data = this.found.enums.get(name);
                  if (!data) {
                     data = new ExtraEnumData();
                     this.found.enums.set(name, data);
                  }
                  try {
                     data.parse(info.signature, info.body);
                  } catch (ex) {
                     errors.push(ex);
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
   
   concat(/*ExtraDataFile*/ other) {
      for(const [k, v] of other.found.enums) {
         let prior = this.found.enums.get(k);
         if (prior) {
            prior.concat(v);
         } else {
            this.found.enums.set(k, v);
         }
      }
      for(const [k, v] of other.found.vars) {
         this.found.vars.set(k, v);
      }
   }
}