# Differences Between Wren and Cardinal

## Stricter Rules For Fibers

In Wren, you may not `call(..)` a running Fiber, but you may `transfer(..)` to
one. In Cardinal this is not allowed (this may change in the future).

## Exclusive Range Operator

Wren uses the triple dot `...` for exclusive range and the double dot `..` for
inclusive range. In practice we found these easy to confuse visually, so the
exclusive range operator has been changed to `..<`.

## Precedence on Assignments

In Wren the `=` symbol is always lowest precedence, including situations where
it appears midway through a 

## Compound Assignments

Cardinal supports compound assignments, e.g. `x += 5`. These do not exist in
Wren. They are available for the @todo operators, and are implemented as a 
call each to the getter and setter.

## No Attributes

Cardinal presently does not support attributes. This may change in the future.

## Variable Case

Wren uses `dromedaryCase` for variables and `CamelCase` for class names and
globals. Cardinal prefers `snake_case` for the former and `Title_Snake_Case`
for the latter. This is mostly a matter of personal preference except that
core methods have been renamed to follow suit (e.g. `Obj.toString` becomes
`Obj.to_string`).

## Additional Methods on String



## Explicit Declaration of Fields

Wren declares fields implicitly when they are first referenced. We found this
to be the greatest source of errors in our code: a variable would be used in
two places, subtly misspelled in one, and the could would eventually crash with
a confusing error.

Cardinal does not implicitly declare fields when they are first referenced.
The `field` and `static field` statements allow you to explicitly declare them.

## GC Parameters are Tunable from within Wren
