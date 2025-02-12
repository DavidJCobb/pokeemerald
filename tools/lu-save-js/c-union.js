
class CUnion {
   constructor() {
      this.node = null;
      
      this.symbol  = null;
      this.tag     = null;
      this.members = []; // Array<CValue>
   }
   
   // `node` should be a `union` element
   from_xml(node) {
      this.node   = node;
      this.symbol = node.getAttribute("name");
      this.tag    = node.getAttribute("tag");
      
      for(let child of node.children) {
         if (child.nodeName == "members") {
            this.members_from_xml(child);
         }
      }
   }
   members_from_xml(node) { // `node` should be a `members` element
      for(let item of node.children) {
         let member = new CValue();
         member.from_xml(item);
         this.members.push(member);
      }
   }
};

class CUnionInstance {
   constructor(/*SaveFormat*/ format, /*CUnion*/ type) {
      this.save_format = format;
      this.type        = type; // CUnion
      this.value       = null;
   }
};