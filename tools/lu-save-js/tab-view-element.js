class TabViewElement extends HTMLElement {
   #active_tab_body  = null;
   #child_observer   = null;
   #resize_observer  = null;
   #title_observer   = null;
   #shadow;
   
   #scroll_button_container;
   #tab_nodes = {
      list: [], // tabs, not bodies
      bodies_to_tabs: new Map(), // Map<Element body, Element tab>
      tabs_to_bodies: new Map(), // Map<Element tab,  Element body>
   };
   #tab_strip;
   
   constructor() {
      super();
      this.#shadow = this.attachShadow({ mode: "open", slotAssignment: "manual" });
      this.#shadow.innerHTML = `
         <link rel="stylesheet" href="tab-view-element.css" />
         <div class="tab-strip-wrap" part="tab-strip">
            <ul class="tab-strip" aria-role="tablist">
            </ul>
            <span></span>
            <div class="scroll-buttons">
               <button aria-hidden="true" part="scroll-button left">&lt;</button>
               <button aria-hidden="true" part="scroll-button right">&gt;</button>
            </div>
         </div>
         <div class="current-tab-body" part="tab-body" aria-role="tabpanel">
            <div>
               <slot></slot>
            </div>
         </div>
      `.trim();
      
      this.#child_observer = new MutationObserver(this.#on_tree_mutations_observed.bind(this));
      this.#child_observer.observe(this, {
         childList: true,
         subtree:   false,
      });
      
      this.#title_observer = new MutationObserver(this.#on_tab_body_mutations_observed.bind(this));
      
      this.#resize_observer = new ResizeObserver(this.#on_resize.bind(this));
      this.#resize_observer.observe(this);
      
      let strip = this.#tab_strip = this.#shadow.querySelector(".tab-strip");
      strip.addEventListener("click", this.#on_tab_interaction.bind(this));
      strip.addEventListener("keypress", this.#on_tab_interaction.bind(this));
      strip.addEventListener(
         ("onwheel" in document) ? "wheel" : "mousewheel",
         this.#on_tabstrip_wheel.bind(this)
      );
      
      let scroll = this.#scroll_button_container = this.#shadow.querySelector(".scroll-buttons");
      scroll.children[0].addEventListener("click", this.#scroll_tab_strip.bind(this, -1));
      scroll.children[1].addEventListener("click", this.#scroll_tab_strip.bind(this, 1));
   }
   
   //
   // Accessors
   //
   
   get selectedTabBody() {
      return this.#active_tab_body;
   }
   get selectedTabIndex() {
      if (!this.#active_tab_body)
         return -1;
      return this.children.indexOf(this.#active_tab_body);
   }
   set selectedTabBody(node) {
      this.#select_tab(node);
   }
   set selectedTabIndex(i) {
      if (i < 0 || i >= this.children.length)
         throw new Error("Out-of-bounds tab index.");
      this.#select_tab(this.children[i]);
   }
   
   //
   // Events
   //
   
   #on_tab_interaction(e) {
      let tab = e.target.closest(".tab-strip>li");
      if (!tab)
         return;
      if (e.target.closest(".tab-strip>li>button.close")) {
         if (e.type != "click")
            return;
         e.preventDefault();
         this.#remove_tab(tab);
         return;
      }
      if (e instanceof KeyboardEvent) {
         switch (e.key) {
            case "Enter":
            case "Select":
            case " ":
               break;
            default:
               return;
         }
         e.preventDefault();
      }
      let body = this.#tab_nodes.tabs_to_bodies.get(tab);
      console.assert(!!body);
      this.#select_tab(body);
   }
   
   #on_tabstrip_wheel(e) {
      if (e.deltaX)
         return;
      //
      // Adapt vertical mousewheel events for horizontally scrolling 
      // the tab strip.
      //
      let direction = 0;
      if (e.deltaY < 0) {
         direction = -1;
      } else if (e.deltaY > 0) {
         direction = 1;
      } else {
         return;
      }
      this.#scroll_tab_strip(direction);
   }
   
   #on_tab_body_mutations_observed(records) {
      let changed = false;
      for(const record of records) {
         let body = record.target;
         let tab  = this.#tab_nodes.bodies_to_tabs.get(body);
         if (!tab)
            continue;
         changed = true;
         tab.textContent = body.getAttribute("data-title");
         if (body.hasAttribute("data-tab-is-closable")) {
            let close = tab.querySelector("button.close");
            if (!close) {
               tab.append(this.#make_tab_close_button());
            }
         }
      }
      if (changed)
         this.#update_scroll_button_visibility();
   }
   
   #on_tree_mutations_observed(records) {
      let removed    = new Set();
      let switch_tab = false;
      let switch_to  = null; // if the active tab is removed, switch to this tab
      let added      = false;
      let reordered  = false;
      for(const record of records) {
         for(const node of record.removedNodes) {
            removed.add(node);
            if (node === this.#active_tab_body) {
               switch_tab = true;
               switch_to  = record.previousSibling || record.nextSibling;
            }
         }
         for(const node of record.addedNodes) {
            if (removed.has(node)) {
               removed.delete(node);
               reordered = true;
            } else {
               added = true;
            }
         }
      }
      for(const node of removed) {
         this.#destroy_tab_node(node);
      }
      this.#update_tab_node_list();
      if (added) {
         //
         // If we were previously empty (or otherwise had no selected 
         // tab), then select our first tab.
         //
         if (!switch_to && !this.#active_tab_body) {
            switch_tab = true;
            switch_to  = this.children[0];
         }
      }
      if (switch_tab) {
         this.#select_tab(switch_to);
      }
   }
   
   #on_resize(entries) {
      this.#update_scroll_button_visibility();
   }
   
   //
   // Internals (tab-strip nodes)
   //
   
   #remove_tab(tab) {
      let body = this.#tab_nodes.tabs_to_bodies.get(tab);
      if (!body) {
         return;
      }
      this.#destroy_tab_node(body);
      let switch_tab = false;
      let switch_to  = null;
      if (body === this.#active_tab_body) {
         switch_tab = true;
         switch_to  = body.previousSibling || body.nextSibling;
      }
      body.remove();
      this.dispatchEvent(new CustomEvent("tabremove", { detail: {
         tabBody: body,
      }}));
      if (switch_tab) {
         this.#active_tab_body = null;
         this.#select_tab(switch_to);
         if (!switch_to) {
            this.#dispatch_tab_change(null);
         }
      }
   }
   
   #destroy_tab_node(body) {
      let node = this.#tab_nodes.bodies_to_tabs.get(body);
      if (!node)
         return;
      node.remove();
      let i = this.#tab_nodes.list.indexOf(node);
      if (i >= 0)
         this.#tab_nodes.list.splice(i, 1);
      this.#tab_nodes.bodies_to_tabs.delete(body);
      this.#tab_nodes.tabs_to_bodies.delete(node);
   }
   
   #make_tab_close_button() {
      let close = document.createElement("button");
      close.classList.add("close");
      close.setAttribute("part", "close-button");
      close.textContent = "Close";
      return close;
   }
   
   #make_tab_node(body) {
      let node  = document.createElement("li");
      node.setAttribute("aria-role", "tab");
      node.setAttribute("part",      "tab");
      node.setAttribute("tabindex",  0);
      let title = body.getAttribute("data-title");
      if (title) {
         node.textContent = title;
      } else {
         node.textContent = "unnamed";
      }
      if (body.hasAttribute("data-tab-is-closable")) {
         node.append(this.#make_tab_close_button());
      }
      this.#tab_nodes.bodies_to_tabs.set(body, node);
      this.#tab_nodes.tabs_to_bodies.set(node, body);
      this.#title_observer.observe(body, {
         attributes: true,
         attributeFilter: [ "data-title", "data-tab-is-closable" ],
      });
      return node;
   }
   
   #update_tab_node_list() {
      let list      = [];
      let partnames = [];
      for(let i = 0; i < this.children.length; ++i) {
         let body = this.children[i];
         let tab  = this.#tab_nodes.bodies_to_tabs.get(body);
         if (!tab)
            tab = this.#make_tab_node(body);
         list.push(tab);
         
         let partname = "tab";
         if (body == this.#active_tab_body)
            partname += " selected";
         if (i == 0)
            partname += " first";
         else if (i == this.children.length - 1)
            partname += " last";
         else
            partname += " middle";
         partnames.push(partname);
      }
      this.#tab_nodes.list = list;
      this.#tab_strip.replaceChildren(...list);
      for(let i = 0; i < list.length; ++i)
         list[i].setAttribute("part", partnames[i]);
      this.#update_scroll_button_visibility();
   }
   
   #update_scroll_button_visibility() {
      let can_scroll = this.#tab_strip.clientWidth < this.#tab_strip.scrollWidth;
      this.#scroll_button_container.classList[can_scroll ? "add" : "remove"]("can-scroll");
   }
   
   //
   // Internals (behavior)
   //
   
   #dispatch_tab_change(body) {
      this.dispatchEvent(new CustomEvent("tabchange", { detail: {
         tabBody: body,
      }}));
   }
   
   static #REGEX_TAB_IS_CURRENT = /(?:^|\s)selected(?:\s|$)/;
   static #REGEX_MAKE_TAB_NON_CURRENT = /^selected$|selected\s|\sselected$/;
   
   #select_tab(body) {
      let slot = this.#shadow.querySelector(".current-tab-body slot");
      if (!body) {
         body = this.children[0];
         if (!body) {
            //
            // We have no tabs.
            //
            this.#active_tab_body = null;
            slot.assign();
            return;
         }
      }
      if (body.parentNode != this) {
         throw new Error("The tab view does not own the given tab-body element.");
      }
      
      let changed = this.#active_tab_body !== body;
      this.#active_tab_body = body;
      
      let tab    = this.#tab_nodes.bodies_to_tabs.get(body);
      let former = this.#tab_strip.querySelector("[part~='selected']");
      if (former !== tab && (tab || former)) {
         if (former) {
            let parts = former.getAttribute("part");
            parts = parts.replace(TabViewElement.#REGEX_MAKE_TAB_NON_CURRENT, "");
            former.setAttribute("part", parts);
         }
         if (tab) {
            let tab_parts = tab.getAttribute("part");
            if (!TabViewElement.#REGEX_TAB_IS_CURRENT.test(tab_parts)) {
               tab_parts += " selected";
               tab.setAttribute("part", tab_parts);
            }
         }
      }
      slot.assign(body);
      if (changed) {
         this.#dispatch_tab_change(body);
      }
   }
   
   // NOTE: Does not account for the presence/absence of scroll-margin on the 
   // tabs.
   #get_leftmost_scrolled_tab_index() {
      let tabs = this.#tab_nodes.list;
      if (!tabs.length)
         return -1;
      let strip_rect = this.#tab_strip.getBoundingClientRect();
      for(let i = 0; i < tabs.length; ++i) {
         let rect = tabs[i].getBoundingClientRect();
         if (rect.left <= strip_rect.left && rect.right >= strip_rect.right) {
            //
            // This tab is wider than the tab strip itself, and we are scrolled 
            // into its middle.
            //
            return -2;
         }
         rect.x -= strip_rect.left;
         if (rect.x >= 0)
            return i;
      }
      return -1;
   }
   
   #scroll_tab_strip(/*Variant<int, Element tab>*/ target) {
      const IS_NATIVE_CODE_USABLE = false;
      //
      // A native-code implementation only produces correct results 
      // if the browser supports the `container` property on the 
      // options object for Element.scrollIntoView, AND if the 
      // browser rounds scroll positions rather than truncating 
      // them.
      //
      const strip = this.#tab_strip;
      if (IS_NATIVE_CODE_USABLE) {
         let index = this.#get_leftmost_scrolled_tab_index();
         if (index < 0) {
            if (index == -2) {
               strip.scrollLeft += 20 * target;
            }
            return;
         }
         let tab = this.#tab_nodes.list[index + target];
         if (tab)
            tab.scrollIntoView({ container: "nearest", inline: "start", behavior: "instant" });
         else if (target < 0)
            strip.scrollLeft = 0;
         else
            strip.scrollLeft = Infinity;
         return;
      }
      //
      // Absent a viable native-code approach, we have to manually 
      // identify the tabs' positions relative to the tab strip, 
      // get their scroll-margin-left, and scroll them appropriately.
      //
      const TOLERANCE = 2;
      if (!target)
         return;
      let tabs = this.#tab_nodes.list;
      if (!tabs.length)
         return;
      if (target instanceof Element) {
         if (!tabs.includes(target))
            throw new Error("This tab view does not own the specified element.");
         target.scrollIntoView();
         return;
      }
      let strip_rect = strip.getBoundingClientRect();
      let i;
      for(i = 0; i < tabs.length; ++i) {
         let rect = tabs[i].getBoundingClientRect();
         if (rect.left <= strip_rect.left && rect.right >= strip_rect.right) {
            //
            // A particular tab is wider than the entire tab strip, and 
            // is scrolled into view. Scroll by pixels.
            //
            strip.scrollLeft += 20 * target;
            return;
         }
         if (target > 0) {
            //
            // If we're scrolling rightward, then we'll base our scrolling on 
            // the leftmost visible tab. This means that if the leftmost tab 
            // is fully scrolled into view, we'll scroll leftward to reveal 
            // an off-screen tab; whereas if the leftmost tab is partially 
            // scrolled into view, we'll scroll just far enough leftward to 
            // reveal the tab.
            //
            // This case is a bit tricky because we have to account for the 
            // tabs' scroll-margin-left (since we also account for it while 
            // scrolling). Some tabbar styles may want to emulate collapsing 
            // borders for tabs by deleting the left border of all tabs after 
            // the first, and then using scroll-margin-left to ensure that 
            // when we scroll that tab into view, we also scroll the previous 
            // sibling's righthand border into view.
            //
            let x = Math.round(rect.x - strip_rect.left);
            let m = parseFloat(getComputedStyle(tabs[i]).scrollMarginLeft) || 0;
            if (x - m >= 0)
               break;
            x = Math.round(rect.right - strip_rect.left);
            m = 0;
            if (i + 1 < tabs.length)
               m = parseFloat(getComputedStyle(tabs[i + 1]).scrollMarginLeft) || 0;
            if (x - m > 0)
               break;
         } else {
            const TOLERANCE = 4;
            //
            // If we're scrolling leftward, then we'll base our scrolling on 
            // the leftmost fully-in-view tab. This means that if a tab is 
            // scrolled partially into view, we'll scroll that tab fully into 
            // view.
            //
            if (Math.round(rect.x - strip_rect.left) >= 0)
               break;
         }
      }
      if (i >= tabs.length) { // failed to find a leftmost tab?
         strip.scrollLeft = 0;
         return;
      }
      let node = tabs[i + target];
      if (!node) { // there aren't any more tabs in the desired direction?
         if (target > 0) {
            //
            // Special-case: scroll as far rightward as possible, respecting 
            // the last tab's scroll-margin-right.
            //
            node = tabs[tabs.length - 1];
            let margin = parseFloat(getComputedStyle(node).scrollMarginRight);
            let x      = node.getBoundingClientRect().x - strip_rect.x + margin;
            x += strip.scrollLeft;
            strip.scrollLeft = Math.round(x);
            return;
         }
         node = tabs[0];
      }
      let rect   = node.getBoundingClientRect();
      let margin = parseFloat(getComputedStyle(node).scrollMarginLeft);
      let x      = node.getBoundingClientRect().x - strip_rect.x - margin;
      x += strip.scrollLeft;
      strip.scrollLeft = Math.round(x);
   }
};
customElements.define("tab-view", TabViewElement);