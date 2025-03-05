
--[[--

   A class (though it doesn't use our new class system) that accepts 
   (as input to its `parse` member function) a list of c_lex_token 
   objects, and returns an AST.
   
   Each primary node in the AST has `type`, `case`, and `data` 
   properties, and most of them correspond to grammar rules as named 
   in the ISO C spec. The main exceptions are AST nodes for two-term 
   operators like addition and division: where ISO C uses separate 
   grammar rules for each operator (taking advantage of precedence 
   climbing), we instead use a single `type` value ("operator-tree"). 
   The `data` member of an operator-tree AST node is a subtree of 
   special "operator nodes" and their terms, which may be AST nodes.
   
   Currently, code for attempting to execute a parsed AST (to compute 
   an integer constant expression) is in `lu-save-js-indexer/main.lua`. 
   We should probably update this parser to:
   
    - Use our class system for the parser itself.
    
    - Use our class system for the AST nodes.
    
    - Use our class system for operator nodes.
    
    - Define the "compute constant integer expression" code in a 
      function in a separate file, to keep `main.lua` clean.

--]]--
c_parser = {}
do
   c_parser.__index = c_parser
   function c_parser:new()
      if self ~= c_parser then
         error("c_parser:new() is a static method and cannot be invoked on instances.")
      end
      local instance = setmetatable({}, self)
      instance.tokens = nil
      instance.i      = nil
      return instance
   end
   
   function c_parser:_require_instance()
      if self == c_parser then
         error("Instance methods must be invoked using ':' on an instance.")
      end
   end
   
   --
   -- Instance methods
   --
   
   function c_parser:parse(tokens)
      self:_require_instance()
      self.tokens = tokens
      self.index  = 1
      
      local result = self:parse_expression()
      if self.index >= #self.tokens + 1 then
         self.has_garbage = false
         return result
      end
      error("garbage content at the end")
   end
   
   function c_parser:peek_token(n)
      if not n then
         n = 0
      end
      return self.tokens[self.index + n]
   end
   function c_parser:consume_token()
      local token = self.tokens[self.index]
      self.index = self.index + 1
      return token
   end
   
   function c_parser:advance(n)
      if not n then
         n = 1
      end
      self.index = self.index + n
   end
   function c_parser:rewind(n)
      if not n then
         n = 1
      end
      self.index = self.index - n
   end
   function c_parser:error(text, detail)
      if detail and stringify_table then
         text = text .. "\n" .. stringify_table(detail)
      end
      error("C parse error at " .. tostring(self.index) .. ": " .. text)
   end
   
   function c_parser:parse_typename()
      local t = self:peek_token()
      if not t then
         return nil
      end
      if t.type == "identifier" then
         return t.data
      end
      return nil
   end
   
   function c_parser:parse_primary_expression()
      local t = self:peek_token()
      if not t then
         return nil
      end
      if t.type == "identifier" or t.type == "constant" or t.type == "string-literal" then
         self:advance()
         return {
            type = "primary-expression",
            case = t.type,
            data = t.data
         }
      end
      if t.type == "token" and t.data == "(" then
         self:advance()
         local info = self:parse_expression()
         if not info then
            self:rewind()
            return nil
         end
         t = self:peek_token()
         if t and t.type == "token" and t.data == ")" then
            self:advance()
            return {
               type = "primary-expression",
               case = "parenthetical",
               data = {
                  expression = info
               }
            }
         end
         self:rewind()
      end
      return nil
   end
   function c_parser:_parse_compound_literal()
      local t = self:peek_token()
      if not t or t.type ~= "token" or t.data ~= "(" then
         return nil
      end
      
      self:advance()
      local typename = self:parse_typename()
      if not typename then
         self:rewind()
         return nil
      end
      
      local b = self:peek_token(0)
      local c = self:peek_token(1)
      if (not b or b.type ~= "token" ~= b.data ~= ")")
      or (not c or c.type ~= "token" ~= c.data ~= "{") then
         return nil
      end
      self:advance(2)
      local initializer = self:parse_initializer_list()
      if not initializer then
         self:rewind(2)
         return nil
      end
      
      local d = self:consume_token()
      if d and d.type == "token" and d.data == "," then
         d = self:consume_token()
      end
      if not d or d.type ~= "token" or d.data ~= "}" then
         self:error("Expected the end of a compound literal's initializer list.")
      end
      return {
         type = "postfix-expression",
         case = "compound-literal",
         data = {
            typename    = typename,
            initializer = initializer
         }
      }
   end
   function c_parser:parse_postfix_expression()
      local info = self:parse_primary_expression()
      if not info then
         info = self:_parse_compound_literal()
         if not info then
            return nil
         end
      end
      local t = self:peek_token()
      if not t or t.type ~= "token" then
         return info
      end
      if t.data == "[" then -- array subscript
         self:advance()
         local array = info
         local index = self:parse_expression()
         if not index then
            self:rewind()
            return info
         end
         local u = self:consume_token()
         if not u or u.type ~= "token" or u.data ~= "]" then
            self:error("Expected the end of an array subscript.")
         end
         return {
            type = "postfix-expression",
            case = "array-subscript",
            data = {
               array = array,
               index = index
            }
         }
      elseif t.data == "(" then -- function call
         self:advance()
         local arguments = {}
         local arg = nil
         repeat
            arg = self:parse_assignment_expression()
            if arg then
               arguments[#arguments + 1] = arg
               local t = self:peek_token()
               if t and t.type == "token" and t.data == "," then
                  self:consume_token()
               else
                  break
               end
            end
         until not arg
         local t = self:consume_token()
         if not t or t.type ~= "token" or t.data ~= ")" then
            self:error("Expected end of argument list.")
         end
         return {
            type = "postfix-expression",
            case = "call",
            data = {
               func      = info,
               arguments = arguments
            }
         }
      elseif t.data == "." or t.data == "->" then
         self:advance()
         local member = self:consume_token()
         if not member or member.type ~= "identifier" then
            self:error("Expected an identifier.")
         end
         local case = "member-access"
         if t.data == "->" then
            case = "dereferencing-member-access"
         end
         return {
            type = "postfix-expression",
            case = case,
            data = {
               struct = info,
               member = member.data
            }
         }
      elseif t.data == "++" or t.data == "--" then
         self:consume_token()
         return {
            type = "postfix-expression",
            case = "operator",
            data = {
               subject  = info,
               operator = t.data
            }
         }
      end
      return info
   end
   function c_parser:parse_unary_expression()
      local info = self:parse_postfix_expression()
      if info then
         return info
      end
      local t = self:peek_token()
      if not t then
         self:error("Unexpected end of stream.")
      end
      if t.type == "token" then
         if t.data == "++" or t.data == "--" then
            self:advance()
            return {
               type = "unary-expression",
               case = "operator",
               data = {
                  subject  = info,
                  operator = t.data
               }
            }
         end
         local OPERATORS = { "&", "*", "+", "-", "~", "!" }
         local found     = false
         for _, v in ipairs(OPERATORS) do
            if t.data == v then
               found = true
               break
            end
         end
         if found then
            self:advance()
            local expr = self:parse_cast_expression()
            if not expr then
               self:error("Expected an operand for a unary operator.")
            end
            return {
               type = "unary-expression",
               case = "operator",
               data = {
                  subject  = info,
                  operator = t.data
               }
            }
         end
         return nil
      end
      if t.type == "identifier" then
         if t.data ~= "sizeof" then
            self:error("Unexpected identifier.", { prior = { info, t } })
         end
         self:advance()
         local u = self:peek_token()
         if u and u.type == "token" and u.data == "(" then
            self:advance()
            local typename = self:parse_typename()
            if not typename then
               self:error("Expected typename for sizeof operator.")
            end
            u = self:consume_token()
            if not u or u.type ~= "token" or u.data ~= ")" then
               self:error("Unterminated sizeof-typename expression.")
            end
            return {
               type = "unary-expression",
               case = "sizeof-typename",
               data = typename
            }
         end
         local info = self:parse_unary_expression()
         if not info then
            self:error("Expected operand for sizeof operator.")
         end
         return {
            type = "unary-expression",
            case = "sizeof-expression",
            data = info
         }
      end
      return nil
   end
   function c_parser:parse_cast_expression()
      local info = self:parse_unary_expression()
      if info then
         return info
      end
      local t = self:peek_token()
      if not t or t.type ~= "token" or t.data ~= "(" then
         return nil
      end
      self:consume_token()
      local typename = self:parse_typename()
      if not typename then
         self:error("Expected typename for cast operator.")
      end
      t = self:consume_token()
      if not t or t.type ~= "token" or t.data ~= ")" then
         self:error("Expected terminating parenthesis for cast operator.")
      end
      info = self:parse_cast_expression()
      if not info then
         self:error("Expected operand for cast operator.")
      end
      return {
         type = "cast-expression",
         case = "cast",
         data = {
            typename = typename,
            subject  = info
         }
      }
   end
   
   --
   -- C defines separate syntax tokens for all of its operators, but 
   -- this would be highly repetitive and a pain in the neck for us, 
   -- so let's just parse operators left-to-right and opportunistically 
   -- rewrap them as precedence requires.
   --
   local OPERATOR_TABLE = {
      { "*", "/", "%" },
      { "+", "-" },
      { "<<", ">>" },
      { "<", ">", "<=", ">=" },
      { "==", "!=" },
      { "&" },
      { "^" },
      { "&&" },
      { "||" },
   }
   function c_parser:parse_two_term_expression()
      local term = self:parse_cast_expression()
      if not term then
         return nil
      end
      
      local flat = {}
      
      local t = nil
      repeat
         t = self:peek_token()
         if not t or t.type ~= "token" then
            break
         end
         local operator = nil
         local rank     = nil
         for i, v in ipairs(OPERATOR_TABLE) do
            for _, op in ipairs(v) do
               if t.data == op then
                  operator = op
                  rank     = #OPERATOR_TABLE - i
                  break
               end
            end
            if operator then
               break
            end
         end
         if not operator then
            break
         end
         self:advance()
         
         local rhs = self:parse_cast_expression()
         if not rhs then
            error("Expected righthand operand for operator " .. operator .. ".")
         end
         local size = #flat
         if size == 0 then
            flat[1] = { item_type = "value", data = term }
            size    = 1
         end
         flat[size + 1] = { item_type = "operator", operator = operator, rank = rank }
         flat[size + 2] = { item_type = "value", data = rhs }
      until false
      if #flat == 0 then
         return term
      end
      
      function coalesced_shunting_yard(list)
         local rpn = {}
         do
            local opstack = {}
            for _, item in ipairs(list) do
               if item.item_type == "value" then
                  rpn[#rpn + 1] = item
               elseif item.item_type == "operator" then
                  local size = #opstack
                  while size > 0 do
                     local last = opstack[size]
                     if last.rank >= item.rank then
                        rpn[#rpn + 1] = last -- pop from optsack to rpn
                        opstack[size] = nil
                        size = size - 1
                     else
                        break
                     end
                  end
                  opstack[size + 1] = item
               end
            end
            do -- pop remaining opstack to output
               local src_size = #opstack
               local dst_size = #rpn
               local count    = #opstack
               for i = 1, count do
                  dst_size = dst_size + 1
                  rpn[dst_size] = opstack[src_size]
                  src_size = src_size - 1
               end
            end
         end
         --
         -- Convert the RPN list into a tree.
         --
         local tree  = {}
         local stack = {}
         for i = 1, #rpn do
            local item = rpn[i]
            if item.item_type == "value" then
               stack[#stack + 1] = item
            elseif item.item_type == "operator" then
               local a = stack[#stack - 1]
               local b = stack[#stack]
               
               local coalesced = false
               if a.item_type == "node" and a.operator == item.operator then
                  if b.item_type == "node" and b.operator == item.operator then
                     local a_size = #a.terms
                     local b_size = #b.terms
                     for i = 1, b_size do
                        a_size = a_size + 1
                        a.terms[a_size] = b.terms[i]
                     end
                  else
                     a.terms[#a.terms + 1] = b
                  end
                  coalesced = true
               end
               if not coalesced then
                  local node = {
                     item_type = "node",
                     operator  = item.operator,
                     rank      = item.rank,
                     terms     = { a, b }
                  }
                  stack[#stack - 1] = node
               end
               stack[#stack] = nil
            end
         end
         return stack[1]
      end
      
      local root = coalesced_shunting_yard(flat)
      return {
         type = "operator-tree",
         case = nil,
         data = root
      }
   end
   
   function c_parser:parse_conditional_expression()
      local condition = self:parse_two_term_expression()
      if not condition then
         return nil
      end
      local t = self:peek_token()
      if not (t and t.type == "token" and t.data == "?") then
         return condition
      end
      self:advance()
      local if_true = self:parse_expression()
      t = self:consume_token()
      if not (t and t.type == "token" and t.data == ":") then
         error("Expected separator between the branches of a ternary.")
      end
      local if_false = self:parse_conditional_expression()
      if not if_false then
         error("Expected false-branch for a ternary.")
      end
      return {
         type = "conditional-expresion",
         case = nil,
         data = {
            condition = condition,
            if_true   = if_true,
            if_false  = if_false
         }
      }
   end
   function c_parser:parse_assignment_expression()
      local ternary = self:parse_conditional_expression()
      if ternary then
         return ternary
      end
      local unary = self:parse_unary_expression()
      local t  = self:consume_token()
      if not t or t.type ~= "token" then
         error("Assignment operator expected.")
      end
      
      local OPERATORS = { "=", "*=", "/=", "%=", "+=", "-=", "<<=", ">>=", "&=", "^=", "|=" }
      local found = false
      for _, v in ipairs(OPERATORS) do
         if v == t.data then
            found = true
            break
         end
      end
      if not found then
         error("Assignment operator expected.")
      end
      local rhs = self:parse_assignment_expression()
      if not rhs then
         error("Assignment righthand operand expected.")
      end
      return {
         type = "assignment-expression",
         case = nil,
         data = {
            operator = t.data,
            lhs      = unary,
            rhs      = rhs
         }
      }
   end
   function c_parser:parse_expression()
      local expr = self:parse_assignment_expression()
      if not expr then
         return nil
      end
      local t = self:peek_token()
      if t and t.type == "token" and t.data == "," then
         local other = self:parse_assignment_expression()
         return {
            type = "comma-expression",
            case = nil,
            data = {
               discarded = expr,
               retained  = other
            }
         }
      end
      return expr
   end
end