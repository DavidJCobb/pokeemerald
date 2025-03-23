import CArrayInstance from "../c/c-array-instance.js";
import CInstance from "../c/c-instance.js";
import CStructInstance from "../c/c-struct-instance.js";
import CTypeInstance from "../c/c-type-instance.js";
import CUnionInstance from "../c/c-union-instance.js";
import CValueInstance from "../c/c-value-instance.js";
import LiteralPokeStringPrinter from "../poke-string/literal-printer.js";
import PokeString from "../poke-string/poke-string.js";

class CViewElement extends TreeRowViewElement {
   static Model = class Model extends TreeRowViewModel {
      constructor(owner, formatters) {
         super();
         
         this.owner = owner;
         this.scope = null;
         this.typed_value_formatters = formatters;
         
         this.columns = [
            { name: "Name",  width:  "3fr", minWidth: "6em" },
            { name: "Value", width:  "1fr", minWidth: "6em" },
            { name: "Type",  width: "30ch", minWidth: "6em" },
         ];
      }
      
      /*CInstance*/ getItem(inst, row) /*const*/ {
         if (inst instanceof CArrayInstance)
            return inst.values[row];
         if (inst instanceof CStructInstance) {
            let i = 0;
            for(let key in inst.members) {
               if (i == row)
                  return inst.members[i];
               ++i;
            }
            return null;
         }
         if (inst instanceof CUnionInstance) {
            if (row == 0)
               return inst.value;
         }
         return null;
      }
      /*Optional<Variant<Array<CInstance>, Map<String, T>>>*/ getItemChildren(/*CInstance*/ inst) /*const*/ {
         if (inst === null && this.scope) {
            return this.getItemChildren(this.scope);
         }
         if (inst instanceof CValueInstance)
            return null;
         if (inst instanceof CArrayInstance)
            return inst.values;
         if (inst instanceof CStructInstance) {
            return new Map(Object.entries(inst.members));
         }
         if (inst instanceof CUnionInstance) {
            if (!inst.value)
               return null;
            let map = new Map();
            map.set(inst.value.decl.name, inst.value);
            return map;
         }
         return null;
      }
      /*bool*/ itemHasChildren(/*const CInstance*/ inst) /*const*/ {
         if (inst instanceof CValueInstance)
            return false;
         if (inst instanceof CArrayInstance)
            return inst.values.length > 0;
         if (inst instanceof CStructInstance) {
            for(let key in inst.members)
               return true;
            return false;
         }
         if (inst instanceof CUnionInstance)
            return !!inst.value;
         return false;
      }
      
      stringify_name(inst, is_selected) {
         function _style(name, text) {
            text = (text+"").replaceAll("[", "[raw][[/raw]");
            if (is_selected)
               return text;
            return `[style=${name}]${text}[/style]`;
         }
         
         if (inst.decl) {
            let aggr = inst.is_member_of;
            if (aggr instanceof CArrayInstance) {
               //
               // Check for a display override.
               //
               {
                  let format  = inst.decl.save_format;
                  let display = null;
                  for(let item of format.display_overrides) {
                     if (!item.overrides.display_index)
                        continue;
                     if (item.matches(inst)) {
                        display = item;
                        break;
                     }
                  }
                  if (display) {
                     let text = display.overrides.display_index(inst);
                     if (text !== null) {
                        return _style("name-text", text);
                     }
                  }
               }
               //
               // No override.
               //
               return _style("name-text", aggr.values.indexOf(inst));
            }
            return _style("name-text", inst.decl.name);
         }
         return "[raw][[/raw]unnamed]";
      }
      
      stringify_value(inst, is_selected) {
         {
            let format  = inst.decl.save_format;
            let display = null;
            for(let item of format.display_overrides) {
               if (!item.overrides.display_string)
                  continue;
               if (item.matches(inst)) {
                  display = item;
                  break;
               }
            }
            if (display) {
               let text = display.overrides.display_string(inst);
               if (text !== null) {
                  if (is_selected) {
                     text = parseBBCodeToPlainText(text).replaceAll("[", "[raw][[/raw]");
                  }
                  return text;
               }
            }
         }
         
         function _style(name, text) {
            text = (text+"").replaceAll("[", "[raw][[/raw]");
            if (is_selected)
               return text;
            return `[style=${name}]${text}[/style]`;
         }
         
         if (inst instanceof CStructInstance) {
            let type      = inst.type;
            let typename  = type.tag || type.symbol;
            let formatter = this.typed_value_formatters[typename];
            if (formatter) {
               let text;
               try {
                  text = formatter(inst);
               } catch (e) {
                  console.log(e);
                  return _style("error", "[custom formatter error]");
               }
               if (is_selected) {
                  text = parseBBCodeToPlainText(text).replaceAll("[", "[raw][[/raw]");
               } else {
                  text = `[style=value-text]${text}[/style]`;
               }
               return text;
            }
         }
         if (inst instanceof CArrayInstance) {
            let decl = inst.decl;
            switch (decl.type) {
               case "boolean":
               case "integer":
               case "string":
                  break;
               case "omitted":
                  return _style("deemphasize", "[omitted]");
               default:
                  return "[raw][[/raw] " + _style("deemphasize", "...") + " ]";
            }
            if (inst.rank + 1 < decl.array_extents.length)
               return null;
            let text   = "[raw][[/raw] ";
            let extent = decl.array_extents[inst.rank];
            for(let i = 0; i < extent; ++i) {
               if (i > 0)
                  text += ", ";
               let element = inst.values[i];
               text += this.getItemCellContent(element, 1, is_selected);
            }
            text += " ]";
            return text;
         }
         if (!(inst instanceof CValueInstance)) {
            return null;
         }
         let decl = inst.decl;
         console.assert(!!decl);
         if (decl.type == "omitted") {
            return _style("deemphasize", "[omitted]");
         }
         if (inst.value === null) {
            return _style("deemphasize", "[empty]");
         }
         switch (decl.type) {
            case "boolean":
               return _style("value-text", ""+inst.value);
            case "integer":
               {
                  let typename = decl.c_types.serialized.name;
                  let format   = inst.save_format;
                  let enum_def = format.enums.get(typename);
                  if (enum_def) {
                     for(const [name, value] of enum_def)
                        if (value == inst.value)
                           return _style("value-text", name);
                  }
               }
               return _style("value-text", ""+inst.value);
            case "pointer":
               return _style("value-text", "0x" + inst.value.toString(16).padStart(8, '0').toUpperCase());
            case "string":
               let printer = new LiteralPokeStringPrinter();
               printer.escape_quotes = true;
               printer.print(inst.value);
               let text = printer.result;
               return _style("value-text", `"${text}"`);
            case "buffer":
               {
                  let text = "";
                  for(let i = 0; i < inst.value.byteLength; ++i) {
                     let e = inst.value.getUint8(i);
                     text += e.toString(16).padStart(2, '0').toUpperCase() + ' ';
                  }
                  return _style("value-text", text);
               }
               break;
         }
      }
      stringify_type(inst, is_selected) {
         function _style(name, text) {
            text = (text+"").replaceAll("[", "[raw][[/raw]");
            if (is_selected)
               return text;
            return `[style=${name}]${text}[/style]`;
         }
         
         function _stringify_string_length(decl) {
            let len = "...";
            if (decl.options.length) {
               len = decl.options.length;
               if (decl.options.needs_terminator) {
                  len += " + sizeof('\\0')";
               }
            }
            return len;
         }
         
         if (inst instanceof CTypeInstance) {
            let type = inst.type;
            if (type) {
               if (type.tag) {
                  return type.tag;
               } else if (type.symbol) {
                  return type.symbol;
               }
            }
         } else if (inst instanceof CArrayInstance) {
            let typename = inst.decl.c_types.serialized.name;
            if (inst.decl.type == "string") {
               typename = `string<${typename}[raw][[/raw]${_stringify_string_length(inst.decl)}]>`;
            }
            for(let i = 0; i < inst.decl.array_extents.length; ++i) {
               if (i < inst.rank)
                  continue;
               typename += "[raw][[/raw]" + inst.decl.array_extents[i] + "]";
            }
            return typename;
         } else if (inst instanceof CValueInstance) {
            let decl     = inst.decl;
            let typename = decl.c_types.serialized.name;
            if (decl.type == "string") {
               typename = `string<${typename}[raw][[/raw]${_stringify_string_length(inst.decl)}]>`;
            } else if (decl.bitfield_info) {
               typename += ":" + decl.bitfield_info.size;
            }
            return typename;
         }
      }
      
      /*String*/ getItemCellContent(/*const CInstance*/ inst, col, is_selected) /*const*/ {
         if (col == 0) { // name
            return this.stringify_name(inst, is_selected);
         } else if (col == 1) { // value
            return this.stringify_value(inst, is_selected);
         } else if (col == 2) { // type
            return this.stringify_type(inst, is_selected);
         }
         return null;
      }
      /*Optional<String>*/ getItemTooltip(/*CInstance*/ item, col) /*const*/ {
         if (col == 0) {
            return item.build_path_string();
         }
         return null;
      }
   };
   
