
class CUnion extends CContainerTypeDefinition {
   constructor() {
      super();
      
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
   
   /*Optional<CValue>*/ member_by_tag_value(/*Variant<int, CValueInstance>*/ tag) {
      if (tag instanceof CValueInstance) {
         tag = tag.value;
      }
      tag = +tag;
      if (isNaN(tag))
         return null;
      return this.members_by_tag_value[tag] || null;
   }
};

class CUnionInstance extends CTypeInstance {
   constructor(/*SaveFormat*/ format, /*CUnion*/ type, /*CValue*/ decl) {
      super(format, type, decl);
      this.value        = null;
      this.external_tag = null; // Optional<CValueInstance>
      if (type)
         console.assert(type instanceof CUnion);
   }
   
   // TODO: Does anything use this? If not, remove it.
   get value_name() {
      if (this.value)
         return this.value.decl.name;
      return null;
   }
   
   copy_contents_of(/*const CUnionInstance*/ other) {
      if (other.value === null) {
         this.value = null;
         return;
      }
      if (this.value === null) {
         if (this.external_tag) {
            let v = this.external_tag.value;
            if (v === null)
               throw new Error("cannot emplace this union (external tag not set)");
            let memb = this.type.members_by_tag_value[+tv];
            if (!memb)
               throw new Error("cannot emplace this union (external tag matches no member)");
            this.emplace(memb);
         } else {
            let memb = this.type.member_by_name(other.value.decl.name);
            if (!memb)
               throw new Error("cannot emplace this union (internally tagged; no member has the same name as the source's active member)");
            this.emplace(memb);
         }
      }
      this.value.copy_contents_of(other.value);
   }
   
   //
   // Internal use:
   //
   
   /*CInstance*/ emplace(/*Variant<String, CValue>*/ member) {
      console.assert(!!member);
      if (member instanceof CDefinition) {
         if (!this.type.members.includes(member))
            throw new Error("Failed to emplace a CUnionInstance's value: the specified member CDefinition does not belong to this union's type.");
      } else {
         member = this.type.member_by_name(member);
         if (!member)
            throw new Error("Failed to emplace a CUnionInstance's value: the specified member name must exist in this union's type.");
      }
      if (member instanceof CStruct) {
         this.value = new CStructInstance(this.save_format, member);
      } else if (member instanceof CUnion) {
         this.value = new CUnionInstance(this.save_format, member);
      } else {
         console.assert(member instanceof CValue);
         this.value = member.make_instance_representation(this.save_format);
      }
      this.value.is_member_of = this;
      return this.value;
   }
   /*CInstance*/ get_or_emplace(member) {
      console.assert(!!member);
      if (member instanceof CDefinition) {
         if (this.value?.decl === member)
            return this.value;
      } else {
         if (this.value?.decl.name == member)
            return this.value;
      }
      let common_members = null;
      if (this.type.internal_tag_name) {
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
      let v = this.emplace(member);
      if (common_members) {
         for(let name in common_members) {
            v.members[name].value.copy_contents_of(common_members[name].value);
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