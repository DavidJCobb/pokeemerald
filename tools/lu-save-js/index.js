
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
});