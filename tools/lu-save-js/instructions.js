
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
      this.array = new InstructionNodeValuePath();
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
      this.to_be_transformed = new InstructionNodeValuePath();
      this.transformed_type  = null; // String typename
      this.transform_through = [];   // Array<String> of typenames
      this.transformed_var   = null; // String variable name
   }
   from_xml(node) {
      this.to_be_transformed.parse(node.getAttribute("value"));
      this.transformed_type  = node.getAttribute("transformed-type");
      this.transformed_var   = node.getAttribute("transformed-value");
      
      super.from_xml(node);
   }
};

class UnionSwitchInstructionNode extends InstructionNode {
   constructor() {
      super();
      this.tag       = new InstructionNodeValuePath();
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
      this.value   = new InstructionNodeValuePath(); // value to serialize
      this.options = new CBitpackOptions();
   }
   from_xml(node) {
      this.type = node.nodeName;
      this.value.parse(node.getAttribute("value"));
      this.options.from_xml(node, true);
   }
};