   #model;
   #typed_value_formatters = Object.create(null);
   
   constructor() {
      super();
      this.#model = new CViewElement.Model(this, this.#typed_value_formatters);
      this.model = this.#model;
      this.addTextStyle("deemphasize");
      this.addTextStyle("error");
      this.addTextStyle("name-text");
      this.addTextStyle("value-text");
   }
   
   // Compare to writing DisplayString for a type in Natvis.
   getTypeFormatter(typename) {
      return this.#typed_value_formatters[typename] || null;
   }
   setTypeFormatter(typename, formatter) {
      if (!formatter) {
         if (!this.#typed_value_formatters[typename])
            return;
         this.#typed_value_formatters[typename] = null;
      } else {
         if (!(formatter instanceof Function))
            throw new TypeError("the formatter must be a function");
         if (this.#typed_value_formatters[typename] == formatter)
            return;
         this.#typed_value_formatters[typename] = formatter;
      }
      this.repaint();
   }
   
   get scope() { return this.#model.scope; }
   set scope(v) {
      if (!v) {
         this.#model.scope = null;
         this.repaint();
         return;
      }
      if (v == this.#model.scope)
         return;
      if (!(v instanceof CInstance))
         throw new TypeError("invalid scope object");
      this.#model.scope = v;
      this.repaint();
   }
};
customElements.define("c-view", CViewElement);