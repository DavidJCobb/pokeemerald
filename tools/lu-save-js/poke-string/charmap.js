class CharmapSubset {
   constructor() {
      this.entities = {
         canonical: Object.create(null), // this.entities[name] == [byte, byte, ...]
         lowercase: Object.create(null),
      };
      this.glyphs = {
         by_byte: Object.create(null),
         by_name: Object.create(null),
      };
   }
   define_entity(name, /*Array*/ bytes) {
      this.entities.canonical[name] = bytes;
      this.entities.lowercase[name.toLowerCase()] = bytes;
   }
   define_glyph(byte, name) {
      this.glyphs.by_byte[byte] = name;
      this.glyphs.by_name[name] = byte;
   }
   define_glyph_run(base, str) {
      for(let i = 0; i < str.length; ++i) {
         let cc = base + i;
         let ch = str[i];
         this.define_glyph(cc, ch);
      }
   }
   
   lookup_entity(name) {
      return this.entities.lowercase[name.toLowerCase()];
   }
};

class Charmap {
   constructor() {
      this.common   = new CharmapSubset();
      this.latin    = new CharmapSubset();
      this.japanese = new CharmapSubset();
      {
         let charset = this.common;
         charset.define_glyph_run(0xA1, "0123456789");
         charset.define_glyph_run(0xB5, "♂♀");
         charset.define_glyph_run(0xB9, "×/");
         charset.define_glyph_run(0xEF, "▶:ÄÖÜäöü"); // [EF, F6]
         
         //
         // TODO: Define extended charcodes (0x0100 onward) here, and modify 
         //       PokeStringPrinter to use them. Several of these codes are 
         //       only available in the Latin fonts, but others are common to 
         //       both encodings.
         //
      }
      {
         let charset = this.latin;
         charset.define_glyph_run(0x00, " ÀÁÂÇÈÉÊËÌÎÏÒÓÔŒÙÚÛÑßàá");
         charset.define_glyph_run(0x19, "çèéêëì");
         charset.define_glyph_run(0x20, "îïòóôœùúûñºª");
         charset.define_glyph(0x2C, "SUPER_ER");
         charset.define_glyph_run(0x2D, "&+");
         charset.define_glyph(0x34, "LV"); // "Lv." glyph
         charset.define_glyph(0x35, "=");
         charset.define_glyph(0x36, ";");
         charset.define_glyph(0x51, "¿");
         charset.define_glyph(0x52, "¡");
         charset.define_glyph(0x53, "PKMN_PK");
         charset.define_glyph(0x54, "PKMN_MN");
         charset.define_glyph(0x55, "POKEBLOCK_PO");
         charset.define_glyph(0x56, "POKEBLOCK_KE");
         charset.define_glyph(0x57, "POKEBLOCK_BL");
         charset.define_glyph(0x58, "POKEBLOCK_OC");
         charset.define_glyph(0x59, "POKEBLOCK_K");
         charset.define_glyph_run(0x5A, "Í%()");
         charset.define_glyph(0x68, "â");
         charset.define_glyph(0x6F, "í");
         charset.define_glyph(0x77, "UNK_SPACER");
         charset.define_glyph(0x79, "UP_ARROW");
         charset.define_glyph(0x7A, "DOWN_ARROW");
         charset.define_glyph(0x7B, "LEFT_ARROW");
         charset.define_glyph(0x7C, "RIGHT_ARROW");
         charset.define_glyph(0x84, "SUPER_E");
         charset.define_glyph_run(0x85, "<>");
         charset.define_glyph(0xA0, "SUPER_RE");
         charset.define_glyph_run(0xAB, "!?.-·…“”‘’♂♀¥,");
         charset.define_glyph_run(0xBB, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"); // [BB, EE]
         
         //
         // TODO: Define extended charcodes (0x0100 onward) here, and modify 
         //       PokeStringPrinter to use them. Several of these codes are 
         //       only available in the Latin fonts.
         //
      }
      {
         let charset = this.japanese;
         //
         // Hiragana
         //
         charset.define_glyph_run(0x00, "　あいうえおかきくけこさしすせそたちつてとなにぬねのはひふへほまみむめもやゆよらりるれろわをんぁぃぅぇぉゃゅょがぎぐげござじずぜぞだぢづでどばびぶべぼぱぴぷぺぽっ");
         //
         // Katakana
         //
         charset.define_glyph_run(0x51, "アイウエオカキクケコサシスセソタチツテトナニヌネノハヒフヘホマミムメモヤユヨラリルレロワヲンァィゥェォャュョガギグゲゴザジズゼゾダヂヅデドバビブベボパピプペポッ");
         //
         // A1 through AA are common (Arabic numerals).
         //
         charset.define_glyph_run(0xAB, "！？。ー\u2126‥『』「」");
         charset.define_glyph_run(0xB7, "円．×");
         charset.define_glyph_run(0xBB, "ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ"); // [BB, EE]
      }
   }
};
const CHARMAP = new Charmap();