// Subclass this, and give your subclass to the view element.
class TreeRowViewModel/*<T>*/ {
   constructor() {
      this.columns = [
         { name: "Items", width: "1fr" }
      ];
   }
   
   /*T*/ getItem(/*Optional<const T>*/ parent, row) /*const*/ {
      return null;
   }
   /*Optional<Variant<Array<T>, Map<String, T>>>*/ getItemChildren(/*const T*/ item) /*const*/ {
      return null;
   }
   /*bool*/ itemHasChildren(/*const T*/ item) /*const*/ {
      let children = this.getItemChildren(item);
      if (!children)
         return false;
      if (Array.isArray(children))
         return children.length > 0;
      return children.size > 0;
   }
   
   /*String*/ getItemCellContent(/*const T*/ item, col, /*bool*/ is_selected) /*const*/ {
      //
      // Returns a BBCode-formatted string (proper tag nesting required).
      //
      return (""+item).replaceAll("[", "[raw][[/raw]");
   }
   /*Optional<String>*/ getItemTooltip(/*const T*/ item, col) /*const*/ {
      return null;
   }
};

class TreeRowViewElement extends HTMLElement {
   static #Column = class Column {
      constructor(name, width) {
         this.name  = name || "";
         this.width = width;
         if (!width && width !== 0)
            this.width = "1fr";
         
         this.computed_width = null;
      }
   };
   
   static TextStyle = class TextStyle {
      constructor(name) {
         this.part_node = document.createElement("div");
         this.part_node.setAttribute("part", name);
         this.part_node.setAttribute("data-type", "text");
         
         this.color     = "#000";
         this.font      = "";
         this.font_size = 0; // used for computing sizing
      }
      get height() {
         return this.font_size;
      }
      
      recache() {
         let style = getComputedStyle(this.part_node);
         
         this.color     = style.color;
         this.font      = style.font;
         this.font_size = parseFloat(style.fontSize);
         
         return style; // return, so overrides don't have to call getComputedStyle again
      }
   };
   
   static BoxStyle = class BoxStyle extends TreeRowViewElement.TextStyle {
      constructor(name) {
         super(name);
         this.part_node.setAttribute("data-type", "box");
         
         const ZEROES = {
            top:    0,
            right:  0,
            bottom: 0,
            left:   0,
         };
         
         this.background   = "transparent";
         this.border_color = {};
         this.border_style = {};
         this.border_width = Object.assign({}, ZEROES);
         this.padding      = Object.assign({}, ZEROES);
         for(let side in ZEROES) {
            this.border_color[side] = "transparent";
            this.border_style[side] = "solid";
         }
      }
      
      recache() {
         let style = super.recache();
         
         this.background = style.backgroundColor;
         this.border_color = {
            bottom: style.borderBottomColor,
            left:   style.borderLeftColor,
            right:  style.borderRightColor,
            top:    style.borderTopColor,
         };
         this.border_width = {
            bottom: parseFloat(style.borderBottomWidth),
            left:   parseFloat(style.borderLeftWidth),
            right:  parseFloat(style.borderRightWidth),
            top:    parseFloat(style.borderTopWidth),
         };
         this.padding = {
            bottom: parseFloat(style.paddingBottom),
            left:   parseFloat(style.paddingLeft),
            right:  parseFloat(style.paddingRight),
            top:    parseFloat(style.paddingTop),
         };
         
         return style; // return, so overrides don't have to call getComputedStyle again
      }
      
      get contentLeft() {
         return this.border_width.left + this.padding.left;
      }
      get contentTop() {
         return this.border_width.top + this.padding.top;
      }
      
      get contentInsetHorizontal() {
         return this.border_width.left + this.border_width.right + this.padding.left + this.padding.right;
      }
      get contentInsetVertical() {
         return this.border_width.top + this.border_width.bottom + this.padding.top + this.padding.bottom;
      }
      
      /*DOMRect*/ calcContentBox(/*const DOMRect*/ border_box) /*const*/ {
         let x = this.border_width.left + this.padding.left;
         let y = this.border_width.top  + this.padding.top;
         return new DOMRect(
            border_box.x + x,
            border_box.y + y,
            border_box.width  - x - this.border_width.right  - this.padding.right,
            border_box.height - y - this.border_width.bottom - this.padding.bottom
         );
      }
      /*DOMRect*/ calcPaddingBox(/*const DOMRect*/ border_box) /*const*/ {
         return new DOMRect(
            border_box.x + this.padding.left,
            border_box.y + this.padding.top,
            border_box.width  - this.padding.left - this.padding.right,
            border_box.height - this.padding.top  - this.padding.bottom
         );
      }
   };
   
   static IconStyle = class IconStyle {
      constructor(name) {
         this.part_node = document.createElement("div");
         this.part_node.setAttribute("part", name);
         this.part_node.setAttribute("data-type", "icon");
         
         this.color  = "transparent";
         this.width  = 0;
         this.height = 0;
      }
      
      recache() {
         let style = getComputedStyle(this.part_node);
         
         this.color  = style.color;
         this.width  = parseFloat(style.width);
         this.height = parseFloat(style.height);
         
         return style;
      }
   };
   
   #shadow;
   #canvas;
   #scroll_pane;
   #scroll_sizer;
   #tooltip_region_container;
   
   #allow_selection = false;
   #selected_item   = null;
   #expanded_items  = new Set();
   
   #computed_column_widths = []; // Array<double>
   
   #resize_observer = null;
   
   #recache_queued = true;  // force recaching of types on first repaint
   #repaint_queued = false;
   
   #content_size = { width: 0, height: 0 };
   #scroll_pos   = { x: 0, y: 0 };
   
   #builtin_style_names = [];
   #cached_styles = Object.create(null);
   #cached_style_addenda = {
      header_height:         0,
      header_content_height: 0,
      row_height:            0,
      row_content_height:    0,
   };
   
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
.parts {
   position:       absolute !important;
   visibility:     hidden   !important;
   pointer-events: none     !important;
}
[part] {
   transition-duration: 0.001s;
   transition-behavior: allow-discrete;
   
   &[data-type="box"] {
      transition-property: color, font, background, border, padding;
   }
   &[data-type="text"] {
      transition-property: color, font;
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
      <div class="parts">
      </div>
      <div class="scroll">
         <canvas></canvas>
         <div class="tooltip-stubs"></div>
         <div class="sizer"></div>
      </div>
      `;
      this.#canvas       = this.#shadow.querySelector("canvas");
      this.#scroll_pane  = this.#shadow.querySelector(".scroll");
      this.#scroll_sizer = this.#shadow.querySelector(".scroll .sizer");
      this.#tooltip_region_container = this.#shadow.querySelector(".tooltip-stubs");
      
      {
         const BoxStyle  = TreeRowViewElement.BoxStyle;
         const IconStyle = TreeRowViewElement.IconStyle;
         const container = this.#shadow.querySelector(".parts");
         
         let header_row  = this.#cached_styles["header-row"]  = new BoxStyle("header-row");
         let header_cell = this.#cached_styles["header-cell"] = new BoxStyle("header-cell");
         container.append(header_row.part_node);
         header_row.part_node.append(header_cell.part_node);
         
         let row    = this.#cached_styles["row"]    = new BoxStyle("row");
         let cell   = this.#cached_styles["cell"]   = new BoxStyle("cell");
         let twisty = this.#cached_styles["twisty"] = new IconStyle("twisty");
         container.append(row.part_node);
         row.part_node.append(cell.part_node);
         cell.part_node.append(twisty.part_node);
         {
            let row    = this.#cached_styles["selected-row"]    = new BoxStyle("selected row");
            let cell   = this.#cached_styles["selected-cell"]   = new BoxStyle("selected cell");
            let twisty = this.#cached_styles["selected-twisty"] = new IconStyle("selected twisty");
            container.append(row.part_node);
            row.part_node.append(cell.part_node);
            cell.part_node.append(twisty.part_node);
         }
         
         this.#builtin_style_names = Object.keys(this.#cached_styles);
      }
      
      this.#shadow.addEventListener("transitionend", this.#on_observe_css_change.bind(this));
      
      this.addEventListener("click", this.#on_click.bind(this));
      this.#scroll_pane.addEventListener("scroll", this.#on_scroll.bind(this));
      
      this.model = null; // Optional<TreeRowViewModel>
   }
   
   //
   // Accessors (behavior):
   //
   
   get allowSelection() { return this.#allow_selection; }
   set allowSelection(v) {
      this.#allow_selection = !!v;
      if (!this.#allow_selection) {
         this.#selected_item = null;
         this.repaint();
      }
   }
   
   get selectedItem() { return this.#selected_item; }
   
   //
   // Accessors (painting):
   //
   
   addTextStyle(name, parent_name) {
      if (!name)
         throw new Error("invalid text style name");
      if (this.#builtin_style_names.includes(name))
         throw new Error("the specified name is already in use by a built-in style");
      if (this.#cached_styles[name])
         throw new Error("the specified name is already in use");
      
      let created = this.#cached_styles[name] = new TreeRowViewElement.TextStyle(name);
      let parent_node;
      if (parent_name) {
         let style = this.#cached_styles[parent_name];
         if (style)
            parent_node = style.part_node;
      }
      if (!parent_node)
         parent_node = this.#shadow.querySelector(".parts");
      parent_node.append(created.part_node);
      this.#queue_repaint(true);
   }
   removeTextStyle(name) {
      if (!name)
         throw new Error("invalid text style name");
      if (this.#builtin_style_names.includes(name))
         throw new Error("cannot remove a built-in style");
      let style = this.#cached_styles[name];
      if (!style)
         return;
      style.part_node.remove();
      delete this.#cached_styles[name];
      this.#queue_repaint(true);
   }
   
   //
   // Lifecycle:
   //
   
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
   // Event handlers:
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
      this.#select(row.item);
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
   // Internals (behavior)
   //
   
   #recache_styles() {
      for(let key in this.#cached_styles) {
         let style = this.#cached_styles[key];
         style.recache();
      }
      const addenda = this.#cached_style_addenda;
      const styles  = this.#cached_styles;
      
      addenda.header_content_height = Math.round(styles["header-cell"].font_size);
      addenda.header_height = Math.floor(
         styles["header-row"].contentInsetVertical +
         styles["header-cell"].contentInsetVertical +
         addenda.header_content_height
      );
      
      addenda.row_content_height = Math.max(
         styles["cell"].font_size,
         styles["twisty"].height,
         styles["selected-cell"].font_size,
         styles["selected-twisty"].height
      );
      for(let name in styles) {
         let style = styles[name];
         if (["header-row", "header-cell", "row", "cell", "twisty"].includes(name))
            continue;
         if (!(style instanceof TreeRowViewElement.TextStyle))
            continue;
         addenda.row_content_height = Math.max(addenda.row_content_height, style.font_size);
      }
      addenda.row_height =
         styles["row"].contentInsetVertical +
         styles["cell"].contentInsetVertical +
         addenda.row_content_height
      ;
   }
   #select(item) {
      if (item && !this.#allow_selection) {
         return;
      }
      if (this.#selected_item == item)
         return;
      this.#selected_item = item;
      this.#queue_repaint();
      this.dispatchEvent(new CustomEvent("selection-changed", { detail: {
         item: item,
      }}));
   }
   #toggle_item_expansion(item) {
      if (this.#expanded_items.has(item)) {
         this.#expanded_items.delete(item);
      } else {
         this.#expanded_items.add(item);
      }
      this.repaint();
   }
   
   //
   // Internals (repaint)
   //
   
   #tooltips_pending_update = null; // #tooltips_pending_update[row][col]
   
   // Helper function for drawing within a clip region and guaranteeing 
   // that we successfully tear the region down when we're done. The 
   // functor you pass in receives the clip region as an argument, for 
   // convenience.
   #draw_within_rect(/*DOMRect*/ rect, functor) {
      let context = this.#canvas.getContext("2d");
      context.save();
      context.rect(rect.x, rect.y, rect.width, rect.height);
      context.clip();
      context.beginPath(); // ensure that that `rect` call doesn't affect any later `fill` calls
      context.translate(rect.x, rect.y);
      try {
         functor(rect);
      } catch (e) {
         context.restore();
         throw e;
      }
      context.restore();
   }
   
   // During a repaint, force a tooltip on a given content cell. You can 
   // optionally override the bounds (coordinates are canvas-relative) 
   // for cases where you want to cover only part of a cell.
   #update_tooltip(row, col, /*String*/ tip, /*DOMRect*/ override_bounds) {
      if (override_bounds) {
         if (override_bounds.width === 0 || override_bounds.height === 0)
            return;
      }
      let x, y, w, h;
      
      x = 0;
      for(let i = 0; i < col; ++i)
         x += this.#computed_column_widths[i];
      w = this.#computed_column_widths[col];
      
      x -= this.#scroll_pos.x;
      y = this.#cached_style_addenda.header_height + row * this.#cached_style_addenda.row_height - this.#scroll_pos.y;
      h = this.#cached_style_addenda.row_height;
      
      if (override_bounds) {
         if (override_bounds.x || override_bounds.x === 0)
            x = override_bounds.x;
         if (override_bounds.y || override_bounds.y === 0)
            y = override_bounds.y;
         if (override_bounds.width)
            w = override_bounds.width;
         if (override_bounds.height)
            h = override_bounds.height;
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
   
   #paint_box(/*BoxStyle*/ style, /*DOMRect*/ border_box, row, col, content_paint_functor) {
      let column_count = 1;
      if (this.model)
         column_count = this.model.columns.length;
      
      const canvas  = this.#canvas;
      const context = canvas.getContext("2d");
      if (col !== undefined) {
         //
         // Adjust metrics to account for border collapsing.
         //
         if (col + 1 < column_count) {
            let thick = style.border_width.right;
            if (col > 0) {
               thick = Math.max(thick, style.border_width.left);
            }
            border_box.width += Math.ceil(thick / 2);
         }
      }
      
      context.fillStyle = style.background;
      if (row || row === 0) {
         let inner = style.calcPaddingBox(border_box);
         context.rect(inner.x, inner.y, inner.width, inner.height);
      } else {
         //
         // Apply border-collapsing.
         //
         let bl = style.border_width.left;
         let br = style.border_width.right;
         let bt = style.border_width.top;
         let bb = style.border_width.bottom;
         if (row > 0)
            bl = Math.max(bl, br);
         if (col > 0)
            bt = Math.max(bt, bb);
         if (col < column_count - 1) {
            bb = Math.max(style.border_width.top, bb);
         }
         context.rect(
            border_box.x + bl,
            border_box.y + bt,
            border_box.width  - bl - br,
            border_box.height - bt - bb
         );
      }
      context.fill();
      
      let thick;
      let color;
      context.lineCap = "butt";
      {  // Horizontal
         let cx1 = Math.floor(border_box.x);
         let cx2 = Math.floor(border_box.x + border_box.width);
         if (row === undefined || row === 0) {
            //
            // For table-cell-like boxes, we draw top borders only if there is 
            // no cell above the current cell.
            //
            thick = style.border_width.top;
            color = style.border_color.top;
            if (thick > 0) {
               context.strokeStyle = color;
               context.lineWidth   = thick;
               let cy = Math.floor(border_box.y + thick) - thick / 2;
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
            let cy = Math.floor(border_box.y + border_box.height) - thick / 2;
            context.beginPath();
            context.moveTo(cx1, cy);
            context.lineTo(cx2, cy);
            context.stroke();
         }
      }
      {  // Vertical
         let cy1 = Math.floor(border_box.y);
         let cy2 = Math.floor(border_box.y + border_box.height);
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
               let cx = Math.floor(border_box.x) + thick / 2;
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
            let cx = Math.floor(border_box.x + border_box.width) - thick / 2;
            context.beginPath();
            context.moveTo(cx, cy1);
            context.lineTo(cx, cy2);
            context.stroke();
         }
      }
      
      if (content_paint_functor) {
         this.#draw_within_rect(style.calcContentBox(border_box), content_paint_functor);
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
      
      this.#paint_box(
         this.#cached_styles["header-row"],
         new DOMRect(x, y, canvas_width, addenda.header_height)
      );
      
      x += styles["header-row"].contentLeft;
      y += styles["header-row"].contentTop;
      let w = canvas_width - styles["header-row"].contentInsetHorizontal;
      let h = addenda.header_height - styles["header-row"].contentInsetVertical;
      let r = x + w;
      
      context.font = styles["header-cell"].font;
      
      let col_x     = 0;
      let col_names = [];
      if (this.model) {
         for(let c of this.model.columns)
            col_names.push(c.name);
      }
      for(let i = 0; i < this.#computed_column_widths.length; ++i) {
         let w = this.#computed_column_widths[i];
         if (i > 0) {
            x = col_x;
            if (i == this.#computed_column_widths.length - 1) {
               w = r - x;
            }
         } else {
            w -= x;
         }
         this.#paint_box(
            this.#cached_styles["header-cell"],
            new DOMRect(x, y, w, h),
            0,
            i,
            function(content_box) {
               const text = col_names[i];
               context.fillStyle = styles["header-cell"].color;
               context.fillText(text, 0, 0);
               let width = context.measureText(text).width;
               if (width > content_box.w) {
                  // TODO: implement tooltips on headers
               }
            }
         );
         col_x += this.#computed_column_widths[i];
      }
   }
   
   #paint_item_row(row, indent, item, name, is_expanded) {
      let canvas  = this.#canvas;
      let context = canvas.getContext("2d");
      
      const canvas_width  = canvas.width;
      const canvas_height = canvas.height;
      let   row_height    = this.#cached_style_addenda.row_height;
      const indent_width  = this.#cached_styles["cell"].font_size;
      
      let is_selected = this.#allow_selection && this.#selected_item === item;
      
      let x = 0;
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
      
      this.#paint_box(
         this.#cached_styles[is_selected ? "selected-row" : "row"],
         new DOMRect(x, y, canvas_width, row_height)
      );
      const row_style = this.#cached_styles["row"];
      let row_inset_l = row_style.border_width.left   + row_style.padding.left;
      let row_inset_t = row_style.border_width.top    + row_style.padding.top;
      let row_inset_r = row_style.border_width.right  + row_style.padding.right;
      let row_inset_b = row_style.border_width.bottom + row_style.padding.bottom;
      let row_inner_h = row_height - row_inset_t - row_inset_b;
      
      let paint_row = row;
      if (row_inset_t > 0 && row_inset_b > 0) {
         paint_row = 0; // HACK: ensure top/bottom borders don't collapse
      }
      
      let cell_x = row_inset_l;
      let cell_y = row_inset_t;
      let cell_h = row_inner_h;
      for(let i = 0; i < this.#computed_column_widths.length; ++i) {
         let cell_w = this.#computed_column_widths[i];
         if (i == 0) {
            cell_w -= row_inset_l;
         } else if (i == this.#computed_column_widths.length - 1) {
            cell_w -= row_inset_r;
         }
         let tooltip;
         this.#paint_box(
            this.#cached_styles[is_selected ? "selected-cell" : "cell"],
            new DOMRect(x + cell_x, y + cell_y, cell_w, cell_h),
            null,
            null,
            (function(content_box) {
               let data_x = 0;
               if (i == 0) {
                  let x = indent * indent_width;
                  let y = 0;
                  if (is_expanded || this.model.itemHasChildren(item)) {
                     //
                     // Item can have children. Draw a twisty.
                     //
                     const w  = this.#cached_styles.twisty.width;
                     const h  = this.#cached_styles.twisty.height;
                     const dy = (content_box.height - h) / 2;
                     context.fillStyle = this.#cached_styles[is_selected ? "selected-twisty" : "twisty"].color;
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
                     context.beginPath();
                     painted.coords.twisty = {
                        x: content_box.x + x,
                        y: content_box.y + y + dy,
                        w: w,
                        h: h,
                     };
                  }
                  x += this.#cached_styles.twisty.width; // advance past the twisty (or where it would be)
                  
                  context.font = this.#cached_styles.cell.font;
                  x += context.measureText(" ").width;
                  
                  data_x = x;
               }
               this.#draw_within_rect(
                  new DOMRect(data_x, 0, content_box.width - data_x, content_box.height),
                  (function(content_box) {
                     let overflowed = false;
                     
                     let data = this.model.getItemCellContent(item, i, is_selected);
                     tooltip = this.model.getItemTooltip(item, i);
                     if (data || data === 0) {
                        let fulltext = this.#draw_cell_content(content_box, data, is_selected, !!tooltip);
                        tooltip = tooltip || fulltext;
                     }
                  }).bind(this)
               );
            }).bind(this)
         );
         if (tooltip) {
            let override_bounds = null;
            if (i == 0) {
               //
               // Don't show the tooltip overtop the twisty or indent.
               //
               context.font = this.#cached_styles.cell.font;
               //
               let push = indent * indent_width;
               push += this.#cached_styles.cell.contentLeft;
               push += this.#cached_styles.twisty.width;
               push += context.measureText(" ").width;
               override_bounds = new DOMRect(x + cell_x + push, y + cell_y, cell_w - push, cell_h);
            }
            this.#update_tooltip(row, i, tooltip, override_bounds);
         }
         cell_x += cell_w;
      }
      this.#last_repaint_result.rows.push(painted);
   }
   
   static #font_regex = /(?:(?<style>normal|italic|oblique|-?(?:\d+|\d+\.\d+|\.\d+)deg) )?(?:(?<variant>normal|small-caps) )?(?:(?<weight>\d+|normal|bold|bolder|lighter) )?(?:(?<width>normal|(?:ultra-|extra-|semi-)?(?:condensed|expanded)) )?(?:(?<size>-?[\d\.]+[^\d\s]+) )?(?:\/ (?<lineheight>[\d\.]+[^\d\s]+) )?(?<family>.+)/i;
   
   static #decompose_font(str) {
      let match = str.match(TreeRowViewElement.#font_regex);
      if (match)
         return match.groups;
      return null;
   }
   static #recompose_font(data) {
      let out = "";
      if (data.style)
         out += data.style + ' ';
      if (data.variant)
         out += data.variant + ' ';
      if (data.weight)
         out += data.weight + ' ';
      if (data.width)
         out += data.width + ' ';
      if (data.size)
         out += data.size + ' ';
      if (data.lineheight)
         out += '/' + data.lineheight + ' ';
      if (data.family)
         out += data.family;
      return out;
   }
   
   #draw_cell_content(content_box, bbcode, is_selected, has_own_tooltip) {
      let x = 0;
      let y = 0;
      let context = this.#canvas.getContext("2d");
      
      let parsed     = parseBBCode(bbcode);
      let overflowed = false;
      let plaintext  = "";
      
      let _render;
      _render = (function(items) {
         for(let item of items) {
            //
            // Skip drawing the content, but still gather text-content so we can use it in a tooltip.
            //
            if (!overflowed) {
               overflowed = x >= content_box.width;
               if (overflowed && has_own_tooltip)
                  return;
            }
            if (overflowed) {
               if (item+"" === item) {
                  plaintext += item;
               } else if (item.children) {
                  _render(item.children);
               }
               continue;
            }
            //
            // Draw the content.
            //
            if (item+"" === item) {
               plaintext += item;
               context.fillText(item, x, y);
               x += context.measureText(item).width;
               continue;
            }
            if (!item.children || !item.children.length) {
               continue;
            }
            let prior_y    = y;
            let prior_fill = context.fillStyle;
            let prior_font = context.font;
            let changed    = true;
            switch (item.name) {
               case "b":
               case "i":
               case "u":
                  {
                     let data = TreeRowViewElement.#decompose_font(prior_font);
                     if (data) {
                        switch (item.name) {
                           case "b": data.weight = "bold"; break;
                           case "i": data.style  = "italic"; break;
                        }
                        context.font = TreeRowViewElement.#recompose_font(data);
                     }
                  }
                  break;
               case "color":
                  context.fillStyle = item.data;
                  break;
               case "style":
                  {
                     let style = this.#cached_styles[item.data];
                     if (style && style instanceof TreeRowViewElement.TextStyle) {
                        context.fillStyle = style.color;
                        context.font      = style.font;
                        y = (content_box.height - style.font_size) / 2;
                     }
                  }
                  break;
               default:
                  changed = false;
                  break;
            }
            _render(item.children);
            context.fillStyle = prior_fill;
            context.font      = prior_font;
            y = prior_y;
         }
      }).bind(this);
      {
         let style = this.#cached_styles[is_selected ? "selected-cell" : "cell"];
         context.fillStyle = style.color;
         context.font      = style.font;
         y = (content_box.height - style.font_size) / 2;
      }
      _render(parsed);
      
      if (has_own_tooltip)
         return null;
      if (x > content_box.width)
         return plaintext;
      return null;
   }
   
   #queue_repaint(also_styles) {
      if (also_styles)
         this.#recache_queued = true;
      if (this.#repaint_queued)
         return;
      this.#repaint_queued = true;
      window.setTimeout(this.repaint.bind(this), 1);
   }
   
   #recompute_column_widths() {
      let canvas  = this.#canvas;
      let context = canvas.getContext("2d");
      context.font = this.#cached_styles["cell"].font;
      
      const CH = context.measureText("0").width;
      const EM = (function() {
         let metrics = context.measureText("X");
         return metrics.emHeightAscent - metrics.emHeightDescent;
      })();
      
      let base_width = canvas.width;
      let flex_width = base_width;
      let flex_sum   = 0;
      if (!this.model) {
         this.#computed_column_widths = [ base_width ];
         return;
      }
      
      const columns = this.model.columns;
      this.#computed_column_widths = [];
      
      for(let i = 0; i < columns.length; ++i) {
         const col     = columns[i];
         let   desired = col.width;
         
         this.#computed_column_widths[i] = null;
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
            width = CH * parseFloat(desired);
         } else if (desired.endsWith("em")) {
            width = EM * parseFloat(desired);
         }
         if (+width > 0) {
            this.#computed_column_widths[i] = width;
            flex_width -= width;
         } else {
            col.width = "1fr"; // correct width
            flex_sum += 1;
         }
      }
      for(let i = 0; i < columns.length; ++i) {
         const col = columns[i];
         if (this.#computed_column_widths[i] !== null)
            continue;
         let flex = parseFloat(col.width) / flex_sum;
         this.#computed_column_widths[i] = flex_width * flex;
      }
   }
   
   repaint() {
      this.#repaint_queued = false;
      if (this.#recache_queued) {
         this.#recache_queued = false;
         this.#recache_styles();
      }
      
      let canvas    = this.#canvas;
      canvas.width  = this.#scroll_pane.clientWidth;
      canvas.height = this.#scroll_pane.clientHeight;
      let context   = canvas.getContext("2d");
      context.textBaseline = "top";
      
      {  // Prep for tooltip changes.
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
      
      this.#recompute_column_widths();
      
      const ROW_HEIGHT = this.#cached_style_addenda.row_height;
      
      let current_row    = 0;
      let current_indent = 0;
      let current_scroll = this.#scroll_pos;
      let first_row_y    = this.#cached_style_addenda.header_height;
      let canvas_end_y   = current_scroll.y + this.offsetHeight;
      this.#last_repaint_result.rows = [];
      
      let paint_item;
      let paint_children = (function(/*Optional<T>*/ item) {
         let children = this.model.getItemChildren(item);
         if (children) {
            if (Array.isArray(children)) {
               for(let i = 0; i < children.length; ++i) {
                  paint_item(i, children[i]);
               }
            } else {
               for(const [key, child] of children) {
                  paint_item(key, child);
               }
            }
         }
      }).bind(this);
      //
      paint_item = (function(name, /*T*/ item) {
         let rows     = 1;
         let expanded = false;
         if (this.#expanded_items.has(item))
            expanded = true;
         
         let y = first_row_y + current_row * ROW_HEIGHT;
         if (y + ROW_HEIGHT > current_scroll.y && y < canvas_end_y) {
            this.#paint_item_row(current_row, current_indent, item, name, expanded);
         }
         ++current_row;
         if (expanded) {
            ++current_indent;
            paint_children(item);
            --current_indent;
         }
      }).bind(this);
      
      if (this.model) {
         paint_children(null);
      }
      
      this.#content_size.height = first_row_y + current_row * ROW_HEIGHT;
      this.#paint_sticky_header();
      
      this.#scroll_sizer.style.height = `${this.#content_size.height}px`;
      
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
customElements.define("tree-row-view-element", TreeRowViewElement);