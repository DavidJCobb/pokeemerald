import { AbstractDataFormatTranslator } from "../savedata-classes/data-format-translator.js";
import SaveFormatIndex from "../savedata-classes/save-format-index.js";

import CArrayInstance from "../c/c-array-instance.js";
import CValueInstance from "../c/c-value-instance.js";

//
// In the vanilla game, Easy Chat words are encoded as a group index and a word 
// index, where 9 bits are allotted for words and the remaining 7 bits are used 
// to identify the groups. However, groups only need 5 bits, so this design 
// wastes 2 bits. Moreover, it also implicitly caps Pokemon species IDs and move 
// IDs at 511, as higher IDs will overflow within Easy Chat.
//
// You can increase EC_MASK_BITS from 9 to 11 to allow Pokemon species IDs and 
// move IDs up to 2047 to be used within Easy Chat, but this will constitute a 
// change to your savedata format. This translator can detect the change and 
// perform the update.
//
export default class EasyChatWordIndexBitcountTranslator extends AbstractDataFormatTranslator {
   #word_bits_by_format = new Map();
   
   #get_word_bits_by_format(/*SaveFormat*/ format) {
      if (!format)
         return null;
      
      let bits = this.#word_bits_by_format.get(format);
      if (bits)
         return bits;
      
      let info = SaveFormatIndex.find_by_format(format);
      if (!info)
         return null;
      let xtra = info.extra_data;
      if (!xtra)
         return null;
      let easy = xtra.easy_chat;
      if (!easy)
         return null;
      bits = easy.word_index_bits;
      if (!bits)
         return null;
      
      this.#word_bits_by_format.set(format, bits);
      return bits;
   }
   
   translateInstance(src, dst) {
      //
      // Ensure we only act on single EasyChatWordIDs, and not arrays.
      //
      if (src.type.symbol != "EasyChatWordID" || dst.type.symbol != "EasyChatWordID")
         return;
      if (!(src instanceof CValueInstance) || !(dst instanceof CValueInstance)) {
         if (src instanceof CArrayInstance && dst instanceof CArrayInstance)
            this.pass(dst);
         return;
      }
      
      if (src.value === 0xFFFF) {
         //
         // This is the "none" sentinel.
         //
         dst.value = 0xFFFF;
         return;
      }
      
      let src_word_bits = this.#get_word_bits_by_format(src.save_format);
      if (!src_word_bits)
         return;
      let dst_word_bits = this.#get_word_bits_by_format(dst.save_format);
      if (!dst_word_bits)
         return;
      
      if (src_word_bits == dst_word_bits) {
         dst.value = src.value;
         return;
      }
      
      let group = src.value >> src_word_bits;
      let index = src.value & ((1 << src_word_bits) - 1);
      if (group > (0xFFFF >> dst_word_bits)) {
         //
         // Fail: the group index in the old save file doesn't fit in the new 
         // save file.
         //
         return;
      }
      if (index > ((1 << dst_word_bits) - 1)) {
         //
         // Fail: the word index in the old save file doesn't fit in the new 
         // save file.
         //
         return;
      }
      
      dst.value = (
         (group << dst_word_bits) |
         index
      );
   }
   
   install(/*TranslationOperation*/ operation) {
      operation.translators_by_typename.add("src", "EasyChatWordID", this);
      operation.translators_by_typename.add("dst", "EasyChatWordID", this);
   }
}