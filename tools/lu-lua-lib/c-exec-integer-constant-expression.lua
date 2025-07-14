
function try_execute_c_constant_integer_expression(
   ast_root,
   ordinary_name_space -- table
)
   if not ordinary_name_space then
      ordinary_name_space = {}
   end
   local function _exec(ast_node)
      if c_ast.primary_expression.is(ast_node) then
         if ast_node.case == "constant" then
            return ast_node.data
         end
         if ast_node.case == "identifier" then
            local iv = ordinary_name_space[ast_node.data]
            if iv then
               return iv
            end
            error("missing identifier", 0)
         end
         if ast_node.case == "parenthetical" then
            return _exec(ast_node.data.expression)
         end
         if ast_node.case == "string-literal" then
            error("string literal (not an integral constant)", 0)
         end
         error("unhandled primary-expression case")
      end
      do -- postfix-expression
         if c_ast.compound_literal.is(ast_node) then
            error("not a valid constant expression (bare compound literal)", 0)
         end
         if c_ast.array_subscript_expression.is(ast_node) then
            error("not a valid constant expression (array subscript)", 0)
         end
         if c_ast.call_expression.is(ast_node) then
            error("not a valid constant expression (function call)", 0)
         end
         if c_ast.postfix_expression.is(ast_node) then
            if ast_node.case == "dereferencing-member-access" then
               error("not a valid constant expression (dereference and member access)", 0)
            end
            if ast_node.case == "member-access" then
               error("not a valid constant expression (member access)", 0)
            end
            if ast_node.case == "operator" then
               error("not a valid constant expression (increment/decrement operator)", 0)
            end
            error("unhandled postfix-expression case")
         end
      end
      do -- unary-expression
         if c_ast.sizeof_expression.is(ast_node) then
            error("not a computable constant expression (we can't process sizeof here)", 0)
         end
         if c_ast.unary_operator_expression.is(ast_node) then
            local op = ast_node.operator
            if op == "&" or op == "*" or op == "++" or op == "--" then
               error("not a valid constant expression (unary/prefix operator " .. op .. ")", 0)
            end
            if op == "!" or op == "-" or op == "+" or op == "~" then
               local subject = _exec(ast_node.subject)
               if op == "!" then
                  return subject == 0
               end
               if op == "-" then
                  return -tonumber(subject)
               end
               if op == "+" then
                  return tonumber(subject)
               end
               if op == "~" then
                  return ~tonumber(subject)
               end
            end
            error("unhandled unary operator")
         end
      end
      if c_ast.cast_expression.is(ast_node) then
         error("not a valid constant expression (static cast)", 0)
      end
      if c_ast.operator_tree_expression.is(ast_node) then
         local function _operate(node)
            function _term_to_number(term)
               if term.item_type == "node" then
                  return _operate(term)
               end
               term = term.data
               if c_ast.expression.is(term) then
                  return _exec(term)
               end
               return term
            end
         
            local accum = _term_to_number(node.terms[1])
            for i = 2, #node.terms do
               local term = _term_to_number(node.terms[i])
               local comparison = nil
               if node.operator == "+" then
                  accum = accum + term
               elseif node.operator == "-" then
                  accum = accum - term
               elseif node.operator == "&" then
                  accum = accum & term
               elseif node.operator == "|" then
                  accum = accum | term
               elseif node.operator == "^" then
                  accum = accum ~ term
               elseif node.operator == "*" then
                  accum = accum * term
               elseif node.operator == "/" then
                  accum = accum / term
               elseif node.operator == "%" then
                  accum = accum % term
               elseif node.operator == "<<" then
                  accum = accum << term
               elseif node.operator == ">>" then
                  accum = accum >> term
               elseif node.operator == "|" then
                  accum = accum | term
               elseif node.operator == "&" then
                  accum = accum & term
               elseif node.operator == "^" then
                  accum = accum ~ term
               elseif node.operator == "==" then
                  comparison = accum == term
               elseif node.operator == "!=" then
                  comparison = accum ~= term
               elseif node.operator == "<" then
                  comparison = accum < term
               elseif node.operator == ">" then
                  comparison = accum > term
               elseif node.operator == "<=" then
                  comparison = accum <= term
               elseif node.operator == ">=" then
                  comparison = accum >= term
               elseif node.operator == "&&" then
                  comparison = accum ~= 0 and term ~= 0
               elseif node.operator == "||" then
                  comparison = accum ~= 0 or term ~= 0
               end
               if comparison ~= nil then
                  accum = 0
                  if comparison then
                     accum = 1
                  end
               end
            end
            return accum
         end
         return _operate(ast_node.tree)
      end
      if c_ast.conditional_expression.is(ast_node) then
         local cond = _exec(ast_node.condition)
         if cond and cond ~= 0 then
            return _exec(ast_node.branches.if_true)
         else
            return _exec(ast_node.branches.if_false)
         end
      end
      if c_ast.assignment_expression.is(ast_node) then
         error("not a valid constant expression (assignment)", 0)
      end
      if c_ast.comma_expression.is(ast_node) then
         return _exec(ast_node.retained)
      end
      error("unhandled AST node type")
   end
   return _exec(ast_root)
end