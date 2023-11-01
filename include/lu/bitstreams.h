#ifndef GUARD_LU_BITSTREAMS
#define GUARD_LU_BITSTREAMS

struct lu_BitstreamState {
   u8* target;
   u8  shift; // offset within target byte
};

extern void lu_BitstreamInitialize(struct lu_BitstreamState*, u8* target);

//
// READING:
//

// Read a single bit.
extern bool8 lu_BitstreamRead_bool(struct lu_BitstreamState*);

extern u8 lu_BitstreamRead_u8(struct lu_BitstreamState*, u8 bitcount);
extern u16 lu_BitstreamRead_u16(struct lu_BitstreamState*, u8 bitcount);
extern u32 lu_BitstreamRead_u32(struct lu_BitstreamState*, u8 bitcount);

extern s8 lu_BitstreamRead_s8(struct lu_BitstreamState*, u8 bitcount);
extern s16 lu_BitstreamRead_s16(struct lu_BitstreamState*, u8 bitcount);
extern s32 lu_BitstreamRead_s32(struct lu_BitstreamState*, u8 bitcount);

extern void lu_BitstreamRead_string(struct lu_BitstreamState*, u8* value, u16 max_length, u8 length_bitcount);
extern void lu_BitstreamRead_string_optional_terminator(struct lu_BitstreamState*, u8* value, u16 max_length);

extern void lu_BitstreamRead_buffer(struct lu_BitstreamState*, void* value, u16 bytecount);

//
// WRITING:
//

// Write a single bit.
extern void lu_BitstreamWrite_bool(struct lu_BitstreamState*, bool8 value);

// Write up to eight bits. When you wish to write signed values, you should either: 
// make sure to include the sign bit; or increase the signed value by its negated 
// minimum (e.g. if the minimum is -5, add 5) before serializing and then decrement 
// after serializing.
extern void lu_BitstreamWrite_u8(struct lu_BitstreamState*, u8 value, u8 bitcount);

// Write up to sixteen bits.
//
// This function assumes that `bitcount` will be greater than 8, and WILL NOT FUNCTION PROPERLY 
// if that assumption is not true.
extern void lu_BitstreamWrite_u16(struct lu_BitstreamState*, u16 value, u8 bitcount);

// Write up to thirty-two bits.
//
// This function assumes that `bitcount` will be greater than 16, and WILL NOT FUNCTION PROPERLY 
// if that assumption is not true.
extern void lu_BitstreamWrite_u32(struct lu_BitstreamState*, u32 value, u8 bitcount);

extern void lu_BitstreamWrite_string(struct lu_BitstreamState*, u8* value, u16 max_length, u8 length_bitcount);
extern void lu_BitstreamWrite_string_optional_terminator(struct lu_BitstreamState*, u8* value, u16 max_length);

extern void lu_BitstreamWrite_buffer(struct lu_BitstreamState*, const void* value, u16 bytecount);

#endif