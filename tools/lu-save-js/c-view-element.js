
class CViewElement extends HTMLElement {
   #shadow;
   #canvas;
   #scope = null; // Variant<CStructInstance, CUnionInstance, CValueInstanceArray, CValue>
   
   #resize_observer = null;
   
   // Contains all CStructInstances, etc., that the user has expanded within the treeview
   #expanded_items = new Set();
   
   #content_size = {
      width:  0,
      height: 0,
   };
   #scroll_pos = {
      x: 0,
      y: 0,
   };
   
   #style = {
      font_size:   12, // pixels
      row_padding: 12 * 0.25,
   };
   
   // Cache of the last-painted rows, so we can handle click events, etc., 
   // sanely and without having to re-layout everything.
   //
   // items: {
   //    item:     ...,
   //    indent:   0,
   //    expanded: true,
   //    coords:   { // canvas-relative
   //       twisty: { x: 0, y: 0 },
   //    }
   // }
   #last_painted_rows = [];
   
   constructor() {
      super();
      this.#shadow = this.attachShadow({ mode: "open" });
      this.#shadow.innerHTML = `
      <style>
:host {
   display: inline-block;
}
canvas {
   display: block;
   width:   100%;
   height:  100%;
}
      </style>
      `;
      this.#canvas = document.createElement("canvas");
      this.#shadow.append(this.#canvas);
      
      this.addEventListener("click", this.#on_click.bind(this));
      this.addEventListener("wheel", this.#on_mousewheel.bind(this));
   }
   
   get scope() { return this.#scope; }
   set scope(v) {
      if (!v) {
         this.#scope = null;
         this.repaint();
         return;
      }
      if (v == this.#scope)
         return;
      {
         let found = false;
         for(let allowed of [
            CStructInstance,
            CUnionInstance,
            CValueInstance,
            CValueInstanceArray,
         ]) {
            if (v instanceof allowed) {
               found = true;
               break;
            }
         }
         if (!found)
            throw new TypeError("invalid scope object");
      }
      this.#scope = v;
      this.repaint();
   }
   
   #has_ever_been_connected = false;
   connectedCallback() {
      if (!this.#has_ever_been_connected) {
         this.#has_ever_been_connected = true;
         this.#resize_observer = new ResizeObserver(this.#on_observe_resize.bind(this));
         this.#resize_observer.observe(this);
      }
      this.repaint();
   }
   
   //
   // Event handlers
   //
   
   #on_click(e) {
      let x = e.offsetX;
      let y = e.offsetY;
      if (this.#last_painted_rows.length == 0)
         return;
      
      let shift_y    = this.#last_painted_rows[0].y;
      let row_height = this.#last_painted_rows[0].h;
      
      let i = Math.floor((y - shift_y) / row_height);
      if (i < 0 || i >= this.#last_painted_rows.length)
         return;
      
      let row = this.#last_painted_rows[i];
      {
         let twisty = row.coords.twisty;
         if (twisty) {
            if (x >= twisty.x && x <= twisty.x + twisty.w) {
               if (y >= twisty.y && y <= twisty.y + twisty.h) {
                  e.preventDefault();
                  this.#toggle_item_expansion(row.item);
                  return;
               }
            }
         }
      }
   }
   
   #on_mousewheel(e) {
      if (e.deltaY) {
         const outer_height = this.#canvas.height;
         const inner_height = this.#content_size.height;
         if (inner_height > outer_height) {
            let y = e.deltaY;
            switch (e.deltaMode) {
               case WheelEvent.DOM_DELTA_PIXEL:
                  break;
               case WheelEvent.DOM_DELTA_LINE:
                  y *= (this.#style.font_size + this.#style.row_padding * 2);
                  break;
               case WheelEvent.DOM_DELTA_PAGE:
                  break;
            }
            if (this.#scroll_y_by(y))
               e.preventDefault();
         }
      }
   }
   
   #on_observe_resize(entries) {
      for(const entry of entries) {
         if (entry.target != this)
            continue;
         this.repaint();
         break;
      }
   }
   
   //
   // Internal behavior
   //
   
   #scroll_y_by(px) {
      const outer_height = this.#canvas.height;
      const inner_height = this.#content_size.height;
      
      let y = px + this.#scroll_pos.y;
      y = Math.max(0, Math.min(inner_height - outer_height, y));
      if (y != this.#scroll_pos.y) {
         this.#scroll_pos.y = y;
         this.repaint();
         return true;
      }
      return false;
   }
   
   #toggle_item_expansion(item) {
      if (this.#expanded_items.has(item)) {
         this.#expanded_items.delete(item);
      } else {
         this.#expanded_items.add(item);
      }
      this.repaint();
   }
   
   #repaint_scrollbar() {
      const SCROLLBAR_WIDTH = 8;
      
      let x      = this.#canvas.width - SCROLLBAR_WIDTH;
      let height = this.#canvas.height;
      if (this.#content_size.height <= height)
         return;
      
      let track_height = height;
      
      let context = this.#canvas.getContext("2d");
      context.fillStyle = "#DDD"; // track
      context.fillRect(x, 0, SCROLLBAR_WIDTH, track_height);
      
      context.fillStyle = "#888"; // thumb
      let thumb_size = (height / this.#content_size.height) * track_height;
      let thumb_pos  = (this.#scroll_pos.y / this.#content_size.height) * track_height;
      thumb_size = Math.max(thumb_size, 1);
      context.fillRect(x, thumb_pos, SCROLLBAR_WIDTH, thumb_size);
   }
   
   #stringify_value(item) {
      console.assert(item instanceof CValueInstance);
      if (item.value === null) {
         return "[empty]";
      }
      
      let base = item.base;
      console.assert(!!base);
      switch (base.type) {
         case "boolean":
         case "integer":
            return ""+item.value;
         case "pointer":
            return "0x" + item.value.toString(16).padStart(8, '0').toUpperCase();
         case "string":
            {
               let text = "";
               for(let i = 0; i < item.value.length; ++i) {
                  let cc = item.value[i];
                  let ch = CHARSET_ENGLISH.bytes_to_chars[cc];
                  if (!ch) {
                     ch = CHARSET_CONTROL_CODES.bytes_to_chars[cc];
                     if (ch == '\0')
                        break;
                  }
                  
                  text += "'" + ch + "' ";
               }
               return text;
            }
            break;
         case "buffer":
            {
               let text = "";
               for(let i = 0; i < item.value.byteLength; ++i) {
                  let e = item.value.getUint8(i);
                  text += e.toString(16).padStart(2, '0').toUpperCase() + ' ';
               }
               return text;
            }
            break;
         case "omitted":
            return "[omitted]";
      }
      
   }
   
   #paint_item_row(row, indent, item, name, is_expanded) {
      let canvas  = this.#canvas;
      let context = canvas.getContext("2d");
      
      const row_height   = (this.#style.font_size + (this.#style.row_padding * 2));
      const indent_width = this.#style.font_size;
      
      let x = indent * indent_width;
      let y = row * row_height;
      x -= this.#scroll_pos.x;
      y -= this.#scroll_pos.y;
      
      let painted = {
         item:     item,
         indent:   indent,
         expanded: is_expanded,
         x:        x,
         y:        y,
         h:        row_height,
         coords:   {
            twisty: null,
         },
      };
      
      x += this.#style.row_padding;
      y += this.#style.row_padding;
      
      if (!(item instanceof CValueInstance)) {
         //
         // Item can have children. Draw a twisty.
         //
         context.fillStyle = "#888";
         if (is_expanded) {
            context.moveTo(x, y);
            context.lineTo(x + this.#style.font_size, y);
            context.lineTo(x + this.#style.font_size / 2, y + this.#style.font_size);
            context.closePath();
         } else {
            context.moveTo(x, y);
            context.lineTo(x + this.#style.font_size, y + this.#style.font_size / 2);
            context.lineTo(x, y + this.#style.font_size);
            context.closePath();
         }
         context.fill();
         painted.coords.twisty = {
            x: x,
            y: y,
            w: this.#style.font_size,
            h: this.#style.font_size,
         };
      }
      x += this.#style.font_size; // advance past the twisty (or where it would be)
      //
      // Spacing between twisty and next content:
      //
      x += this.#style.row_padding;
      
      // Draw item text.
      {
         let value = null;
         if (item instanceof CValueInstance) {
            value = this.#stringify_value(item);
         }
         context.fillStyle = "#000";
         context.fillText(name, x, y);
         if (value !== null) {
            x += context.measureText(name).width;
            context.fillStyle = "#888";
            context.fillText(": ", x, y);
            x += context.measureText(": ").width;
            context.fillStyle = "#4A6";
            context.fillText(value, x, y);
         }
      }
      
      this.#last_painted_rows.push(painted);
   }
   
   repaint() {
      let canvas    = this.#canvas;
      canvas.width  = this.offsetWidth;
      canvas.height = this.offsetHeight;
      let context   = canvas.getContext("2d");
      
      const ROW_HEIGHT = (this.#style.font_size + (this.#style.row_padding * 2));
      
      const INDENT_WIDTH = this.#style.font_size;
      
      context.font = `${this.#style.font_size}px Arial`;
      context.fillStyle = "#000";
      context.textBaseline = "top";
      
      let current_row    = 0;
      let current_indent = 0;
      let current_scroll = this.#scroll_pos;
      let canvas_end_y   = current_scroll.y + this.offsetHeight;
      let paint_item;
      this.#last_painted_rows = [];
      paint_item = (function(name, item) {
         let rows     = 1;
         let expanded = false;
         if (!(item instanceof CValueInstance))
            if (this.#expanded_items.has(item))
               expanded = true;
//expanded = true; // TEST
         
         let y = current_row * ROW_HEIGHT;
         if (y + ROW_HEIGHT > current_scroll.y && y < canvas_end_y) {
            this.#paint_item_row(current_row, current_indent, item, name, expanded);
         }
         ++current_row;
         if (expanded) {
            ++current_indent;
            if (item instanceof CValueInstanceArray) {
               for(let i = 0; i < item.values.length; ++i) {
                  let child = item.values[i];
                  paint_item(i, child);
               }
            } else if (item instanceof CStructInstance) {
               for(let name in item.members) {
                  let child = item.members[name];
                  paint_item(name, child);
               }
            } else if (item instanceof CUnionInstance) {
               let name = "[unnamed]";
               for(let memb of item.type.members) {
                  if (memb == item.value.base) {
                     name = memb.name;
                     break;
                  }
               }
               paint_item(name, item.value);
            }
            --current_indent;
         }
      }).bind(this);
      
      let item = this.#scope;
      if (this.#scope instanceof CValueInstance) {
         paint_item("[...]", item);
      } else if (this.#scope instanceof CValueInstanceArray) {
         for(let i = 0; i < item.values.length; ++i) {
            let child = item.values[i];
            paint_item(i, child);
         }
      } else if (this.#scope instanceof CStructInstance) {
         for(let name in item.members) {
            let child = item.members[name];
            paint_item(name, child);
         }
      } else if (this.#scope instanceof CUnionInstance) {
         let name = "[unnamed]";
         for(let memb of item.type.members) {
            if (memb == item.value.base) {
               name = memb.name;
               break;
            }
         }
         paint_item(name, item.value);
      }
      
      this.#content_size.height = current_row * ROW_HEIGHT;
      
      this.#repaint_scrollbar();
   }
};
customElements.define("c-view", CViewElement);