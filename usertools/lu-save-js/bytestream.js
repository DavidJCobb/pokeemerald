
class Bytestream {
   #endianness = true; // little-endian
   #offset = 0;
   #view;
   
   constructor(buffer) {
      if (buffer instanceof DataView) {
         this.#view = buffer;
      } else {
         this.#view = new DataView(buffer);
      }
   }
   get length() {
      return this.#view.byteLength;
   }
   get remaining() {
      return this.#view.byteLength - this.#offset;
   }
   get position() {
      return this.#offset;
   }
   set position(v) {
      v = +v;
      if (isNaN(v))
         throw new TypeError("Invalid position.");
      if (v < 0 || v > this.#view.byteLength)
         throw new Error("Out-of-bounds seek.");
      this.#offset = v;
   }
   
   #require_size_for_read(n) {
      if (this.#offset + n > this.#view.byteLength)
         throw new Error("Out-of-bounds read.");
   }
   
   #read_sized_string(size) {
      this.#require_size_for_read(size);
      let s = "";
      for(let i = 0; i < size; ++i)
         s += String.fromCharCode(this.#view.getUint8(this.#offset + i));
      this.#offset += size;
      return s;
   }
   #read_unsigned_int(bytes, signed) {
      this.#require_size_for_read(bytes);
      let v = this.#view["get" + (signed ? "I" : "Ui") + "nt" + (bytes * 8)](this.#offset, this.#endianness);
      this.#offset += bytes;
      return v;
   }
   
   viewFromHere(size) {
      if (size !== undefined && size !== null) {
         size = +size;
         if (isNaN(size) || size !== Math.floor(size))
            throw new TypeError("The size, if specified, must be an integer.");
         if (size < 0 || size + this.#offset > this.length)
            throw new Error("Out-of-bounds slice (from "+this.#offset+" past "+this.length+" with desired length "+size+").");
      } else {
         size = this.remaining;
      }
      return new DataView(
         this.#view.buffer,
         this.#view.byteOffset + this.#offset,
         size
      );
   }
   bytestreamFromHere(size) {
      return new Bytestream(this.viewFromHere(size));
   }
   
   readFourCC() {
      return this.#read_sized_string(4);  
   }
   readEightCC() {
      return this.#read_sized_string(8);  
   }
   readSizedString(size) {
      if (size < 0)
         throw new Error("Invalid size.");
      return this.#read_sized_string(size);  
   }
   
   readUint8() {
      return this.#read_unsigned_int(1, false);
   }
   readUint16() {
      return this.#read_unsigned_int(2, false);
   }
   readUint32() {
      return this.#read_unsigned_int(4, false);
   }
   readInt8() {
      return this.#read_unsigned_int(1, true);
   }
   readInt16() {
      return this.#read_unsigned_int(2, true);
   }
   readInt32() {
      return this.#read_unsigned_int(4, true);
   }
   
   readLengthPrefixedString(prefix_width) {
      switch (prefix_width) {
         case 1:
         case 2:
         case 4:
            break;
         default:
            throw new Error("Invalid string prefix width.");
      }
      let size = this.#read_unsigned_int(prefix_width);
      return this.#read_sized_string(size);
   }
   
   skipBytes(size) {
      if (size < 0)
         throw new Error("Invalid size.");
      if (size > this.remaining)
         throw new Error("Out of bounds skip.");
      this.#offset += size;
   }
};