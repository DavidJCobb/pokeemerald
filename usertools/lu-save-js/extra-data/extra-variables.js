export default class ExtraVariables {
   constructor() {
      this.variables = new Map(); // Map<String, int>
   }
   
   // Parse enum data from a VARIABLS subrecord.
   parse(/*Bytestream*/ body) {
      while (body.remaining) {
         let name      = body.readLengthPrefixedString(2);
         let is_signed = (body.readUint8() & 0x01) != 0;
         let value     = body["read" + (is_signed ? "I" : "Ui") + "nt32"]();
         this.variables.set(name, value);
      }
   }
}