
// Similar to InstructionsApplier, but translates from one save format to another.
function translate_value_instance_tree(
   /*SaveFormat*/      src_format,
   /*CStructInstance*/ src_root,
   /*SaveFormat*/      dst_format,
   /*CStructInstance*/ dst_root
) {
   let result = {
      dst:         dst_root,
      lost_values: [], // instance-objects in src that weren't found in dst
      missing:     [], // instance-objects in dst that weren't found in src AND weren't defaulted
      alterations: [],
   };
   function _translate(src, dst) {
      function _on_no_dst_for_src() {
         result.lost_values.push(src);
      }
      function _on_no_src_for_dst() {
         if (!(dst instanceof CValueInstance)) {
            result.missing = result.missing.concat(dst.fill_in_defaults());
         } else {
            let dv = dst.base?.default_value;
            if (dv === undefined) {
               result.missing.push(dst);
            } else {
               dst.value = dv;
            }
         }
      }
      
      if (src && !dst) {
         _on_no_dst_for_src();
         return;
      }
      if (dst && !src) {
         _on_no_src_for_dst();
         return;
      }
      if (dst.constructor != src.constructor) {
         _on_no_dst_for_src();
         _on_no_src_for_dst();
         return;
      }
      
      if (dst instanceof CStructInstance) {
         for(let key in dst.members)
            if (src.members[key])
               _translate(src.members[key], dst.members[key]);
         return;
      }
      if (dst instanceof CUnionInstance) {
         let tag_value = null;
         if (dst.external_tag) {
            tag_value = dst.external_tag.value;
         } else if (dst.type.internal_tag_name) {
            if (dst.type.internal_tag_name == src.type.internal_tag_name) {
               let value_inst = src.value.members[dst.type.internal_tag_name];
               if (value_inst)
                  tag_value = value_inst.value;
            }
         }
         if (tag_value === null) {
            _on_no_dst_for_src();
            return;
         }
         let member_base = dst.type.members_by_tag_value[tag_value];
         if (!member_base) {
            if (src.value !== null) {
               _on_no_dst_for_src();
            }
            return;
         }
         if (src.value === null) {
            _on_no_src_for_dst();
            return;
         }
         dst.get_or_emplace(member_base.name);
         _translate(src.value, dst.value);
         return;
      }
      if (dst instanceof CValueInstanceArray) {
         let i   = 0;
         let end = Math.max(src.values.length, dst.values.length);
         for(; i < end; ++i) {
            _translate(src.values[i], dst.values[i]);
         }
         return;
      }
      if (dst instanceof CValueInstance) {
         if (src.value === null) {
            _on_no_src_for_dst();
            return;
         }
         
         function _log_type_conversion() {
            result.alterations.push({
               src:  src,
               dst:  dst,
               type: "type-conversion",
               from: src.decl.type,
               to:   dst.decl.type,
            });
         }
         function _log_size_truncation(src_size, dst_size) {
            result.alterations.push({
               src:  src,
               dst:  dst,
               type: "size-truncation",
               from: src_size,
               to:   dst_size,
            });
         }
         
         let value = null;
         switch (dst.decl.type) {
            case "boolean":
               switch (src.decl.type) {
                  case "boolean":
                     value = src.value;
                     break;
                  case "integer":
                     value = src.value != 0;
                     _log_type_conversion();
                     break;
               }
               break;
            case "buffer":
               {
                  const dst_size = dst.decl.options.bytecount;
                  
                  let src_size;
                  let i = 0;
                  if (src.decl.type == "buffer") {
                     src_size = src.value.byteLength;
                     
                     let buffer = new ArrayBuffer(dst_size);
                     value = new DataView(buffer);
                     
                     let stop = Math.min(src_size, dst_size);
                     for(; i < stop; ++i)
                        value.setUint8(i, src.value.getUint8(i));
                  } else if (src.decl.type == "string") {
                     src_size = Math.min(src.decl.options.max_length, src.value.bytes.length);
                     
                     let buffer = new ArrayBuffer(dst_size);
                     value = new DataView(buffer);
                     
                     let stop = Math.min(src_size, dst_size);
                     for(; i < stop; ++i)
                        value.setUint8(i, src.value.bytes[i]);
                  } else {
                     break;
                  }
                  for(; i < dst_size; ++i)
                     value.setUint8(i, 0);
                  if (src_size > dst_size)
                     _log_size_truncation(src_size, dst_size);
               }
               break;
            case "integer":
               switch (src.decl.type) {
                  case "boolean":
                     value = src.value ? 1 : 0;
                     _log_type_conversion();
                     break;
                  case "integer":
                     value = src.value;
                     break;
               }
               break;
            case "pointer":
               if (src.decl.type != dst.decl.type) {
                  break;
               }
               value = src.value;
               break;
            case "string":
               {
                  const EOS      = CHARSET_CONTROL_CODES.chars_to_bytes["\0"];
                  const dst_size = dst.decl.options.length;
                  
                  let src_size;
                  let i = 0;
                  if (src.decl.type == "string") {
                     src_size = 0;
                     for(; src_size < src.value.bytes.length; ++src_size)
                        if (src.value.bytes[src_size] == EOS)
                           break;
                     
                     value = new PokeString();
                     
                     let stop = Math.min(src_size, dst_size);
                     for(; i < stop; ++i)
                        value.bytes.push(src.value.bytes[i]);
                  } else if (src.decl.type == "buffer") {
                     src_size = src.value.byteLength;
                     
                     value = new PokeString();
                     
                     let stop = Math.min(src_size, dst_size);
                     for(; i < stop; ++i)
                        value.bytes.push(src.value.getUint8(i));
                  } else {
                     break;
                  }
                  for(; i < dst_size; ++i)
                     value.bytes.push(EOS);
                  if (src_size > dst_size)
                     _log_size_truncation(src_size, dst_size);
               }
               break;
            case "transform":
               //
               // TODO
               //
               break;
         }
         if (value === null) {
            _on_no_dst_for_src();
            _on_no_src_for_dst();
         } else {
            dst.value = value;
         }
      }
   }
   
   for(let key in dst_root.members) {
      _translate(src_root.members[key], dst_root.members[key]);
   }
   return result;
}