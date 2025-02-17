
class CUnion {
   constructor() {
      this.node = null;
      
      this.symbol  = null;
      this.tag     = null;
      this.members = []; // Array<CValue>
      
      this.internal_tag_name    = null;
      this.members_by_tag_value = {};
   }
   
   // `node` should be a `union` element
   from_xml(node) {
      this.node   = node;
      this.symbol = node.getAttribute("name");
      this.tag    = node.getAttribute("tag");
      
      for(let child of node.children) {
         switch (child.nodeName) {
            case "members":
               this.members_from_xml(child);
               break;
            case "union-options":
               this.internal_tag_name = child.getAttribute("tag");
               break;
         }
      }
   }
   members_from_xml(node) { // `node` should be a `members` element
      for(let item of node.children) {
         let member = new CValue();
         member.from_xml(item);
         this.members.push(member);
         
         let tv = item.getAttribute("union-tag-value");
         if (tv !== null) {
            this.members_by_tag_value[tv] = member;
         }
      }
   }
};

class CUnionInstance {
   constructor(/*SaveFormat*/ format, /*CUnion*/ type) {
      this.save_format  = format;
      this.type         = type; // CUnion
      this.value_name   = null;
      this.value        = null;
      this.external_tag = null; // Optional<CValueInstance>
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
      if (this.internal_tag_name) {
         //
         // Keep the internal tag (and all other shared members-of-members).
         //
         if (this.value !== null) {
            if (this.value instanceof CStructInstance && this.value.members[this.type.internal_tag_name]) {
               common_members = {};
               for(let name in this.value.members) {
                  common_members[name] = this.value.members[name];
                  if (name == this.type.tag)
                     break;
               }
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
   
   // Returns a list of any instance-objects that couldn't be filled in.
   fill_in_defaults() {
      if (!this.value)
         return [this];
      
      let member = this.value;
      if (member instanceof CValueInstance) {
         let dv = member.base?.default_value;
         if (dv === undefined) {
            return [member];
         } else {
            member.value = dv;
         }
         return [];
      }
      return member.fill_in_defaults();
   }
};