
//
// Created to test format changes made during development. I had a 
// save file that I created before identifying certain fields in the 
// savedata as strings and annotating them as such; ergo in this older 
// save format, the fields were listed as u8 arrays. This translator 
// just converts the u8 arrays into PokeStrings.
//
class DebuggingTranslator1To2 extends AbstractDataFormatTranslator {
   #char_array_to_string(src, dst, member, terminator) {
      let src_inst = member ? src.members[member] : src;
      let dst_inst = member ? dst.members[member] : dst;
      let dst_text = dst_inst.value = new PokeString();
      let size     = src_inst.values.length;
      if (terminator)
         --size;
      for(let i = 0; i < size; ++i)
         dst_text.bytes[i] = src_inst.values[i].value || 0x00;
   }
   #char_array_array_to_string_array(src, dst, member, terminator) {
      let src_array = member ? src.members[member] : src;
      let dst_array = member ? dst.members[member] : dst;
      for(let i = 0; i < src_array.values.length; ++i) {
         let src_inst = src_array.values[i];
         let dst_inst = dst_array.values[i];
         this.#char_array_to_string(src_inst, dst_inst, null, terminator);
      }
   }
   
   translateInstance(src, dst) {
      this.pass(dst);
      if ((dst.type.symbol || dst.type.tag) == "SaveBlock1") {
         this.#char_array_array_to_string_array(src, dst, "registeredTexts", true);
         return;
      }
      if ((dst.type.symbol || dst.type.tag) == "WaldaPhrase") {
         this.#char_array_to_string(src, dst, "text", true);
         return;
      }
      if ((dst.type.symbol || dst.type.tag) == "WonderCard") {
         this.#char_array_to_string(src, dst, "titleText");
         this.#char_array_to_string(src, dst, "subtitleText");
         this.#char_array_array_to_string_array(src, dst, "bodyText");
         this.#char_array_to_string(src, dst, "footerLine1Text");
         this.#char_array_to_string(src, dst, "footerLine2Text");
         return;
      }
      if ((dst.type.symbol || dst.type.tag) == "WonderNews") {
         this.#char_array_to_string(src, dst, "titleText");
         this.#char_array_array_to_string_array(src, dst, "bodyText");
         return;
      }
   }
   
   install(/*TranslationOperation*/ operation) {
      operation.translators_by_typename.add("dst", "SaveBlock1", this);
      operation.translators_by_typename.add("dst", "WaldaPhrase", this);
      operation.translators_by_typename.add("dst", "WonderCard", this);
      operation.translators_by_typename.add("dst", "WonderNews", this);
   }
}