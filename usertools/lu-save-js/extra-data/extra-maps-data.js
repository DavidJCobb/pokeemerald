export default class ExtraMapsData {
   constructor() {
      this.map_sections = {
         by_name:  new Map(), // Map<String, int>
         by_index: new Map(), // Map<int, String>
      };
      this.map_groups = [];
   }
   
   #parse_name_block(/*Bytestream*/ body, uses_prefix) {
      let prefix = "";
      if (uses_prefix) {
         prefix = body.readLengthPrefixedString(1);
      }
      let block_size = body.readUint32();
      let block_view = body.viewFromHere(block_size);
      body.skipBytes(block_size);
      let decoder    = new TextDecoder();
      let names      = decoder.decode(block_view).split("\0");
      if (prefix)
         for(let i = 0; i < names.length; ++i)
            names[i] = prefix + names[i];
      return names;
   }
   
   // Parse enum data from a MAPSDATA subrecord.
   parse(/*Bytestream*/ body) {
      let group_names = this.#parse_name_block(body, true);
      for(let name of group_names) {
         let group = {
            name: name,
            maps: []
         };
         this.map_groups.push(group);
         group.maps = this.#parse_name_block(body, true);
      }
   }
};