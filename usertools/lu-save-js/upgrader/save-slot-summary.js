import CArrayInstance from "../c-array-instance.js";
import CStructInstance from "../c-struct-instance.js";
import CValueInstance from "../c-value-instance.js";
import PokeString from "../poke-string/poke-string.js";
import DOMPokeStringPrinter from "../poke-string/poke-string-dom-printer.js";

let SaveSlotSummary;
{
   class StringSummary {
      constructor() {
         this.data    = null;    // Optional<PokeString>
         this.charset = "latin"; // Variant<"latin", "japanese">
      }
      
      set_charset_by_language(lang) {
         if (lang === 1)
            this.charset = "japanese";
         else
            this.charset = "latin";
      }
      
      get empty() {
         return !this.data;
      }
      
      /*DocumentFragment*/ to_dom() /*const*/ {
         if (!this.data)
            return new DocumentFragment();
         let printer = new DOMPokeStringPrinter();
         printer.charset = this.charset || "latin";
         printer.print(this.data);
         
         let node = document.createElement("span");
         node.classList.add("pokestring");
         node.append(printer.result);
         return node;
      }
   };
   
   function get_sfi_enum(/*IndexedSaveFormatInfo*/ sfi, name) {
      return sfi?.extra_data?.enums.get(name);
   }
   function get_save_script_flag(/*SaveSlot*/ slot, /*IndexedSaveFormatInfo*/ format, /*String*/ name) {
      if (name.startsWith("FLAG_"))
         name = name.substring(5);
      if (!format)
         return null;
      let flag_index;
      {
         flag_index = format.extra_data?.script_flags?.flag_values.get(name)
         if (!flag_index && flag_index !== 0)
            return null;
      }
      let inst = slot.lookupCInstanceByPath("gSaveBlock1Ptr->flags");
      if (!(inst instanceof CArrayInstance))
         return null;
      
      let bits_per_unit;
      {
         let unit = inst.values[0];
         if (!unit)
            return null;
         bits_per_unit = unit.decl.options.bitcount;
         if (!bits_per_unit)
            return null;
      }
      
      let unit_index = Math.floor(flag_index / bits_per_unit);
      let bit_index  = flag_index % bits_per_unit;
      
      let u = inst.values[unit_index];
      if (!u || typeof u.value !== "number")
         return null;
      return (u.value & (1 << bit_index)) != 0;
   }
   
   class PokemonSummary {
      constructor(/*CStructInstance<Variant<Pokemon, BoxPokemon>>*/ inst, /*IndexedSaveFormatInfo*/ sfi) {
         this.is_egg   = null; // Optional<bool>
         this.level    = null; // Optional<int>
         this.nickname = new StringSummary();
         this.species  = {
            id:   null, // Optional<int>
            name: null, // Optional<String>
         };
         
         let box;
         if (inst.type.symbol == "Pokemon" || inst.type.tag == "Pokemon") {
            box = inst.members["box"];
         } else if (inst.type.symbol == "SerializedBoxPokemon" || inst.type.tag == "SerializedBoxPokemon") {
            box = inst;
         }
         
         {  // is egg
            let memb = box.members["isEgg"];
            if (memb)
               this.is_egg = memb.value;
         }
         {  // level
            if (inst === box) {
               //
               // `Pokemon` caches the current level, but `BoxPokemon` stores 
               // only the total earned EXP. To compute the level given only 
               // a `BoxPokemon`, we'd need to know the species' EXP curve and 
               // the values for that curve. We do not currently have that 
               // information.
               //
            } else {
               let memb = inst.members["level"];
               if (memb)
                  this.level = memb.value;
            }
         }
         {  // nickname
            let v = box.members.nickname?.value;
            if (v instanceof PokeString) {
               this.nickname.data = v;
            }
            this.nickname.set_charset_by_language(box.members.language?.value);
         }
         {  // species
            let memb = box.members.substructs?.members.type0?.members.species;
            if (memb) {
               let v = memb.value;
               if (v || v === 0) {
                  this.species.id = v;
                  //
                  // Query the name.
                  //
                  let enumeration = get_sfi_enum(sfi, "SPECIES");
                  if (enumeration) {
                     let name = enumeration.members.by_value.get(this.species.id);
                     if (name)
                        this.species.name = name;
                  }
               }
            }
         }
      }
      
      /*bool*/ get empty() {
         return !this.species.id;
      }
      
      static /*bool*/ object_is_a_pokemon(/*CInstance*/ inst) {
         if (!inst)
            return false;
         if (!(inst instanceof CStructInstance))
            return false;
         if (!inst.type)
            return false;
         let names = [ inst.type.symbol, inst.type.tag ];
         for(let name of names) {
            if (name == "SerializedBoxPokemon")
               return true;
            if (name == "Pokemon") {
               let memb = inst.members["box"];
               return PokemonSummary.object_is_a_pokemon(memb);
            }
         }
         return false;
      }
   };
   
   SaveSlotSummary = class SaveSlotSummary {
      #slot;
      
      static PokemonSummary = PokemonSummary;
      
      constructor(/*IndexedSaveFormatInfo*/ sfi, /*const SaveSlot*/ slot) {
         this.#slot = slot;
         this.player = {
            name:   new StringSummary(),
            gender: null, // Optional<int>
            badges: null,
            trainer_id: {
               full:    null, // Optional<int>
               visible: null, // Optional<String>
            },
            location: {
               map_group: this.#extract_integer_field("gSaveBlock1Ptr->location.mapGroup"),
               map_num:   this.#extract_integer_field("gSaveBlock1Ptr->location.mapNum"),
               x:         this.#extract_integer_field("gSaveBlock1Ptr->pos.x"),
               y:         this.#extract_integer_field("gSaveBlock1Ptr->pos.y"),
            },
         };
         this.inventory = {
            money: this.#extract_integer_field("gSaveBlock1Ptr->money"),
            coins: this.#extract_integer_field("gSaveBlock1Ptr->coins"),
            items: {
               general:    [],
               balls:      [],
               tms_hms:    [],
               berries:    [],
               key:        [],
               pokeblocks: [],
            }
         };
         this.party = []; // Array<PokemonSummary>
         //
         // Extract data.
         //
         this.player.name.data = this.#extract_string_field("gSaveBlock2Ptr->playerName");
         this.player.gender    = this.#extract_integer_field("gSaveBlock2Ptr->playerGender");
         {  // trainer ID
            let inst = this.#find_value("gSaveBlock2Ptr->playerTrainerId");
            if (inst instanceof CArrayInstance) {
               let bits_per_unit;
               {
                  let unit = inst.values[0];
                  if (unit)
                     bits_per_unit = unit.decl?.options.bitcount;
               }
               if (bits_per_unit) {
                  let merged = 0;
                  for(let i = 0; i < inst.values.length; ++i) {
                     let v = inst.values[i].value;
                     if (!v || +v !== v)
                        break;
                     merged |= v << (i * 8);
                  }
                  if (merged < 0) {
                     //
                     // JavaScript bitwise operations coerce to int32_t; we 
                     // need it unsigned.
                     //
                     merged += 0xFFFFFFFF + 1;
                  }
                  this.player.trainer_id.full = merged;
               }
            } else if (inst instanceof CValueInstance) {
               //
               // Preemptively handle hacks that may refactor the trainer ID to 
               // a single uint32_t.
               //
               let v = inst.value;
               if (+v === v)
                  this.player.trainer_id.full = v;
            }
            if (this.player.trainer_id.full !== null) {
               let s = (this.player.trainer_id.full & 0xFFFF).toString().padStart(5, '0');
               s = s.substring(s.length - 5);
               this.player.trainer_id.visible = s;
            }
         }
         {  // player badges
            let any  = false;
            let list = [];
            for(let i = 0; i < 8; ++i) {
               let name = `FLAG_BADGE${(i + 1).toString().padStart(2, '0')}_GET`;
               let flag = get_save_script_flag(slot, sfi, name);
               list.push(flag);
               if (flag !== null)
                  any = true;
            }
            if (any)
               this.player.badges = list;
         }
         {  // player party
            let inst = this.#find_value("gSaveBlock1Ptr->playerParty");
            if (inst instanceof CArrayInstance && inst.values) {
               for(let elem of inst.values) {
                  if (!PokemonSummary.object_is_a_pokemon(elem))
                     continue;
                  let summ = new PokemonSummary(elem, sfi);
                  if (!summ.empty) {
                     this.party.push(summ);
                  }
               }
            }
         }
         //
         // Inventory
         //
         {  // Bag
            let names = {
               general: "gSaveBlock1Ptr->bagPocket_Items",
               key:     "gSaveBlock1Ptr->bagPocket_KeyItems",
               balls:   "gSaveBlock1Ptr->bagPocket_PokeBalls",
               tms_hms: "gSaveBlock1Ptr->bagPocket_TMHM",
               berries: "gSaveBlock1Ptr->bagPocket_Berries",
            };
            for(let k in names) {
               let memb = this.#find_value(names[k]);
               if (!(memb instanceof CArrayInstance))
                  continue;
               for(let item of memb.values) {
                  if (!(item instanceof CStructInstance))
                     continue;
                  let type = item.members.itemId?.value;
                  let quan = item.members.quantity?.value;
                  if (+type !== type || +quan !== quan)
                     continue;
                  if (type === 0)
                     continue;
                  this.inventory.items[k].push({
                     item_id:  type,
                     quantity: quan,
                  });
               }
            }
         }
         {  // Pokeblocks
            let MEMBERS = [
               "color",
               "spicy",
               "dry",
               "sweet",
               "bitter",
               "sour",
               "feel",
            ];
            let inst = this.#find_value("gSaveBlock1Ptr->pokeblocks");
            if (inst instanceof CArrayInstance) {
               for(let item of inst.values) {
                  if (!(item instanceof CStructInstance))
                     continue;
                  let block = {};
                  for(let name in MEMBERS) {
                     let v = item.members[name]?.value;
                     if (+v !== v)
                        continue;
                     block[name] = v;
                  }
                  if (!block.color) // constant PBLOCK_CLR_NONE, but it's an enum member, so we can't dump it yet
                     continue;
                  this.inventory.pokeblocks.push(block);
               }
            }
         }
         console.log(this);
      }
      
      /*Optional<CInstance>*/ #find_value(path) {
         return this.#slot.lookupCInstanceByPath(path);
      }
      
      /*Optional<PokeString>*/ #extract_string_field(path) {
         let inst = this.#find_value(path);
         if (!inst || !(inst.value instanceof PokeString)) {
            return null;
         }
         return inst.value;
      }
      /*Optional<int>*/ #extract_integer_field(path) {
         let inst = this.#find_value(path);
         if (!inst)
            return null;
         let v = inst.value;
         if (v === null || typeof v !== "number")
            return null;
         return v;
      }
   };
}
export default SaveSlotSummary;