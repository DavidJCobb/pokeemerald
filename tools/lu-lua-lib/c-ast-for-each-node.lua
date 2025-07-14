function c_ast_for_each_node(
   ast_root,
   functor
)
   local function _exec(ast_node)
      functor(ast_node)
      
      if c_ast.primary_expression.is(ast_node) then
         if ast_node.case == "constant"
         or ast_node.case == "identifier"
         or ast_node.case == "string-literal"
         then
            return
         end
         if ast_node.case == "parenthetical" then
            return _exec(ast_node.data.expression)
         end
         error("unhandled primary-expression case")
      end
      do -- postfix-expression
         if c_ast.compound_literal.is(ast_node) then
            error("Parsing of compound literals isn't fully implemented yet")
         end
         if c_ast.array_subscript_expression.is(ast_node) then
            _exec(ast_node.array)
            _exec(ast_node.index)
            return
         end
         if c_ast.call_expression.is(ast_node) then
            _exec(ast_node.callable)
            for _, v in ipairs(ast_node.arguments) do
               _exec(v)
            end
            return
         end
         if c_ast.postfix_expression.is(ast_node) then
            if ast_node.case == "dereferencing-member-access"
            or ast_node.case == "member-access"
            then
               _exec(ast_node.struct)
               return
            end
            if ast_node.case == "operator" then
               _exec(ast_node.subject)
               return
            end
            error("unhandled postfix-expression case")
         end
      end
      do -- unary-expression
         if c_ast.sizeof_expression.is(ast_node) then
            if not ast_node.is_of_typename then
               _exec(ast_node.subject)
            end
            return
         end
         if c_ast.unary_operator_expression.is(ast_node) then
            _exec(ast_node.subject)
            return
         end
      end
      if c_ast.cast_expression.is(ast_node) then
         _exec(ast_node.subject)
         return
      end
      if c_ast.operator_tree_expression.is(ast_node) then
         local function _operate(node)
            for i = 1, #node.terms do
               local term = node.terms[i]
               if term.item_type == "node" then
                  _operate(term)
                  return
               end
               term = term.data
               if c_ast.expression.is(term) then
                  _exec(term)
               end
            end
         end
         return _operate(ast_node.tree)
      end
      if c_ast.conditional_expression.is(ast_node) then
         _exec(ast_node.condition)
         _exec(ast_node.branches.if_true)
         _exec(ast_node.branches.if_false)
         return
      end
      if c_ast.assignment_expression.is(ast_node) then
         _exec(ast_node.lhs)
         _exec(ast_node.rhs)
         return
      end
      if c_ast.comma_expression.is(ast_node) then
         _exec(ast_node.discarded)
         _exec(ast_node.retained)
         return
      end
      error("unhandled AST node type")
   end
   _exec(ast_root)
end