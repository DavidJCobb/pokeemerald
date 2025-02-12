
class InstructionNode {
   constructor() {
   }
};

class ParentInstructionNode extends InstructionNode {
   constructor() {
      super();
      this.instructions = [];
   }
   from_xml(node) {
      for(let child of node.children) {
         let cls = null;
         switch (child.nodeName) {
            case "loop":      cls = LoopInstructionNode; break;
            case "padding":   cls = PaddingInstructionNode; break;
            case "switch":    cls = UnionSwitchInstructionNode; break;
            case "transform": cls = TransformInstructionNode; break;
            
            // Singles
            case "unknown":
            case "omitted":
            case "boolean":
            case "buffer":
            case "integer":
            case "pointer":
            case "string":
            case "struct":
            case "union-internal-tag":
            case "union-external-tag":
               cls = SingleInstructionNode;
               break;
         }
         if (!cls)
            throw new Error("unexpected tag");
         
         let ins = new cls();
         this.instructions.push(ins);
         ins.from_xml(child);
      }
   }
};

class RootInstructionNode extends ParentInstructionNode {
   constructor() {
      super();
   }
};

class LoopInstructionNode extends ParentInstructionNode {
   constructor() {
      super();
      this.array = new CValuePath();
      this.indices = {
         start: 0,
         count: 0,
      };
      this.counter_variable = null; // String
   }
   from_xml(node) {
      console.assert(node.nodeName == "loop");
      this.array.parse(node.getAttribute("array"));
      this.indices.start    = +node.getAttribute("start");
      this.indices.count    = +node.getAttribute("count");
      this.counter_variable = node.getAttribute("counter-var");
      
      super.from_xml(node);
   }
};

class TransformInstructionNode extends ParentInstructionNode {
   constructor() {
      super();
      this.to_be_transformed = new CValuePath();
      this.transformed_type  = null; // String typename
      this.transform_through = [];   // Array<String> of typenames
      this.transformed_var   = null; // String variable name
   }
};

class UnionSwitchInstructionNode extends InstructionNode {
   constructor() {
      super();
      this.tag       = new CValuePath();
      this.cases     = {}; // this.cases[rhs] == ParentInstructionNode
      this.else_case = null;
   }
   from_xml(node) {
      console.assert(node.nodeName == "switch");
      this.tag.parse(node.getAttribute("operand"));
      for(let child of node.children) {
         if (child.nodeName == "case") {
            let value    = +child.getAttribute("value");
            let casenode = new ParentInstructionNode();
            this.cases[value] = casenode;
            casenode.from_xml(child);
         } else if (child.nodeName == "fallback-case") {
            console.assert(!this.else_case);
            let casenode = new ParentInstructionNode();
            this.else_case = casenode;
            casenode.from_xml(child);
         }
      }
   }
};

class PaddingInstructionNode extends InstructionNode {
   constructor() {
      super();
      this.bitcount = 0;
   }
   from_xml(node) {
      this.bitcount = +node.getAttribute("bitcount");
   }
};

class SingleInstructionNode extends InstructionNode {
   constructor() {
      super();
      this.type    = null;
      this.value   = new CValuePath(); // value to serialize
      this.options = {};
   }
   from_xml(node) {
      this.type = node.nodeName;
      this.value.parse(node.getAttribute("value"));
      switch (this.type) {
         case "boolean":
            break;
         case "buffer":
            this.options = {
               bytecount: +node.getAttribute("bytecount"),
            };
            break;
         case "integer":
            this.options = {
               bitcount: +node.getAttribute("bitcount"),
               min:      +node.getAttribute("min") || 0,
               max:      null
            };
            {
               let max = node.getAttribute("max");
               if (max !== null)
                  this.options.max = +max;
            }
            break;
         case "pointer":
            break;
         case "string":
            this.options = {
               length: +node.getAttribute("length"),
               needs_terminator: node.getAttribute("nonstring") != "true",
            };
            break;
         case "struct":
            break;
         /*// Tagnames used for `member` definitions, NOT `single` instruction nodes
         case "transform":
            this.options = {
               transformed_type: node.getAttribute("transformed-type"),
            };
            break;
         case "union-internal-tag":
         case "union-external-tag":
            this.options = {
               tag: node.getAttribute("tag"),
            };
            break;
         //*/
      }
   }
};
