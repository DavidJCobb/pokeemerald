import CDefinition from "./c-definition.js";

export default class CTypeDefinition extends CDefinition {
   constructor(format) {
      refuse_abstract_instantiation(CTypeDefinition);
      super(format);
      this.tag    = null; // struct THIS_STRING { ... }
      this.symbol = null; // typedef struct { ... } THIS_STRING
      this.c_info = {
         alignment: null,
         size:      null,
      };
   }
};