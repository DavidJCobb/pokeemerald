
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

--]]--

local instance_methods = {}
c_parser = make_class({
   instance_members = instance_methods,
})
do
   function instance_methods:parse(tokens)
      self.tokens = tokens
      self.index  = 1
      local result = self:_parse_expression()
      if self.index >= #self.tokens + 1 then
         self.has_garbage = false
         return result
      end
      error("garbage content at the end")
   end

   function instance_methods:_peek_token(n)
      if not n then
         n = 0
      end
      return self.tokens[self.index + n]
   end
   function instance_methods:_consume_token()
      local token = self.tokens[self.index]
      self.index = self.index + 1
      return token
   end
   function instance_methods:_consume_symbol(desired)
      local token = self.tokens[self.index]
      if token and token:is_symbol(desired) then
         self.index = self.index + 1
         return true
      end
      return false
   end
   
   function instance_methods:_advance(n)
      if not n then
         n = 1
      end
      self.index = self.index + n
   end
   function instance_methods:_rewind(n)
      if not n then
         n = 1
      end
      self.index = self.index - n
   end
   function instance_methods:_error(text, detail)
      if detail and stringify_table then
         text = text .. "\n" .. stringify_table(detail)
      end
      error("C parse error at " .. tostring(self.index) .. ": " .. text)
   end
   
   --
   -- AST parsing
   --
   
   function instance_methods:_parse_typename()
      local t = self:_peek_token()
      if not t then
         return nil
      end
      if t.type == "identifier" then
         return t.data
      end
      return nil
   end
   
   function instance_methods:_parse_primary_expression()
      local t = self:_peek_token()
      if not t then
         return nil
      end
      if t.type == "identifier" or t.type == "constant" or t.type == "string-literal" then
         self:_advance()
         return c_ast.primary_expression(t.type, t.data)
      end
      if t:is_symbol("(") then
         self:_advance()
         local info = self:_parse_expression()
         if not info then
            self:_rewind()
            return nil
         end
         t = self:_peek_token()
         if t and t:is_symbol(")") then
            self:_advance()
            return c_ast.primary_expression("parenthetical", { expression = info })
         end
         self:_rewind()
      end
      return nil
   end
   function instance_methods:_parse_initializer_list()
      error("Not yet implemented!")
   end
   function instance_methods:_parse_compound_literal()
      if not self:_consume_symbol("(") then
         return nil
      end
      
      local typename = self:_parse_typename()
      if not typename then
         self:_rewind()
         return nil
      end
      
      if not self:_consume_symbol(")") then
         return nil
      end
      if not self:_consume_symbol("{") then
         self:_rewind()
         return nil
      end
      local initializer = self:_parse_initializer_list()
      if not initializer then
         self:_rewind(2)
         return nil
      end
      self:_consume_symbol(",") -- swallow trailing comma
      if not self:_consume_symbol("}") then
         self:_error("Expected the end of a compound literal's initializer list.")
      end
      return c_ast.compound_literal(typename, initializer)
   end
   function instance_methods:_parse_postfix_expression()
      local info = self:_parse_primary_expression()
      if not info then
         info = self:_parse_compound_literal()
         if not info then
            return nil
         end
      end
      local t = self:_peek_token()
      if not t or t.type ~= "token" then
         return info
      end
      if t.data == "[" then -- array subscript
         self:_advance()
         local array = info
         local index = self:_parse_expression()
         if not index then
            self:_rewind()
            return info
         end
         if not self:_consume_symbol("]") then
            self:_error("Expected the end of an array subscript.")
         end
         return c_ast.array_subscript_expression(array, index)
      elseif t.data == "(" then -- function call
         self:_advance()
         local arguments = {}
         local arg = nil
         repeat
            arg = self:_parse_assignment_expression()
            if arg then
               arguments[#arguments + 1] = arg
               if not self:_consume_symbol(",") then
                  break
               end
            end
         until not arg
         if not self:_consume_symbol(")") then
            self:_error("Expected end of argument list.")
         end
         return c_ast.call_expression(info, arguments)
      elseif t.data == "." or t.data == "->" then
         self:_advance()
         local member = self:_consume_token()
         if not member or member.type ~= "identifier" then
            self:_error("Expected an identifier.")
         end
         local case = "member-access"
         if t.data == "->" then
            case = "dereferencing-member-access"
         end
         return c_ast.postfix_expression(case, {
            struct = info,
            member = member.data
         })
      elseif t.data == "++" or t.data == "--" then
         self:_consume_token()
         return c_ast.postfix_expression("operator", {
            subject  = info,
            operator = t.data
         })
      end
      return info
   end
   function instance_methods:_parse_unary_expression()
      local info = self:_parse_postfix_expression()
      if info then
         return info
      end
      local t = self:_peek_token()
      if not t then
         self:_error("Unexpected end of stream.")
      end
      if t.type == "token" then
         if t.data == "++" or t.data == "--" then
            self:_advance()
            return c_ast.unary_operator_expression(info, t.data)
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
            self:_advance()
            local expr = self:_parse_cast_expression()
            if not expr then
               self:_error("Expected an operand for a unary operator.")
            end
            return c_ast.unary_operator_expression(expr, t.data)
         end
         return nil
      end
      if t.type == "identifier" then
         if t.data ~= "sizeof" then
            self:_error("Unexpected identifier.", { prior = { info, t } })
         end
         self:_advance()
         if self:_consume_symbol("(") then
            local typename = self:_parse_typename()
            if not typename then
               self:_error("Expected typename for sizeof operator.")
            end
            if not self:_consume_symbol(")") then
               self:_error("Unterminated sizeof-typename expression.")
            end
            return c_ast.sizeof_expression(true, typename)
         end
         local info = self:_parse_unary_expression()
         if not info then
            self:_error("Expected operand for sizeof operator.")
         end
         return c_ast.sizeof_expression(false, info)
      end
      return nil
   end
   function instance_methods:_parse_cast_expression()
      local info = self:_parse_unary_expression()
      if info then
         return info
      end
      if not self:_consume_symbol("(") then
         return nil
      end
      local typename = self:_parse_typename()
      if not typename then
         self:_error("Expected typename for cast operator.")
      end
      if not self:_consume_symbol(")") then
         self:_error("Expected terminating parenthesis for cast operator.")
      end
      info = self:_parse_cast_expression()
      if not info then
         self:_error("Expected operand for cast operator.")
      end
      return c_ast.cast_expression(typename, info)
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
      { "|" },
      { "&&" },
      { "||" },
   }
   function instance_methods:_parse_two_term_expression()
      local term = self:_parse_cast_expression()
      if not term then
         return nil
      end
      
      local flat = {}
      
      local t = nil
      repeat
         t = self:_peek_token()
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
         self:_advance()
         
         local rhs = self:_parse_cast_expression()
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
      
      -- Use the shunting yard algorithm to convert a left-to-right 
      -- flat list of terms and operators into a tree of operators. 
      -- Unlike traditional shunting yard, we then take consecutive 
      -- operators of the same time and coalesce them: `a + b + c` 
      -- is one node, not two.
      function coalesced_shunting_yard(list)
         --
         -- Start by converting the expression to reverse Polish 
         -- notation (this is the shunting yard algorithm).
         --
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
      return c_ast.operator_tree_expression(root)
   end
   
   function instance_methods:_parse_conditional_expression()
      local condition = self:_parse_two_term_expression()
      if not condition then
         return nil
      end
      if not self:_consume_symbol("?") then
         return condition
      end
      self:_advance()
      local if_true = self:_parse_expression()
      if not self:_consume_symbol(":") then
         self:_error("Expected separator between the branches of a ternary.")
      end
      local if_false = self:_parse_conditional_expression()
      if not if_false then
         self:_error("Expected false-branch for a ternary.")
      end
      return c_ast.conditional_expression(condition, if_true, if_false)
   end
   function instance_methods:_parse_assignment_expression()
      local ternary = self:_parse_conditional_expression()
      if ternary then
         return ternary
      end
      local unary = self:_parse_unary_expression()
      local t     = self:_consume_token()
      if not t or t.type ~= "token" then
         self:_error("Assignment operator expected.")
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
         self:_error("Assignment operator expected.")
      end
      local rhs = self:_parse_assignment_expression()
      if not rhs then
         self:_error("Assignment righthand operand expected.")
      end
      return c_ast.assignment_expression(t.data, unary, rhs)
   end
   function instance_methods:_parse_expression()
      local expr = self:_parse_assignment_expression()
      if not expr then
         return nil
      end
      if self:_consume_symbol(",") then
         local other = self:_parse_assignment_expression()
         return c_ast.comma_expression(expr, other)
      end
      return expr
   end
end