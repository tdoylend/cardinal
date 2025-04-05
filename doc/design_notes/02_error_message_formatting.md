# Design Notes - Error Formatting

I'm trying to make the error messages in this new language as helpful as can
be. There are a couple of components to that. One is, the messages need to be
really descriptive. There is a general tendency in programming language to
produce very terse error messages:

These are not helpful to beginners -- and the main utility of error messages
is specifically for them, because experts will usually just Ctrl-G to the
line of interest and spot the error by intuition. Reading the message itself is
generally more trouble than it's worth.

So: we need to make the messages descriptive. We can start by using complete
sentences in the King's English rather than bastardized caveman-speak. Another
part of this is, rather than referring to line number, we should refer to line
and point out the offending token or token range:

```
In file `main.car`, on line 376:
    
    var x_velocity = 3 * direction
                       ^

Error: The left operand is a Num, but the right operand is a String. These
       cannot be multiplied together.
```

This explains exactly which operator had the type mismatch. This is useful 
when a line has a lot of complicated math going on -- which operator *exactly*
is responsible for the error?

There are a lot of places we can sit down and do some careful thinking to
make the error message as useful as possible to the user. Sometimes this
involves multiple line references:

```
In module `main`, on line 552:

    var x_velocity
        ^^^^^^^^^^

Error: This variable was already declared earlier.
Note:  The previous declaration was on line 376:

    var x_velocity = 3 * direction
        ^^^^^^^^^^
```

But we don't just want the error messages to be helpful. The output needs to
be formatted in such a way that it can be integrated with a lot of tools. For
example, if you were viewing the `main` source file in an IDE, the above output
should have clickable links for line 552 and 376 that take you to those lines
in the editor. 

# Warnings

There are a bunch of things in Cardinal that, while not invalid syntax, are
usually incorrect code and can be flagged early. The most obvious is constants.
In Python, convention dictates that a constant is written in all caps:

    TILE_SIZE = 16

Cardinal follows the same convention:

    var TILE_SIZE = 16

The trouble is, nothing stops you from modifying the constant later:

    TILE_SIZE += 5

This flies in the face of what a "constant" is assumed to be. However,
constants are just specially-named variables; it's not an error to modify
those. We could make it an error, but that seems unnecessary and adds a rule
that might confuse first-time users. Instead, we can issue a warning:

```
In module `main`, on line 912:
    
    TILE_SIZE += 5
    ^^^^^^^^^^^^

Warning: The variable `TILE_SIZE` is in all-caps and so is likely intended to
         be a constant; however, it is being modified here.
```

Another good place for a warning is if the user calls a method which is not
defined. Obviously this produces a runtime error. But can we detect it at
compile time? Not quite -- because we don't know the type of the object at
compile time, so we don't know what methods it might have. However we can do
*one* thing that is surprisingly useful: If the user calls a method that is
*never implemented on any class, anywhere in the program*, that is almost
certainly a typo and not valid code:

```
In module `main`, on line 1312:

    System.printf("Hello world!")

Warning: None of the classes in the code implement `printf(_)`. It seems likely
         to be a typo.
```


