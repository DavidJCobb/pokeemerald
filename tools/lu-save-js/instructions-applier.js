
class InstructionsApplier {
   constructor() {
      this.root_data = null; // CStructInstance
      this.variables = {
         loop_counters: {}, // [name] == value
         transformed:   {}, // [name] == CValueInstance
      };
      
      this.bitstream = null;
   }
   
   resolve_value_path(/*CValuePath*/ path) {
      let value = this.root_data;
      let rank  = 0;
      for(let segm of path.segments) {
         if (segm.type === null || segm.type == "member-access") {
            for(let memb of value.members) {
               if (memb.name == segm.what) {
                  
                  console.assert(false, "TODO");
                  
               }
            }
         } else if (segm.type == "array-access") {
            let index = segm.what;
            if (!isNaN(+index)) {
               index = this.variables.loop_counters[index];
               console.assert(index !== undefined);
            }
            
            console.assert(false, "TODO");
            
         } else {
            console.assert(false, "unreachable");
         }
         
         // TODO: set `value` to the CValueInstance to which this segment referred
         console.assert(false, "TODO");
      }
      return value;
   }
   
   apply(/*RootInstructionNode*/ inst) {
      for(let node of inst.instructions) {
         if (node instanceof LoopInstructionNode) {
            
         } else if (node instanceof TransformInstructionNode) {
            
         } else if (node instanceof UnionSwitchInstructionNode) {
            
         } else if (node instanceof PaddingInstructionNode) {
            this.bitstream.skip_reading_bits(node.bitcount);
         } else if (node instanceof SingleInstructionNode) {
            
         }
         
         if (node instanceof ParentInstructionNode) {
            this.apply(node);
         }
      }
   }
};