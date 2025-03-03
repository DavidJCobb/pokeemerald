
const FLASH_SECTOR_SIZE = 0x1000;
const FLASH_MEMORY_SIZE = FLASH_SECTOR_SIZE * 32;

const SAVE_SECTOR_FULL_SIZE = FLASH_SECTOR_SIZE;
const SAVE_SECTOR_INFO_SIZE = 128;
const SAVE_SECTOR_DATA_SIZE = SAVE_SECTOR_FULL_SIZE - SAVE_SECTOR_INFO_SIZE;

const SAVE_SECTOR_SIGNATURE = 0x08012025;

const SAVE_SLOT_COUNT       = 2;
const SAVE_SECTORS_PER_SLOT = 14;

const RTC_DATA_SIZE = 16;

/*[[noreturn]]*/ function refuse_abstract_instantiation(cls) {
   if (new.target === cls) {
      throw new Error(cls.name + " is an abstract class and cannot be directly instantiated.");
   }
}
/*[[noreturn]]*/ function purecall() {
   throw new Error("You were supposed to override this function on your subclass!");
}
/*[[noreturn]]*/ function unreachable() {
   throw new Error("This code location should be unreachable.");
}

function assert_xml_validity(condition, message) {
   if (!condition)
      throw new Error(message);
}
function assert_logic(condition, message) {
   if (!condition)
      throw new Error(message);
}
function assert_type(condition, message) {
   if (!condition)
      throw new TypeError(message);
}

function memcpy(/*Variant<DataView, ArrayBuffer>*/ dst, /*Variant<DataView, ArrayBuffer>*/ src, size) {
   let dst_view = dst instanceof DataView ? dst : new DataView(dst);
   let src_view = src instanceof DataView ? src : new DataView(src);
   let i;
   for(i = 0; i + 3 < size; i += 4) {
      dst_view.setUint32(i, src_view.getUint32(i));
   }
   for(; i < size; ++i) {
      dst_view.setUint8(i, src_view.getUint8(i));
   }
}

// browsers don't give you a good way to inspect DataViews
function debug_dump_buffer(/*DataView*/ view, bytes_per_row) {
   if (isNaN(+bytes_per_row) || bytes_per_row <= 0) {
      bytes_per_row = 16;
   }
   let str = "";
   for(let i = 0; i < view.byteLength; ++i) {
      if (!(i % bytes_per_row)) {
         if (i)
            str += '\n';
         str += i.toString(16).toUpperCase().padStart(8, '0');
         str += " | ";
      }
      let byte = view.getUint8(i);
      if (i % bytes_per_row)
         str += ' ';
      str += byte.toString(16).toUpperCase().padStart(2, '0');
   }
   console.log(str);
}

//
// CHECKSUM CODE:
//

// This is a reference implementation for the varying kinds of simple 
// checksums we tend to see in the game's savedata. Prefer defining a 
// dedicated function instead of using this one, since JS doesn't have 
// constexpr.
function checksum(
   /*size_t*/ accumulator_bit_width,
   /*size_t*/ unit_bit_width,
   /*size_t*/ result_bit_width,
   /*const DataView*/ blob,
   /*Optional<size_t>*/ length
) {
   console.assert(unit_bit_width % 8 == 0);
   console.assert(result_bit_width == 8 || result_bit_width == 16 || result_bit_width == 32);
   console.assert(accumulator_bit_width <= 32); // JS bitwise operations coerce to int32_t
   console.assert(result_bit_width <= 32); // JS bitwise operations coerce to int32_t
   
   if (!length)
      length = blob.byteLength;
   
   const bytes_per_unit   = unit_bit_width / 8;
   const accumulator_mask = (accumulator_bit_width == 32) ? 0xFFFFFFFF : (1 << accumulator_bit_width) - 1;
   const result_mask      = (result_bit_width == 32) ? 0xFFFFFFFF : (1 << result_bit_width) - 1;
   
   let checksum = 0;
   for(let i = 0; i < length; i += bytes_per_unit) {
      checksum += blob["getUint" + unit_bit_width](i, true);
      checksum &= accumulator_mask;
   }
   if (result_bit_width < accumulator_bit_width) {
      //
      // Fold the bit widths.
      //
      if (result_bit_width == 8) {
         let c = 0;
         for(let i = 0; i < 4; ++i)
            c += (checksum >> (i * 8)) & 0xFF;
         checksum = c;
      } else if (result_bit_width == 16) {
         let c = 0;
         for(let i = 0; i < 2; ++i)
            c += (checksum >> (i * 16)) & 0xFFFF;
         checksum = c;
      }
   }
   return checksum;
}

//
// Notation: AxUyRz = x-bit accumulator, Y-bit units, Z-bit result
//
// The unit and result widths default to the accumulator width, so they 
// can be omitted when they don't differ. A32U32R16 == A32R16.
//
// Where all values are the same, we omit the letters and just write the 
// one number.
//

/*uint16_t*/ function checksum16(/*const DataView*/ blob, /*Optional<size_t>*/ length) {
   if (!length)
      length = blob.byteLength;
   let checksum = 0;
   for(let i = 0; i + 1 < length; i += 2) {
      checksum += blob.getUint16(i, true);
      checksum &= 0xFFFF; // constrain to 16-bit int
   }
   return checksum;
}

/*int32_t*/ function checksum32(/*const DataView*/ blob, /*Optional<size_t>*/ length) {
   if (!length)
      length = blob.byteLength;
   let checksum = 0;
   for(let i = 0; i + 3 < length; i += 4) {
      checksum += blob.getUint32(i, true);
      checksum |= 0; // constrain to 32-bit int
   }
   return checksum;
}

/*int32_t*/ function checksumA32U8(/*const DataView*/ blob, /*Optional<size_t>*/ length) {
   if (!length)
      length = blob.byteLength;
   let checksum = 0;
   for(let i = 0; i < length; ++i) {
      checksum += blob.getUint8(i);
      checksum |= 0; // constrain to 32-bit int
   }
   return checksum;
}

// Seen on save sectors.
/*uint16_t*/ function checksumA32R16(/*const DataView*/ blob, /*Optional<size_t>*/ length) {
   let checksum = checksum32(blob, length);
   checksum = ((checksum >> 16) + checksum) & 0xFFFF;
   return checksum;
}