class CDefinition {
   constructor(format) {
      refuse_abstract_instantiation(CDefinition);
      assert_type(format instanceof SaveFormat);
      this.save_format = format;
      this.node        = null;
   }
   
   begin_load(node) {
      this.node = node;
   }
};