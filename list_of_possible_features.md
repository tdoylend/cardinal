# List of Possible New Features for Cardinal

1.  First-class functions.
2.  An `assert` keyword, which can optionally be compiled out
    using a command-line switch.
3.  A built-in bytecode bundler or packed executable format.
4.  If not the above, at least a built-in obfuscator.
5.  `getter` and `setter` keywords which automatically add the
    corresponding functions to `field` definitions.
6.  A way to detect when someone has mistyped a variable name
    in a method, which will lead to us calling a method that
    does not exist. Probably the best way to do this is to
    maintain a list of times a given method is defined; if a
    method which is never defined anywhere in the source is
    called, produce a warning.
