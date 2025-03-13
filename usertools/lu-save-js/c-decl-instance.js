class CDeclInstance extends CInstance {
   constructor(decl) {
      refuse_abstract_instantiation(CDeclInstance);
      assert_type(decl instanceof CDeclDefinition);
      super(decl, decl.c_types.serialized.definition);
   }
};