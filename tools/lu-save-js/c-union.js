
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
      
      for(let child of node.children) {
         switch (child.nodeName) {
            case "members":
               this.members_from_xml(child);
               break;
            case "union-options":
               this.tag = child.getAttribute("tag");
               break;
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
      this.value_name  = null;
      this.value       = null;
   }
   emplace(member_name) {
      for(let member of this.type.members) {
         if (member.name != member_name)
            continue;
         if (member instanceof CStruct) {
            this.value = new CStructInstance(this.save_format, member);
         } else if (member instanceof CUnion) {
            this.value = new CUnionInstance(this.save_format, member);
         } else {
            console.assert(member instanceof CValue);
            this.value = member.make_instance_representation(this.save_format);
         }
         this.value_name = member_name;
         return this.value;
      }
      console.assert(false, "invalid member name");
   }
   get_or_emplace(member_name) {
      if (this.value_name == member_name && this.value !== null)
         return this.value;
      let common_members = null;
      if (this.value !== null && this.type.tag !== null) {
         //
         // HACK: Preserve the internal tag while loading. We load the 
         //       tag (which emplaces the first union member), and then 
         //       we want to emplace the proper union member in order 
         //       to load it.
         //
         // TODO: We need a way to explicitly know that a union c-type 
         //       is internally tagged.
         //
         if (this.value instanceof CStructInstance && this.value.members[this.type.tag]) {
            common_members = {};
            for(let name in this.value.members) {
               common_members[name] = this.value.members[name];
               if (name == this.type.tag)
                  break;
            }
         }
      }
      let v = this.emplace(member_name);
      if (common_members) {
         for(let name in common_members) {
            this.value.members[name] = common_members[name];
         }
      }
      return v;
   }
};