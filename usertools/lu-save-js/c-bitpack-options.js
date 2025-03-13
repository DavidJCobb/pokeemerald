
class CBitpackOptions {
   constructor() {
      this.type = null;
   }
   from_xml(node, is_value_node) {
      if (is_value_node) {
         this.type = node.nodeName;
      } else {
         switch (node.nodeName) {
            case "integral-options":
            case "string-options":
            case "transform-options":
            case "unknown-options":
               this.type = node.nodeName.substring(0, node.nodeName.indexOf("-"));
               break;
            case "opaque-bufferoptions":
               this.type = "buffer";
               break;
            case "union-options":
               this.type = "union-external-tag";
               if (node.getAttribute("is-internal") == "true") {
                  this.type = "union-internal-tag";
               }
               break;
         }
      }
      switch (this.type) {
         case "boolean":
            break;
         case "buffer":
            this.bytecount = +node.getAttribute("bytecount");
            break;
         case "integer":
            this.bitcount = +node.getAttribute("bitcount");
            for(let key of ["min", "max"]) {
               let v = node.getAttribute(key);
               if (v !== null)
                  v = +v;
               this[key] = v;
            }
            break;
         case "pointer":
            break;
         case "string":
            this.length           = +node.getAttribute("length");
            this.needs_terminator = node.getAttribute("nonstring") != "true";
            break;
         case "struct":
            break;
         case "transform":
            this.transformed_type = node.getAttribute("transformed-type");
            break;
         case "union-internal-tag":
         case "union-external-tag":
            this.tag = node.getAttribute("tag");
            break;
      }
   }
};