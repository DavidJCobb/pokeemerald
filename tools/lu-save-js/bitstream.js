
class Bitstream {
   constructor(dataview) {
      this.dataview = dataview;
      this.target   = 0; // bytes
      this.shift    = 0; // bits
   }
   
   #advance_by_bits(count) {
      this.shift += count;
      if (this.shift >= 8) {
         this.target += this.shift / 8;
         this.shift  %= 8;
      }
   }
   #advance_by_bytes(count) {
      this.target += count;
   }
   
   #consume_byte_for_read(bitcount) {
      let raw      = this.dataview.getUint8(this.target) & (0xFF >> this.shift);
      let read     = 8 - this.shift;
      let consumed = 0;
      if (bitcount < read) {
         raw = raw >>> (read - bitcount);
         this.#advance_by_bits(bitcount);
         consumed = bitcount;
      } else {
         this.#advance_by_bits(read);
         consumed = read;
      }
      return {
         value:    raw,
         consumed: consumed,
      };
   }
   
   skip_reading_bits(bitcount) {
      this.#advance_by_bits(bitcount);
   }
   
   read_bool() {
      return (this.#consume_byte_for_read(1).value & 1) == 1;
   }
   read_unsigned(bitcount) {
      let info      = this.#consume_byte_for_read(bitcount);
      let result    = info.value;
      let remaining = bitcount - info.consumed;
      while (remaining) {
         info   = this.#consume_byte_for_read(remaining);
         result = (result << info.consumed) | info.value;
         remaining -= info.consumed;
      }
      return result;
   }
   read_signed(bitcount) { // UNTESTED AND NOT NEEDED FOR CODEGEN
      let value    = this.read_unsigned(bitcount);
      let sign_bit = value >> (bitcount - 1);
      if (sign_bit) {
         // Set all bits "above" the serialized ones, to sign-extend.
         value |= ~((1 << bitcount) - 1);
      }
      return value;
   }
   read_string(max_length) {
      let s = [];
      for(let i = 0; i < max_length; ++i) {
         let c = this.read_unsigned(8);
         s.push(c);
      }
      return s;
   }
};