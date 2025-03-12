class TestPlayerNameROT13Translator extends AbstractDataFormatTranslator {
   translateInstance(src, dst) {
      this.pass(dst);
      //
      // We want to translate a specific field, so we `pass` the entire 
      // `dst` object and then write in just the values we want.
      //
      let src_name = src.members["playerName"].value;
      let dst_inst = dst.members["playerName"];
      let dst_name = dst_inst.value = new PokeString();
      for(let i = 0; i < src_name.length; ++i) {
         let byte = src_name.bytes[i];
         if (byte >= 0xBB && byte <= 0xEE) {
            let base   = 0;
            let letter = byte - 0xBB;
            if (letter >= 26) {
               letter -= 26;
               base   += 26;
            }
            letter = (letter + 13) % 26;
            byte = 0xBB + base + letter;
         }
         dst_name.bytes.push(byte);
      }
      //
      // Test some deliberate errors.
      //
      dst.members["playerGender"].value = "gril";
      dst.members["playTimeSeconds"].value = 99;
   }
   
   install(/*TranslationOperation*/ operation) {
      operation.translators_for_top_level_values.add("dst", "gSaveBlock2Ptr", this);
   }
};