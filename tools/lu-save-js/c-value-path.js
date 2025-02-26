
class CValuePathSegment {
   constructor() {
      this.type     = null; // null (root segment), "array-access", "member-access"
      this.what     = null; // member name; array element index; array index var name
      this.resolved = null; // Optional<Variant<CValue, CStruct, CUnion>>
   }
};

class CValuePath {
   constructor() {
      this.segments = [];   // Array<CValuePathSegment>
      
      this.head_segment_dereference_count = 0;
   }
   
   parse(str) {
      // Syntax examples:
      //    a
      //    a.b
      //    a[0]
      //    a[__var]
      //    (*a).b
      //    (*a)[0]
      //    (*a)[__var]
      //    a.b.c
      //    a.b[0]
      //    a.b[__var]
      // And so on.
      
      if (!str)
         return;
      let i = 0;
      {
         let match = str.match(/^\((\**)([^\*\)]+)\)/);
         if (match) {
            this.head_segment_dereference_count = match[1].length;
            let segm = new CValuePathSegment();
            segm.type = null;
            segm.what = match[2];
            this.segments.push(segm);
            i = match[0].length;
         } else {
            match = str.match(/^([^\[\.]+)/);
            console.assert(!!match);
            let segm = new CValuePathSegment();
            segm.type = null;
            segm.what = match[1];
            this.segments.push(segm);
            i = match[0].length;
         }
      }
      while (i < str.length) {
         let c = str[i];
         if (c == '.') {
            ++i;
            let match = str.substring(i).match(/^([^\[\.]+)/);
            console.assert(!!match);
            let segm = new CValuePathSegment();
            segm.type = "member-access";
            segm.what = match[1];
            this.segments.push(segm);
            i += match[0].length;
            continue;
         }
         if (c == '[') {
            ++i;
            let match = str.substring(i).match(/^([^\]]+)/);
            console.assert(!!match);
            let segm = new CValuePathSegment();
            segm.type = "array-access";
            segm.what = match[1];
            if (!isNaN(+match[1]))
               segm.what = +segm.what;
            this.segments.push(segm);
            i += match[0].length;
            i += 1; // ']'
            continue;
         }
         console.assert(false, "unreachable", str, i, c);
      }
   }
};