We parse expressions using two stacks.

Method calls, once present on the Operator stack, have already
emitted the code to put all their arguments on the stack (because
all PUSHes must be generated in expression order).
