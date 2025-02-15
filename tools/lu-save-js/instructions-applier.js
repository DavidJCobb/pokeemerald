
class InstructionsApplier {
   constructor() {
      this.save_format = null; // SaveFormat
      this.root_data   = null; // CStructInstance
      this.variables   = {
         loop_counters: {}, // [name] == value
         transformed:   {}, // [name] == CValueInstance
      };
      
      this.bitstream = null;
   }
   
   /*Variant<CStructInstance, CUnionInstance, CValueInstance, CValueInstanceArray>*/
   resolve_value_path(/*CValuePath*/ path) {
      let value = this.root_data;
      let rank  = 0;
      for(let segm of path.segments) {
         if (segm.type === null || segm.type == "member-access") {
            if (value instanceof CUnionInstance) {
               value = value.get_or_emplace(segm.what);
            } else {
               console.assert(value instanceof CStructInstance);
               value = value.members[segm.what];
            }
            console.assert(!!value);
         } else if (segm.type == "array-access") {
            console.assert(value instanceof CValueInstanceArray);
            
            let index = segm.what;
            if (isNaN(+index)) {
               index = this.variables.loop_counters[index];
               console.assert(index !== undefined);
            }
            
            value = value.values[index];
         } else {
            console.assert(false, "unreachable");
         }
      }
      return value;
   }
   
   apply(/*RootInstructionNode*/ inst) {
      for(let node of inst.instructions) {
         if (node instanceof LoopInstructionNode) {
            for(let i = 0; i < node.indices.count; ++i) {
               this.variables.loop_counters[node.counter_variable] = node.indices.start + i;
               this.apply(node);
            }
         } else if (node instanceof TransformInstructionNode) {
            
            // TODO
            
         } else if (node instanceof UnionSwitchInstructionNode) {
            let tag_value = this.resolve_value_path(node.tag);
            console.assert(tag_value instanceof CValueInstance);
            console.assert(tag_value.base.type == "integer" || tag_value.base.type == "boolean");
            console.assert(!isNaN(+tag_value.value));
            
            let case_node = node.cases[+tag_value.value];
            if (case_node) {
               this.apply(case_node);
            } else if (node.else_case) {
               this.apply(node.else_case);
            } else {
               throw new Error("unexpected union tag value, and the serialization format has no fallback for this! (outdated codegen or a codegen bug?)");
            }
         } else if (node instanceof PaddingInstructionNode) {
            this.bitstream.skip_reading_bits(node.bitcount);
         } else if (node instanceof SingleInstructionNode) {
            let value = this.resolve_value_path(node.value);
            if (node.type == "struct") {
               console.assert(value instanceof CStructInstance);
               let /*CStruct*/ type = value.type;
               console.assert(!!type.instructions);
               
               // spawn a new InstructionsApplier
               //  - have it use our same save format
               //  - have it use our same bitstream (not a copy)
               //     - that way, our bitstream is advanced to the end of the struct too
               //  - set its `root_data` to the target CValueInstance
               //  - leave its `variables` blank
               //     - these variables are scoped to sectors or whole-struct functions
               //     - in essence, you make and run a new InstructionsApplier for each such scope
               //  - run the new InstructionsApplier on the CStruct's `instructions`
               let whole_struct_applier = new InstructionsApplier();
               whole_struct_applier.save_format = this.save_format;
               whole_struct_applier.root_data   = value;
               whole_struct_applier.bitstream   = this.bitstream;
               whole_struct_applier.apply(type.instructions);
            } else {
               console.assert(value instanceof CValueInstance);
               switch (node.type) {
                  case "omitted":
                     value.value = value.base.default_value;
                     break;
                  
                  case "boolean":
                     value.value = this.bitstream.read_bool();
                     break;
                     
                  case "buffer":
                     {
                        let buffer = new ArrayBuffer(node.options.bytecount);
                        let view   = new DataView(buffer);
                        value.value = view;
                        for(let i = 0; i < node.options.bytecount; ++i) {
                           let byte = this.bitstream.read_unsigned(8);
                           view.setUint8(i, byte);
                        }
                     }
                     break;
                     
                  case "integer":
                     value.value = this.bitstream.read_unsigned(node.options.bitcount) + node.options.min;
                     break;
                     
                  case "pointer":
                     value.value = this.bitstream.read_unsigned(32);
                     break;
                     
                  case "string":
                     value.value = this.bitstream.read_string(node.options.length);
                     break;
               }
            }
         }
      }
   }
};