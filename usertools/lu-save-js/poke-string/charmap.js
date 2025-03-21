class CharmapSubset {
   constructor() {
      this.entities = {
         canonical: Object.create(null), // this.entities[name] == [byte, byte, ...]
         lowercase: Object.create(null),
      };
      this.glyphs = {
         by_byte: Object.create(null),
         by_char: Object.create(null),
      };
   }
   define_entity(name, /*Array*/ bytes) {
      if (+bytes === bytes)
         bytes = [bytes];
      this.entities.canonical[name] = bytes;
      this.entities.lowercase[name.toLowerCase()] = bytes;
   }
   define_glyph(byte, name) {
      this.glyphs.by_byte[byte] = name;
      this.glyphs.by_char[name] = byte;
   }
   define_glyph_run(base, str) {
      for(let i = 0; i < str.length; ++i) {
         let cc = base + i;
         let ch = str[i];
         this.define_glyph(cc, ch);
      }
   }
   
   lookup_entity(name) {
      let dfn = this.entities.canonical[name];
      if (dfn)
         return dfn;
      //
      // Prefer canonical-case lookups first, in case two entities have 
      // the same name with different letter cases; then attempt a 
      // case-insensitive lookup.
      //
      return this.entities.lowercase[name.toLowerCase()];
   }
   canonical_entity_name_for(bytes) {
      let map = this.entities.canonical;
      if (+bytes === bytes) {
         for(let name in map) {
            let value = map[name];
            if (value.length == 1 && value[0] == bytes)
               return name;
         }
         return null;
      }
      for(let name in map) {
         let value = map[name];
         if (value.length != bytes.length)
            continue;
         let match = true;
         for(let i = 0; i < value.length; ++i) {
            if (value[i] != bytes[i]) {
               match = false;
               break;
            }
         }
         if (match)
            return name;
      }
      return null;
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
         charset.define_glyph(0xFA, "\v"); // \l // Scroll the text upward.
         charset.define_glyph(0xFB, "\f"); // \p // Start a new paragraph.
         charset.define_glyph(0xFE, "\n"); // \n
         charset.define_glyph(0xFF, "\0");
         
         // Extended characters.
         charset.define_glyph_run(0x0100, "🡅🡇🡄🡆＋");
         charset.define_entity("Lv", [0xF9, 0x05]); // "Lv." glyph
         charset.define_entity("PP", [0xF9, 0x06]); // "PP" glyph
         charset.define_entity("ID", [0xF9, 0x07]);
         charset.define_glyph_run(0x0108, "№＿①②③④⑤⑥⑦⑧⑨()◎△"); // "No." glyph
         charset.define_glyph_run(0x01D2, "¯〜（）⊂＞");
         // 0x01D8: angry eye, left
         // 0x01D9: angry eye, right
         charset.define_glyph_run(0x01DA, "@；➕➖🟰");
         // 0x01DF: spiral
         charset.define_glyph(0x01E0, "👅");
         charset.define_glyph(0x01E1, "▵");
         charset.define_glyph_run(0x01E2, "´`●▼■");
         charset.define_glyph(0x01E7, "❤︎");
         charset.define_glyph(0x01E8, "🌙");
         charset.define_glyph(0x01E9, "♪");
         charset.define_glyph(0x01EA, "◓");
         charset.define_glyph(0x01EB, "🗲");
         charset.define_glyph(0x01EC, "🍃");
         charset.define_glyph(0x01ED, "🔥");
         charset.define_glyph(0x01EE, "💧");
         charset.define_glyph(0x01EF, "🤛");
         charset.define_glyph(0x01F0, "🤜");
         charset.define_glyph(0x01F1, "❀");
         // 0x01F2: small eye?
         // 0x01F3: medium eye, looking down and to the right?
         charset.define_glyph(0x01F4, "💢");
         // 0x01F5: emoji, mischievous
         // 0x01F6: emoji, happy
         // 0x01F7: emoji, angry
         // 0x01F8: emoji, surprised
         // 0x01F9: emoji, ^_^
         // 0x01FA: emoji, evil
         // 0x01FB: emoji, tired
         // 0x01FC: emoji, neutral
         // 0x01FD: emoji, shocked
         // 0x01FE: emoji, rage
      }
      {
         let charset = this.latin;
         charset.define_glyph_run(0x00, " ÀÁÂÇÈÉÊËÌÎÏÒÓÔŒÙÚÛÑßàá");
         charset.define_glyph_run(0x19, "çèéêëì");
         charset.define_glyph_run(0x20, "îïòóôœùúûñºª");
         charset.define_entity("superer", [0x2C]); // superscript "er"
         charset.define_glyph_run(0x2D, "&+");
         charset.define_entity("Lv", [0x34]); // "Lv." glyph
         charset.define_glyph(0x35, "=");
         charset.define_glyph(0x36, ";");
         charset.define_glyph(0x51, "¿");
         charset.define_glyph(0x52, "¡");
         charset.define_entity("pk", [0x53]);
         charset.define_entity("mn", [0x54]);
         charset.define_entity("PokeblockPO", [0x55]);
         charset.define_entity("PokeblockKE", [0x56]);
         charset.define_entity("PokeblockBL", [0x57]);
         charset.define_entity("PokeblockOC", [0x58]);
         charset.define_entity("PokeblockK",  [0x59]);
         charset.define_entity("Pokeblock", [0x55, 0x56, 0x57, 0x58, 0x59]);
         charset.define_glyph_run(0x5A, "Í%()");
         charset.define_glyph(0x68, "â");
         charset.define_glyph(0x6F, "í");
         charset.define_entity("UnkSpacer", [0x77]);
         charset.define_glyph_run(0x79, "↑↓←→");
         charset.define_glyph(0x84, "ᵉ");
         charset.define_glyph_run(0x85, "<>");
         charset.define_entity("superre", [0xA0]); // superscript "re"
         charset.define_glyph_run(0xAB, "!?.-·…“”‘’♂♀¥,");
         charset.define_glyph_run(0xBB, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"); // [BB, EE]
         
         // Extended characters.
         charset.define_glyph(0x0117, "×");
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
         
         // Extended characters.
         //charset.define_glyph(0x01CC, "×"); // R button
         charset.define_glyph_run(0x01CD, "大小゛");
      }
      this.string_terminator = this.common.glyphs.by_char['\0'];
      
      this.simple_escape_sequences = new Map();
   }
   
   get_character_set(name) {
      if (!name)
         return this.latin;
      if (name != "latin" && name != "japanese")
         throw new Error("Invalid character set name.");
      return this[name];
   }
   
   codepoint_to_character(cc, charset) {
      let ch = this.common.glyphs.by_byte[cc];
      if (ch)
         return ch;
      ch = this.get_character_set(charset).glyphs.by_byte[cc];
      return ch || null;
   }
   character_to_codepoint(ch, charset) {
      let cc = this.common.glyphs.by_char[ch];
      if (cc || cc === 0)
         return cc;
      cc = this.get_character_set(charset).glyphs.by_char[ch];
      if (cc || cc === 0)
         return cc;
      return null;
   }
   lookup_entity(name, charset) {
      let bytes = this.common.lookup_entity(name);
      if (bytes === 0)
         return bytes;
      if (!bytes)
         bytes = this.get_character_set(charset).lookup_entity(name);
      if (bytes === 0)
         return bytes;
      return bytes || null;
   }
};
const CHARMAP = new Charmap();