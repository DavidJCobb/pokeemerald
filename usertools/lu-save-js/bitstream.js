
class Bitstream {
   constructor(dataview) {
      this.dataview = dataview;
      this.target   = 0; // bytes
      this.shift    = 0; // bits
   }
   
   #advance_by_bits(count) {
      this.shift += count;
      if (this.shift >= 8) {
         this.target += Math.floor(this.shift / 8);
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
   
   skip_bits(bitcount) {
      this.#advance_by_bits(bitcount);
   }
   
   //
   // Reading
   //
   
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
      if (bitcount == 32 && result < 0) {
         //
         // EDGE-CASE: JavaScript bitwise operations coerce values to 
         // signed 32-bit integers, whereas we here want an unsigned 
         // 32-bit integer.
         //
         result += 0xFFFFFFFF + 1;
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
   
   //
   // Writing
   //
   
   write_bool(v) {
      if (!this.shift) {
         this.dataview.setUint8(this.target, v ? 0x80 : 0x00);
         ++this.shift;
         return;
      }
      let mask = 1 << (8 - this.shift - 1);
      let byte = this.dataview.getUint8(this.target);
      if (v)
         byte |= mask;
      else
         byte &= ~mask;
      this.dataview.setUint8(this.target, byte);
      this.#advance_by_bits(1);
   }
   write_unsigned(bitcount, value) {
      if (bitcount < 8) {
         value &= (1 << bitcount) - 1;
      }
      while (bitcount > 0) {
         if (!this.shift) {
            if (bitcount < 8) {
               this.dataview.setUint8(this.target, value << (8 - bitcount));
               this.#advance_by_bits(bitcount);
               return;
            }
            this.dataview.setUint8(this.target, value >> (bitcount - 8));
            this.#advance_by_bytes(1);
            bitcount -= 8;
            continue;
         }
         let extra = 8 - this.shift;
         let byte  = this.dataview.getUint8(this.target);
         if (bitcount <= extra) {
            //
            // The value can fit entirely within the current byte.
            //
            byte |= value << (extra - bitcount);
            this.dataview.setUint8(this.target, byte);
            this.#advance_by_bits(bitcount);
            return;
         }
         byte &= ~(0xFF >> this.shift); // clear the bits we're about to write
         byte |= ((value >> (bitcount - extra)) & 0xFF);
         this.dataview.setUint8(this.target, byte);
         this.#advance_by_bits(extra);
         bitcount -= extra;
      }
   }
   write_string(max_length, value) {
      for(let i = 0; i < max_length; ++i) {
         this.write_unsigned(8, value[i]);
      }
   }
};