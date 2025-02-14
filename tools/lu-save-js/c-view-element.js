
class CViewElement extends HTMLElement {
   #shadow;
   #canvas;
   #scroll_pane;
   #scroll_sizer;
   #tooltip_region_container;
   
   #column_widths = {
      path:  "3fr",
      value: "1fr",
      type:  "30ch",
   };
   #computed_column_widths = {
      path:  null,
      value: null,
      type:  null,
   };
   
   #scope = null; // Variant<CStructInstance, CUnionInstance, CValueInstanceArray, CValue>
   
   #resize_observer = null;
   
   #recache_styles_queued = false;
   #repaint_queued        = false;
   
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
   
   //
   // Allow setting certain display properties via CSS. We (ab)use CSS transitions 
   // and transition events to detect changes to the computed style, recaching and 
   // repainting when those changes occur.
   //
   #cached_styles = {
      //
      // Each key in this object maps to a ::part(...) exposed by our element, for 
      // the outside world to style. The `_type` fields on each entry identify the 
      // properties we'll check and cache.
      //
      "base-text": {
         _type: "text"
      },
      "name-text": {
         _type: "text"
      },
      "value-text": {
         _type: "text"
      },
      "header-row":  { _type: ["box", "text"], },
      "header-cell": { _type: ["box", "text"], },
      "row": {
         _type: "box",
      },
      "twisty": {
         _type: "icon",
      },
   };
   #cached_style_addenda = {
      header_height:         0,
      header_content_height: 0,
      row_height:            0,
      row_content_height:    0,
   };
   #recache_styles() {
      for(let key in this.#cached_styles) {
         let part  = this.#shadow.querySelector(`[part="${key}"]`);
         let style = getComputedStyle(part);
         let cache = this.#cached_styles[key];
         
         let is_multi = Array.isArray(cache._type);
         let is_box   = cache._type == "box" || (is_multi && cache._type.includes("box"));
         let is_icon  = cache._type == "icon";
         let is_text  = cache._type == "text" || (is_multi && cache._type.includes("text"));
         
         if (is_box) {
            cache.background = style.backgroundColor;
            cache.border_color = {
               bottom: (style.borderBottomColor),
               left:   (style.borderLeftColor),
               right:  (style.borderRightColor),
               top:    (style.borderTopColor),
            };
            cache.border_width = {
               bottom: parseFloat(style.borderBottomWidth),
               left:   parseFloat(style.borderLeftWidth),
               right:  parseFloat(style.borderRightWidth),
               top:    parseFloat(style.borderTopWidth),
            };
            cache.padding = {
               bottom: parseFloat(style.paddingBottom),
               left:   parseFloat(style.paddingLeft),
               right:  parseFloat(style.paddingRight),
               top:    parseFloat(style.paddingTop),
            };
         }
         if (is_icon) {
            cache.color     = style.color;
            cache.width     = parseFloat(style.width);
            cache.height    = parseFloat(style.height);
         }
         if (is_text) {
            cache.color     = style.color;
            cache.font      = style.font;
            cache.font_size = parseFloat(style.fontSize); // for size/layout calculations
         }
      }
      
      const addenda = this.#cached_style_addenda;
      const styles  = this.#cached_styles;
      
      addenda.header_content_height = Math.round(styles["header-cell"].font_size);
      addenda.header_height = Math.floor(
         styles["header-row"].border_width.top +
         styles["header-row"].border_width.bottom +
         styles["header-row"].padding.top +
         styles["header-row"].padding.bottom +
         styles["header-cell"].border_width.top +
         styles["header-cell"].border_width.bottom +
         styles["header-cell"].padding.top +
         styles["header-cell"].padding.bottom +
         addenda.header_content_height
      );
      
      addenda.row_content_height =  Math.max(
         styles["base-text"].font_size,
         styles["name-text"].font_size,
         styles["value-text"].font_size
      );
      addenda.row_height =
         styles.row.padding.top +
         styles.row.padding.bottom +
         styles.row.border_width.top +
         styles.row.border_width.bottom +
         addenda.row_content_height
      ;
   }
   
   #last_repaint_result = {
      sticky_header: {
         y: 0, // canvas-relative
         h: 0,
      },
      
      // rows[i] == {
      //    item:     ...,
      //    indent:   0,
      //    expanded: true,
      //    coords:   { // canvas-relative
      //       twisty: { x: 0, y: 0 },
      //    }
      // }
      rows: [],
   };
   
   constructor() {
      super();
      this.#shadow = this.attachShadow({ mode: "open" });
      this.#shadow.innerHTML = `
      <style>
:host {
   display: block;
}
:where(:host) {
   width:  100%;
   height: 300px;
}
[part] {
   position:       absolute !important;
   visibility:     hidden   !important;
   pointer-events: none     !important;
   
   transition-duration: 0.001s;
   transition-behavior: allow-discrete;
   
   --transition-properties-text
   
   &[data-type="box"] {
      transition-property: padding;
   }
   &[data-type="text"] {
      transition-property: color, font;
   }
   &[data-type~="box"][data-type~="text"] {
      transition-property: padding, color, font;
   }
   &[data-type="icon"] {
      transition-property: color, width, height;
   }
}

:where([part="twisty"]) {
   width:  1em;
   height: 1em;
   color:  #888;
}

.scroll {
   position: relative;
   overflow: auto;
   width:    100%;
   height:   100%;
   scrollbar-gutter: stable;
   
   canvas {
      position: sticky;
      float:    right; /* remove from document flow */
      left:     0;
      top:      0;
      display:  block;
      width:    100%;
      height:   100%;
   }
   .tooltip-stubs {
      position: sticky;
      float:    left;
      left:     0;
      top:      0;
      width:    0;
      height:   0;
      
      div {
         position: absolute;
      }
   }
}
      </style>
      <div part="header-row" data-type="box text">
         <div part="header-cell" data-type="box text"></div>
      </div>
      <div part="row" data-type="box">
         <span part="base-text" data-type="text">
            <div part="twisty" data-type="icon"></div>
            <span part="name-text" data-type="text"></span>
            <span part="value-text" data-type="text"></span>
         </span>
      </div>
      <div class="scroll">
         <canvas></canvas>
         <div class="tooltip-stubs"></div>
         <div class="sizer"></div>
      </div>
      `;
      this.#canvas = this.#shadow.querySelector("canvas");
      this.#scroll_pane  = this.#shadow.querySelector(".scroll");
      this.#scroll_sizer = this.#shadow.querySelector(".scroll .sizer");
      this.#tooltip_region_container = this.#shadow.querySelector(".tooltip-stubs");
      
      this.#shadow.addEventListener("transitionend", this.#on_observe_css_change.bind(this));
      
      this.addEventListener("click", this.#on_click.bind(this));
      this.#scroll_pane.addEventListener("scroll", this.#on_scroll.bind(this));
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
      this.#recache_styles();
      this.repaint();
   }
   
   //
   // Event handlers
   //
   
   #on_click(e) {
      const x = e.offsetX;
      const y = e.offsetY;
      {  // Check for clicks on the sticky header.
         let sh = this.#last_repaint_result.sticky_header;
         if (sh.h > 0 && y >= sh.y && y <= sh.y + sh.h)
            return;
      }
      //
      // Handle interactions with table rows:
      //
      if (this.#last_repaint_result.rows.length == 0)
         return;
      
      const shift_y    = this.#last_repaint_result.rows[0].y;
      const row_height = this.#cached_style_addenda.row_height;
      
      let i = Math.floor((y - shift_y) / row_height);
      if (i < 0 || i >= this.#last_repaint_result.rows.length)
         return;
      
      let row = this.#last_repaint_result.rows[i];
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
   
   #on_observe_css_change(e) {
      this.#queue_repaint(true);
   }
   
   #on_observe_resize(entries) {
      for(const entry of entries) {
         if (entry.target != this)
            continue;
         this.repaint();
         break;
      }
   }
   
   #on_scroll(e) {
      this.#scroll_pos.x = this.#scroll_pane.scrollLeft;
      this.#scroll_pos.y = this.#scroll_pane.scrollTop;
      this.repaint();
   }
   
   //
   // Internal behavior
   //
   
   #toggle_item_expansion(item) {
      if (this.#expanded_items.has(item)) {
         this.#expanded_items.delete(item);
      } else {
         this.#expanded_items.add(item);
      }
      this.repaint();
   }
   
   //
   // Repaint process
   //
   
   #tooltips_pending_update = null; // #tooltips_pending_update[row][col]
   
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
   
   // Helper function for drawing within a clip region and guaranteeing 
   // that we successfully tear the region down when we're done. The 
   // functor you pass in receives the clip region as an argument, for 
   // convenience.
   #within_clip_region(x, y, w, h, functor) {
      let context = this.#canvas.getContext("2d");
      context.save();
      context.rect(x, y, w, h);
      context.clip();
      context.beginPath(); // ensure that that `rect` call doesn't affect any later `fill` calls
      try {
         functor({ x: x, y: y, w: w, h: h });
      } catch (e) {
         context.restore();
         throw e;
      }
      context.restore();
   }
   
   #update_tooltip(row, col, tip, override_bounds) {
      if (override_bounds) {
         if (override_bounds.w === 0 || override_bounds.h === 0)
            return;
      }
      let x, y, w, h;
      switch (col) {
         case 0:
            x = 0;
            w = this.#computed_column_widths.path;
            break;
         case 1:
            x = this.#computed_column_widths.path;
            w = this.#computed_column_widths.value;
            break;
         case 2:
            x = this.#computed_column_widths.path + this.#computed_column_widths.value;
            w = this.#computed_column_widths.type;
            break;
      }
      x -= this.#scroll_pos.x;
      y = row * this.#cached_style_addenda.row_height - this.#scroll_pos.y;
      h = this.#cached_style_addenda.row_height;
      
      if (override_bounds) {
         if (override_bounds.x || override_bounds.x === 0)
            x = override_bounds.x;
         if (override_bounds.y || override_bounds.y === 0)
            y = override_bounds.y;
         if (override_bounds.w)
            w = override_bounds.w;
         if (override_bounds.h)
            h = override_bounds.h;
      }
      
      let node;
      {
         let r = this.#tooltips_pending_update[row];
         if (r) {
            node = r[col];
            delete r[col];
         }
      }
      if (!node) {
         node = document.createElement("div");
         node.setAttribute("data-row", row);
         node.setAttribute("data-col", col);
         this.#tooltip_region_container.append(node);
      }
      
      node.style.left   = x + "px";
      node.style.top    = y + "px";
      node.style.width  = w + "px";
      node.style.height = h + "px";
      node.title = tip;
   }
   
   #paint_cached_style_box(key, coords, content_paint_functor) {
      let { x, y, w, h, row, col, column_count } = coords;
      
      const canvas  = this.#canvas;
      const context = canvas.getContext("2d");
      const style   = this.#cached_styles[key];
      if (col !== undefined && column_count !== undefined) {
         //
         // Adjust metrics to account for border collapsing.
         //
         if (col + 1 < column_count) {
            let thick = style.border_width.right;
            if (col > 0) {
               thick = Math.max(thick, style.border_width.left);
            }
            w += Math.ceil(thick / 2);
         }
      }
      
      context.fillStyle = style.background;
      if (row !== undefined) {
         context.rect(
            Math.floor(x + style.border_width.left),
            Math.floor(y + style.border_width.top),
            Math.floor(w - style.border_width.left - style.border_width.right),
            Math.floor(h - style.border_width.top - style.border_width.bottom)
         );
      } else {
         let bl = style.border_width.left;
         let br = style.border_width.right;
         let bt = style.border_width.top;
         let bb = style.border_width.bottom;
         if (row > 0) {
            bl = Math.max(bl, br);
         }
         if (col > 0) {
            bt = Math.max(bt, bb);
         }
         if (col < column_count - 1) {
            bb = Math.max(style.border_width.top, style.border_width.bottom);
         }
         
         context.rect(
            Math.floor(x + bl),
            Math.floor(y + bt),
            Math.floor(w - bl - br),
            Math.floor(h - bt - bb)
         );
      }
      context.fill();
      //
      // Borders:
      //
      let thick;
      let color;
      context.lineCap = "butt";
      {  // Horizontal
         let cx1   = Math.floor(x);
         let cx2   = Math.floor(x + w);
         if (row === undefined || row === 0) {
            //
            // For table-cell-like boxes, we draw top borders only if there is 
            // no cell above the current cell. Note that row and column numbers 
            // should be relative to the painted cells, not all cells, i.e. the 
            // first row that you paint is row 0.
            //
            thick = style.border_width.top;
            color = style.border_color.top;
            if (thick > 0) {
               context.strokeStyle = color;
               context.lineWidth   = thick;
               let cy = Math.floor(y + thick) - thick / 2;
               context.beginPath();
               context.moveTo(cx1, cy);
               context.lineTo(cx2, cy);
               context.stroke();
            }
         }
         thick = style.border_width.bottom;
         color = style.border_color.bottom;
         if (row > 0) {
            //
            // Collapse table borders, preferring the thicker border.
            //
            if (thick < style.border_width.top) {
               thick = style.border_width.top;
               color = style.border_color.top;
            }
         }
         if (thick > 0) {
            context.strokeStyle = color;
            context.lineWidth   = thick;
            let cy  = Math.floor(y + h) - thick / 2;
            context.beginPath();
            context.moveTo(cx1, cy);
            context.lineTo(cx2, cy);
            context.stroke();
         }
      }
      {  // Vertical
         let cy1 = Math.floor(y);
         let cy2 = Math.floor(y + h);
         if (col === undefined || col === 0) {
            //
            // Left borders work similarly to top borders.
            //
            thick = style.border_width.left;
            color = style.border_color.left;
            if (thick < style.border_width.right) {
               thick = style.border_width.right;
               color = style.border_color.right;
            }
            if (thick > 0) {
               context.strokeStyle = color;
               context.lineWidth   = thick;
               let cx = Math.floor(x) + thick / 2;
               context.beginPath();
               context.moveTo(cx, cy1);
               context.lineTo(cx, cy2);
               context.stroke();
            }
         }
         thick = style.border_width.right;
         color = style.border_color.right;
         if (col > 0) {
            //
            // Collapse table borders, preferring the thicker border.
            //
            if (thick < style.border_width.left) {
               thick = style.border_width.left;
               color = style.border_color.left;
            }
         }
         if (thick > 0) {
            context.strokeStyle = color;
            context.lineWidth   = thick;
            let cx  = Math.floor(x + w) - thick / 2;
            context.beginPath();
            context.moveTo(cx, cy1);
            context.lineTo(cx, cy2);
            context.stroke();
         }
      }
      
      if (content_paint_functor) {
         context.save();
         try {
            let ix = x + style.border_width.left   + style.padding.left;
            let iy = y + style.border_width.top    + style.padding.top;
            let iw = w - style.border_width.right  - style.padding.right  - (ix - x);
            let ih = h - style.border_width.bottom - style.padding.bottom - (iy - y);
            context.rect(ix, iy, iw, ih);
            context.clip();
            context.beginPath(); // prevent `rect` from affecting any later `fill` calls
            context.translate(ix, iy);
            content_paint_functor({ x: ix, y: iy, w: iw, h: ih });
         } catch (e) {
            context.restore();
            throw e;
         }
         context.restore();
      }
   }
   
   #paint_sticky_header() {
      let canvas  = this.#canvas;
      let context = canvas.getContext("2d");
      
      const styles  = this.#cached_styles;
      const addenda = this.#cached_style_addenda;
      
      let canvas_width  = canvas.width;
      let canvas_height = canvas.height;
      
      let x = 0;
      let y = 0;
      this.#last_repaint_result.sticky_header.y = y;
      this.#last_repaint_result.sticky_header.h = addenda.header_height;
      
      this.#paint_cached_style_box(
         "header-row",
         {
            x: x,
            y: y,
            w: canvas_width,
            h: addenda.header_height
         }
      );
      
      x += styles["header-row"].border_width.left + styles["header-row"].padding.left;
      y += styles["header-row"].border_width.top  + styles["header-row"].padding.top;
      let w = 
         canvas_width -
         styles["header-row"].border_width.left -
         styles["header-row"].border_width.right -
         styles["header-row"].padding.left -
         styles["header-row"].padding.right
      ;
      let h = 
         addenda.header_height -
         styles["header-row"].border_width.top -
         styles["header-row"].border_width.bottom -
         styles["header-row"].padding.top -
         styles["header-row"].padding.bottom
      ;
      let r = x + w;
      
      context.font = styles["header-cell"].font;
      this.#paint_cached_style_box(
         "header-cell",
         {
            x:   x,
            y:   y,
            w:   this.#computed_column_widths.path - x,
            h:   h,
            row: 0,
            col: 0,
            column_count: 3,
         },
         function(content_box) {
            const text = "Path";
            context.fillStyle = styles["header-cell"].color;
            context.fillText(text, 0, 0);
            let width = context.measureText(text).width;
            if (width > content_box.w) {
               // TODO: implement tooltips on headers
            }
         }
      );
      
      x = this.#computed_column_widths.path;
      this.#paint_cached_style_box(
         "header-cell", 
         {
            x:   x,
            y:   y,
            w:   this.#computed_column_widths.path + this.#computed_column_widths.value - x,
            h:   h,
            row: 0,
            col: 1,
            column_count: 3,
         },
         function(content_box) {
            const text = "Value";
            context.fillStyle = styles["header-cell"].color;
            context.fillText(text, 0, 0);
            let width = context.measureText(text).width;
            if (width > content_box.w) {
               // TODO: implement tooltips on headers
            }
         }
      );
      
      x = this.#computed_column_widths.path + this.#computed_column_widths.value;
      this.#paint_cached_style_box(
         "header-cell",
         {
            x:   x,
            y:   y,
            w:   r - x,
            h:   h,
            row: 0,
            col: 2,
            column_count: 3,
         },
         function(content_box) {
            const text = "Type";
            context.fillStyle = styles["header-cell"].color;
            context.fillText(text, 0, 0);
            let width = context.measureText(text).width;
            if (width > content_box.w) {
               // TODO: implement tooltips on headers
            }
         }
      );
   }
   
   #paint_item_row(row, indent, item, name, is_expanded) {
      let canvas  = this.#canvas;
      let context = canvas.getContext("2d");
      
      const canvas_width  = canvas.width;
      const canvas_height = canvas.height;
      const row_height    = this.#cached_style_addenda.row_height;
      const indent_width  = this.#cached_styles["base-text"].font_size;
      
      let x = indent * indent_width;
      let y = this.#cached_style_addenda.header_height + row * row_height;
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
      
      // Draw table borders for this row and its cells.
      {
         let ccw = this.#computed_column_widths;
         if (this.#cached_styles.row.background != "transparent") {
            context.fillStyle = this.#cached_styles.row.background;
            context.rect(x, y, ccw.path, row_height);
            context.rect(x + ccw.path, y, ccw.value, row_height);
            context.rect(x + ccw.path + ccw.value, y, ccw.type, row_height);
         }
         let cbw = this.#cached_styles.row.border_width;
         let cbc = this.#cached_styles.row.border_color;
         
         function unblurred_line_h(thickness, x, y, to) {
            x  = Math.round(x);
            y  = Math.round(y);
            to = Math.round(to);
            let off = thickness % 2 ? 0.5 : 0;
            context.beginPath();
            context.moveTo(x + off,  y + off);
            context.lineTo(to - off, y + off);
            context.stroke();
         }
         function unblurred_line_v(thickness, x, y, to) {
            x  = Math.round(x);
            y  = Math.round(y);
            to = Math.round(to);
            let off = thickness % 2 ? 0.5 : 0;
            context.beginPath();
            context.moveTo(x + off, y + off);
            context.lineTo(x + off, to - off);
            context.stroke();
         }
         
         if (row == 0 && cbw.top > 0) {
            context.strokeStyle = cbc.top;
            context.lineWidth   = cbw.top;
            unblurred_line_h(cbw.top, 0, Math.floor(y), canvas_width);
         }
         {  // Row left border
            context.strokeStyle = cbc.left;
            context.lineWidth   = cbw.left;
            unblurred_line_v(cbw.left, 0, Math.floor(y), y + row_height);
         }
         {  // Row right border
            context.strokeStyle = cbc.right;
            context.lineWidth   = cbw.right;
            unblurred_line_v(cbw.right, canvas_width - cbw.right, Math.floor(y), y + row_height);
         }
         {  // Row bottom border
            context.strokeStyle = cbc.bottom;
            context.lineWidth   = cbw.bottom;
            unblurred_line_h(cbw.bottom, 0, y + row_height - cbw.bottom, canvas_width);
         }
         //
         // Borders between cells.
         //
         if (cbw.left || cbw.right) {
            let thick;
            if (cbw.left > cbw.right) {
               context.strokeStyle = cbc.left;
               context.lineWidth   = thick = cbw.left;
            } else {
               context.strokeStyle = cbc.right;
               context.lineWidth   = thick = cbw.right;
            }
            unblurred_line_v(thick, ccw.path,             Math.floor(y), y + row_height);
            unblurred_line_v(thick, ccw.path + ccw.value, Math.floor(y), y + row_height);
         }
         
      }
      
      // COLUMN: Path
      this.#within_clip_region(
         0,
         0,
         this.#computed_column_widths.path - this.#cached_styles.row.padding.right,
         canvas_height,
         (function(clipregion) {
            x += this.#cached_styles.row.padding.left;
            y += this.#cached_styles.row.padding.top;
            
            context.font = this.#cached_styles["base-text"].font;
            const space_width = context.measureText(" ").width;
            
            if (!(item instanceof CValueInstance)) {
               //
               // Item can have children. Draw a twisty.
               //
               const w  = this.#cached_styles.twisty.width;
               const h  = this.#cached_styles.twisty.height;
               const dy = (this.#cached_style_addenda.row_content_height - h) / 2;
               context.fillStyle = this.#cached_styles.twisty.color;
               if (is_expanded) {
                  context.moveTo(x,         dy + y);
                  context.lineTo(x + w,     dy + y);
                  context.lineTo(x + w / 2, dy + y + h);
                  context.closePath();
               } else {
                  context.moveTo(x,     dy + y);
                  context.lineTo(x + w, dy + y + h / 2);
                  context.lineTo(x,     dy + y + h);
                  context.closePath();
               }
               context.fill();
               painted.coords.twisty = {
                  x: x,
                  y: y + dy,
                  w: w,
                  h: h,
               };
            }
            x += this.#cached_styles.twisty.width; // advance past the twisty (or where it would be)
            x += space_width;
            
            //
            // Draw path text.
            //
            context.fillStyle = this.#cached_styles["name-text"].color;
            context.font      = this.#cached_styles["name-text"].font;
            context.fillText(name, x, y);
            
            let width = context.measureText(name).width;
            if (width > clipregion.w - x) {
               this.#update_tooltip(
                  row,
                  0,
                  name,
                  {  // override bounds so we don't cover and block the twisty
                     x:     x,
                     width: clipregion.w - x
                  }
               );
            }
         }).bind(this)
      );
      
      // COLUMN: Value
      x = this.#computed_column_widths.path + this.#cached_styles.row.padding.left;
      this.#within_clip_region(
         this.#computed_column_widths.path,
         0,
         this.#computed_column_widths.value - this.#cached_styles.row.padding.right,
         canvas_height,
         (function(clipregion) {
            let value = null;
            if (item instanceof CValueInstance) {
               value = this.#stringify_value(item);
            }
            if (value === null)
               return;
            context.fillStyle = this.#cached_styles["value-text"].color;
            context.font      = this.#cached_styles["value-text"].font;
            context.fillText(value, x, y);
            
            let width = context.measureText(value).width;
            if (width > clipregion.w - this.#cached_styles.row.padding.left) {
               this.#update_tooltip(row, 1, value);
            }
         }).bind(this)
      );
      
      // COLUMN: Type
      x = this.#computed_column_widths.path + this.#computed_column_widths.value + this.#cached_styles.row.padding.left;
      this.#within_clip_region(
         this.#computed_column_widths.path + this.#computed_column_widths.value,
         0,
         this.#computed_column_widths.type - this.#cached_styles.row.padding.right,
         canvas_height,
         (function(clipregion) {
            let typename = null;
            if (item instanceof CStructInstance) {
               let type = item.type;
               if (type) {
                  if (type.tag) {
                     //typename = "struct " + type.tag;
                     typename = type.tag;
                  } else if (type.symbol) {
                     typename = type.symbol;
                  }
               }
            } else if (item instanceof CUnionInstance) {
               
            } else if (item instanceof CValueInstance) {
               typename = item.base.c_typenames.serialized;
               if (item.base.type == "string") {
                  typename += "[...]";
               }
            } else if (item instanceof CValueInstanceArray) {
               typename = item.base.c_typenames.serialized;
               for(let i = 0; i < item.base.array_ranks.length; ++i) {
                  if (i < item.rank)
                     continue;
                  let extent = item.base.array_ranks[i];
                  typename += "[" + extent + "]";
               }
            }
            if (typename === null)
               return;
            context.fillStyle = this.#cached_styles["name-text"].color;
            context.font      = this.#cached_styles["name-text"].font;
            context.fillText(typename, x, y);
            
            let width = context.measureText(typename).width;
            if (width > clipregion.w - this.#cached_styles.row.padding.left) {
               this.#update_tooltip(row, 2, typename);
            }
         }).bind(this)
      );
      
      this.#last_repaint_result.rows.push(painted);
   }
   
   #queue_repaint(also_styles) {
      if (also_styles)
         this.#recache_styles_queued = true;
      if (this.#repaint_queued)
         return;
      this.#repaint_queued = true;
      window.setTimeout(this.repaint.bind(this), 1);
   }
   repaint() {
      this.#repaint_queued = false;
      if (this.#recache_styles_queued) {
         this.#recache_styles_queued = false;
         this.#recache_styles();
      }
      
      let canvas    = this.#canvas;
      canvas.width  = this.#scroll_pane.clientWidth;
      canvas.height = this.#scroll_pane.clientHeight;
      let context   = canvas.getContext("2d");
      context.textBaseline = "top";
      
      {
         this.#tooltips_pending_update = {};
         for(let tooltip of this.#tooltip_region_container.children) {
            let row = +tooltip.getAttribute("data-row");
            let col = +tooltip.getAttribute("data-col");
            
            let r = this.#tooltips_pending_update[row];
            if (!r)
               r = this.#tooltips_pending_update[row] = [];
            r[col] = tooltip;
         }
      }
      
      {
         context.font = this.#cached_styles["base-text"].font;
         
         let base_width = canvas.width;
         let flex_width = base_width;
         let flex_sum   = 0;
         for(let key in this.#column_widths) {
            this.#computed_column_widths[key] = null;
            let desired = this.#column_widths[key];
            if (desired.endsWith("fr")) {
               flex_sum += Math.max(0, parseFloat(desired));
               continue;
            }
            
            let width;
            if (desired.endsWith("%")) {
               width = base_width * parseFloat(desired) / 100;
            } else if (desired.endsWith("px")) {
               width = parseFloat(desired);
            } else if (desired.endsWith("ch")) {
               width = context.measureText(" ").width * parseFloat(desired);
            } else if (desired.endsWith("em")) {
               let metrics = context.measureText("X");
               let em      = metrics.emHeightAscent - metrics.emHeightDescent;
               width = em * parseFloat(desired);
            }
            if (+width > 0) {
               this.#computed_column_widths[key] = width;
               flex_width -= width;
            } else {
               this.#column_widths[key] = "1fr";
               flex_sum += 1;
            }
         }
         for(let key in this.#column_widths) {
            if (this.#computed_column_widths[key] !== null)
               continue;
            let flex = parseFloat(this.#column_widths[key]) / flex_sum;
            this.#computed_column_widths[key] = flex_width * flex;
         }
      }
      
      const ROW_HEIGHT   = this.#cached_style_addenda.row_height;
      const INDENT_WIDTH = this.#cached_styles["base-text"].font_size;
      
      context.font      = this.#cached_styles["base-text"].font;
      context.fillStyle = this.#cached_styles["base-text"].color;
      
      let current_row    = 0;
      let current_indent = 0;
      let current_scroll = this.#scroll_pos;
      let first_row_y    = this.#cached_style_addenda.header_height;
      let canvas_end_y   = current_scroll.y + this.offsetHeight;
      let paint_item;
      this.#last_repaint_result.rows = [];
      paint_item = (function(name, item) {
         let rows     = 1;
         let expanded = false;
         if (!(item instanceof CValueInstance))
            if (this.#expanded_items.has(item))
               expanded = true;
         
         let y = first_row_y + current_row * ROW_HEIGHT;
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
      
      this.#content_size.height = first_row_y + current_row * ROW_HEIGHT;
      this.#paint_sticky_header();
      
      this.#scroll_sizer.style.height = `${current_row * ROW_HEIGHT}px`;
      
      for(let row in this.#tooltips_pending_update) {
         let list = this.#tooltips_pending_update[row];
         for(let col in list) {
            if (list[col])
               list[col].remove();
         }
      }
      this.#tooltips_pending_update = {};
   }
};
customElements.define("c-view", CViewElement);