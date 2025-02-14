
document.querySelector(".form button").addEventListener("click", async function() {
   let data_x = null;
   let data_s = null;
   {
      let node_x = document.getElementById("file-xml");
      if (!node_x.files.length) {
         alert("Error: no save file format given");
         return;
      }
      let file = node_x.files[0];
      
      let parser = new DOMParser();
      data_x = parser.parseFromString(await file.text(), "text/xml");
   }
   {
      let node_s = document.getElementById("file-sav");
      if (!node_s.files.length) {
         alert("Error: no save file given");
         return;
      }
      let file = node_s.files[0];
      data_s = new DataView(await file.arrayBuffer());
   }
   
   let format = new SaveFormat();
   format.from_xml(data_x.documentElement);
   console.log("Save format: ", format);
   
   let decoded = format.load(data_s);
   console.log("Save dump: ", decoded);
   
   for(let i = 0; i < decoded.slots.length; ++i) {
      let slot      = decoded.slots[i];
      let container = document.querySelector(".slot[data-slot-id='" + (i + 1) + "']");
      let header    = container.querySelector("aside");
      let view      = document.createElement("c-view");
      container.replaceChildren(header, view);
      view.scope = decoded.slots[0];
      window.setTimeout(function() { view.repaint() }, 1);
      
      let version = -1;
      let counter = -1;
      let s_frag  = new DocumentFragment();
      for(let sector of slot.sectors) {
         let s_item = document.createElement("li");
         s_frag.append(s_item);
         if (sector.signature == 0x8012025) {
            s_item.className   = sector._checksum_is_valid ? "good" : "bad";
            s_item.textContent = sector.sector_id;
            version = Math.max(version, sector.version);
            counter = Math.max(counter, sector.counter);
         }
      }
      header.querySelector(".sectors").replaceChildren(s_frag);
      header.querySelector("[data-field='version'] .value").textContent = version;
      header.querySelector("[data-field='counter'] .value").textContent = counter;
   }
});