import AbstractPokeStringPrinter from "./printer.js";
import CHARMAP from "./charmap.js";

export default class DOMPokeStringPrinter extends AbstractPokeStringPrinter {
   constructor() {
      super();
      this.#reset();
   }
   
   #reset() {
      this.result    = new DocumentFragment();
      this.container = this.result;
      this.pending   = "";
      this.charset   = "latin";
      this.forced_fixed_width = 0;
   }
   
   #commit_pending_text() {
      if (!this.pending)
         return;
      if (this.forced_fixed_width > 0) {
         let frag = new DocumentFragment();
         for(let i = 0; i < this.pending.length; ++i) {
            let node = document.createElement("span");
            node.classList.add("letter");
            node.textContent = this.pending[i];
            frag.append(node);
         }
         this.container.append(frag);
      } else {
         //
         // Sadly, we have to wrap bare runs of text in a SPAN so we can set 
         // vertical-align: top on it, in order to have un-shifted text line 
         // up properly relative to vertically shifted text.
         //
         let node = document.createElement("span");
         node.textContent = this.pending;
         this.container.append(node);
      }
      this.pending = "";
   }
   
   #enter_formatting() {
      if (this.pending) {
         this.#commit_pending_text();
      }
      let span = document.createElement("span");
      this.container.append(span);
      this.container = span;
      return span;
   }
   
   #set_text_content(node, text) {
      if (this.forced_fixed_width <= 0) {
         node.textContent = text;
         return;
      }
      let frag = new DocumentFragment();
      for(let i = 0; i < text.length; ++i) {
         let item = document.createElement("span");
         item.classList.add("letter");
         item.textContent = text[i];
         frag.append(item);
      }
      node.replaceChildren(frag);
   }
   
   post_print_handler() {
      this.#commit_pending_text();
   }
   
   #handle_ligature_entity(content) {
      let node = document.createElement("span");
      node.classList.add("ligature");
      node.textContent = content;
      this.container.append(node);
   }
   #handle_pokeblock_entity(name) {
      let ch   = name.substring(("Pokeblock").length);
      let node = document.createElement("span");
      node.classList.add("ligature");
      node.classList.add("skinny");
      node.style.setProperty("--char-count", ch.length);
      node.textContent = ch;
      this.container.append(node);
   }
   #handle_superscript_entity(name) {
      name = name.substring(("super").length);
      let node = document.createElement("sup");
      node.classList.add("ligature");
      node.textContent = name;
      this.container.append(node);
   }
   #handle_super_subscript_entity(char_a, char_b) {
      let node = document.createElement("span");
      node.classList.add("ligature");
      node.classList.add("sup-sub");
      node.style.setProperty("--sup-char", `"${char_a}"`);
      node.style.setProperty("--sub-char", `"${char_b || " "}"`);
      this.container.append(node);
   }
   #handle_entity(name) {
      this.#commit_pending_text();
      switch (name) {
         case "PokeblockPO":
         case "PokeblockKE":
            this.#handle_super_subscript_entity(name[name.length - 2], name[name.length - 1]);
            return;
         case "PokeblockBL":
         case "PokeblockOC":
         case "PokeblockK":
            this.#handle_pokeblock_entity(name);
            return;
         case "pk":
         case "mn":
            this.#handle_super_subscript_entity(
               name[0].toUpperCase(),
               name[1].toUpperCase()
            );
            return;
         case "Lv":
            this.#handle_ligature_entity("Lv");
            return;
      }
      if (name.startsWith("super")) {
         this.#handle_superscript_entity(name);
         return;
      }
   }
   
   handle_char(cc) {
      let ch = CHARMAP.codepoint_to_character(cc, this.charset);
      if (!ch) {
         let bytes = cc;
         if (bytes >= 0x0100) {
            bytes = [0xF9, cc & 0xFF];
         }
         let name = CHARMAP.get_character_set(this.charset).canonical_entity_name_for(bytes);
         if (name) {
            this.#handle_entity(name);
         }
         return;
      }
      switch (ch) {
         case '\v':
         case '\f':
         case '\n':
            this.#commit_pending_text();
            //
            // TODO: Check what font, etc., we have, and preserve those settings.
            //
            // TODO: Change how we handle "inline" formatting (i.e. anything that 
            //       doesn't shift/skip coordinates): store it as state on the 
            //       printer, so we can restore it here.
            //
            this.container = this.result;
            this.container.append(document.createElement("br"));
            return;
      }
      if (ch) {
         this.pending += ch;
         return;
      }
   }
   
   handle_keypad_icon(index) {
      // TODO
   }
   
   handle_extra_symbol(index) {
      this.handle_char(0x0100 | index);
   }
   
   handle_ctrl_location_end() {}
   
   handle_ctrl_set_color(type, index) {
      let node = this.#enter_formatting();
      node.style.setProperty("--" + type, index);
   }
   handle_ctrl_set_all_colors(color, highlight, shadow) {
      let node = this.#enter_formatting();
      node.style.setProperty("--color",     color);
      node.style.setProperty("--highlight", highlight);
      node.style.setProperty("--shadow",    shadow);
   }
   handle_ctrl_set_font(index) {
      let node = this.#enter_formatting();
      node.style.setProperty("--font", index);
   }
   handle_ctrl_reset_font(index) {
      let node = this.#enter_formatting();
      node.style.setProperty("--font", "initial");
   }
   
   handle_ctrl_pause(delay) {}
   handle_ctrl_wait_for_sound() {}
   handle_ctrl_play_music(index) {}
   
   handle_ctrl_shift(axis, by) {
      let node = this.#enter_formatting();
      node.classList.add("shift");
      node.style.setProperty("--shift-" + axis, by + "px");
   }
   
   handle_ctrl_fill_window(axis, by) {
      this.#reset();
   }
   
   handle_ctrl_play_sound(index) {}
   
   handle_ctrl_clear_pixels_ahead(by) {
      this.#commit_pending_text();
      let node = document.createElement("span");
      node.classList.add("clear");
      node.style.width = by + "px";
      this.container.append(node);
   }
   
   handle_ctrl_skip_pixels(by) {
      this.#commit_pending_text();
      let node = document.createElement("span");
      node.classList.add("skip");
      node.style.width = by + "px";
      this.container.append(node);
   }
   
   handle_ctrl_clear_to_x_coordinate(x) {
      this.#commit_pending_text();
      // TODO
   }
   
   handle_ctrl_set_letter_spacing(v) {
      if (this.forced_fixed_width == v) {
         return;
      }
      this.forced_fixed_width = v;
      if (v > 0) {
         let node = this.#enter_formatting();
         node.classList.add("force-fixed-width");
         node.style.setProperty("--fixed-width", v + "px");
      }
   }
   
   handle_ctrl_set_character_set(what) {
      this.charset = what;
   }
   
   handle_ctrl_pause_music() {}
   handle_ctrl_resume_music() {}
   
   handle_string_substitution(index) {
      let subst = [
         "",
         "PLAYER",
         "STRING_VAR_1",
         "STRING_VAR_2",
         "STRING_VAR_3",
         "PLAYER_HONORIFIC",
         "RIVAL",
         "EMERALD",
         "AQUA",
         "MAGMA",
         "ARCHIE",
         "MAXIE",
         "KYOGRE",
         "GROUDON",
      ];
      if (index >= subst.length)
         return;
      let text = subst[index];
      if (!text)
         return;
      if (index > 0 && index < 7) {
         this.#commit_pending_text();
         let node = document.createElement("span");
         node.classList.add("placeholder");
         this.#set_text_content(node, text);
         this.container.append(node);
         return;
      }
      this.pending += text;
   }
   
};