
c_ast = {}

c_ast.node = make_class({})

c_ast.expression = make_class({
   superclass = c_ast.node,
})

local function _case_and_data_constructor(self, a, b)
   if b then
      self.case = a
      self.data = b
   else
      self.case = nil
      self.data = a
   end
end

c_ast.primary_expression = make_class({
   superclass  = c_ast.expression,
   constructor = _case_and_data_constructor,
})

c_ast.postfix_expression = make_class({
   superclass  = c_ast.expression,
   constructor = _case_and_data_constructor,
})

-- postfix-expression
c_ast.array_subscript_expression = make_class({
   superclass  = c_ast.expression,
   constructor = function(array, index)
      self.array = array
      self.index = index
   end,
})

-- postfix-expression
c_ast.call_expression = make_class({
   superclass  = c_ast.expression,
   constructor = function(func, args)
      self.callable  = func -- node
      self.arguments = args -- node[]
   end,
})

c_ast.compound_literal = make_class({
   superclass  = c_ast.postfix_expression,
   constructor = function(self, typename, initializer)
      self.typename    = typename
      self.initializer = initializer
   end,
})

-- unary-expression
c_ast.unary_operator_expression = make_class({
   superclass  = c_ast.expression,
   constructor = function(self, subject, operator)
      if not subject then
         error("unary operator expression requires a subject")
      end
      self.subject  = subject
      self.operator = operator
   end,
})

-- unary-expression
c_ast.sizeof_expression = make_class({
   superclass  = c_ast.expression,
   constructor = function(self, is_typename, subject)
      if not subject then
         error("sizeof expression requires a subject")
      end
      self.is_of_typename = is_typename
      self.subject        = subject
   end,
})

c_ast.cast_expression = make_class({
   superclass  = c_ast.expression,
   constructor = function(self, typename, subject)
      if not subject then
         error("cast expression requires a subject")
      end
      self.typename = typename -- string
      self.subject  = subject  -- node
   end,
})

c_ast.operator_tree_expression = make_class({
   superclass  = c_ast.expression,
   constructor = function(self, tree)
      self.tree = tree
   end,
})

c_ast.conditional_expression = make_class({
   superclass  = c_ast.expression,
   constructor = function(self, condition, if_true, if_false)
      self.condition = condition
      self.branches  = {
         if_true  = if_true,
         if_false = if_false
      }
   end,
})

c_ast.assignment_expression = make_class({
   superclass  = c_ast.expression,
   constructor = function(self, a, b, c)
      if c then
         self.operator = a -- string
         self.lhs      = b -- node
         self.rhs      = c -- node
      else
         self.operator = "="
         self.lhs      = a
         self.rhs      = b
      end
   end,
})

c_ast.comma_expression = make_class({
   superclass  = c_ast.expression,
   constructor = function(self, discarded, retained)
      self.discarded = discarded
      self.retained  = retained
   end,
})