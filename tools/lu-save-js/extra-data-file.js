
// Parser for our *.dat files.
class ExtraDataFile {
   #bytestream;
   #buffer = null;
   #offset = 0;
   #view   = null;
   
   constructor(buffer) {
      this.found = {
         enums: new Map(), // Map<String prefix, Map<String name, int value>>
         vars:  new Map(),
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
   
   #parse_ENUMDATA(body) {
      let enumeration = new Map();
      
      let flags  = body.readUint8();
      let prefix = body.readLengthPrefixedString(1);
      let count  = body.readUint32();
      
      let is_signed  = flags & 1;
      let is_sparse  = flags & 2;
      let has_lowest = flags & 4;
      
      let names;
      let lowest;
      {
         let name_block;
         let size;
         if (is_sparse) {
            size = body.readUint32();
            name_block = body.viewFromHere(size);
         } else {
            if (has_lowest) {
               lowest = body["read" + (is_signed ? "I" : "Ui") + "nt32"]();
            } else {
               lowest = 0;
            }
            name_block = body.viewFromHere();
         }
         let decoder = new TextDecoder();
         names = decoder.decode(name_block).split("\0"); 
         if (names.length == 1 && !names[0]) {
            names = []; // "" splits to [""] when we'd want []
         }
         if (is_sparse) {
            body.skipBytes(size);
         }
         if (names.length != count) {
            throw new Error("Expected " + count + " names; got " + names.length + ".", { cause: names });
         }
      }
      if (is_sparse) {
         for(let i = 0; i < names.length; ++i) {
            let value = body["read" + (is_signed ? "I" : "Ui") + "nt32"]();
            enumeration.set(prefix + names[i], value);
         }
      } else {
         for(let i = 0; i < names.length; ++i) {
            enumeration.set(prefix + names[i], lowest + i);
         }
      }
      this.found.enums.set(prefix, enumeration);
   }
   
   #parse_VARIABLS(body) {
      while (body.remaining) {
         let name      = body.readLengthPrefixedString(2);
         let is_signed = (body.readUint8() & 0x01) != 0;
         let value     = body["read" + (is_signed ? "I" : "Ui") + "nt32"]();
         this.found.vars.set(name, value);
      }
   }
   
   #parse() {
      let info;
      while (info = this.#extract_next_subrecord()) {
         switch (info.signature) {
            case "ENUMDATA":
               this.#parse_ENUMDATA(info.body);
               break;
            case "VARIABLS":
               this.#parse_VARIABLS(info.body);
               break;
         }
      }
   }
   
}