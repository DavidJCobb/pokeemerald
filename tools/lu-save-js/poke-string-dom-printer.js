class DOMPokeStringPrinter extends AbstractPokeStringPrinter {
   constructor() {
      super();
      this.#reset();
   }
   
   #reset() {
      this.result    = new DocumentFragment();
      this.container = this.result;
      this.pending   = "";
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
   
   handle_char(cc) {
      let ch = CHARSET_ENGLISH.bytes_to_chars[cc];
      if (!ch) {
         return;
      }
      if (ch == "TALL_PLUS") {
         ch = "＋";
      }
      if (ch && ch.length == 1) {
         this.pending += ch;
         return;
      }
      if (ch.startsWith("SUPER_")) {
         let what = ch.split("_")[1];
         let node = document.createElement("sup");
         node.classList.add("ligature");
         node.textContent = ({
            "ER": "er",
            "RE": "re",
            "E":  "e",
         })[what];
         this.container.append(node);
         return;
      }
      if (ch.startsWith("PKMN_") || ch == "POKEBLOCK_PO" || ch == "POKEBLOCK_KE") {
         ch = ch.split("_")[1];
         let a = ch[0];
         let b = ch[1] || "";
         
         this.#commit_pending_text();
         let node = document.createElement("span");
         node.classList.add("ligature");
         node.classList.add("sup-sub");
         node.style.setProperty("--sup-char", `"${a}"`);
         node.style.setProperty("--sub-char", `"${b || " "}"`);
         this.container.append(node);
         
         return;
      }
      if (ch.startsWith("POKEBLOCK_")) {
         ch = ch.split("_")[1];
         
         this.#commit_pending_text();
         let node = document.createElement("span");
         node.classList.add("ligature");
         node.classList.add("skinny");
         node.style.setProperty("--char-count", ch.length);
         node.textContent = ch;
         this.container.append(node);
         
         return;
      }
      if (cc == 0xFA || cc == 0xFB || cc == 0xFE) {
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
      }
   }
   
   handle_keypad_icon(index) {
      // TODO
   }
   
   handle_extra_symbol(index) {
      // TODO
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
      // TODO
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