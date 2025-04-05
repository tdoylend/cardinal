[Warning]

Module `lib.hal`, line 25, in `syscall(_,_)`:

    if (flags.contanis("readonly")) {
              ^^^^^^^^

Warning: The method `containis(_)` is not defined anywhere in the program. It
         seems likely to be a typo.
Note:    Perhaps you meant `contains(_)`?

[Runtime Error]

Module `lib.hal`, line 17, in `syscall(_,_)`:
    
    var rptr = base + offset
                    ^
Error:   The types are mismatched. The left operand is a Num, but the right
         operand is a Null.

[Traceback]

 - Module `lib.hal`, line 218, in `sleep(_)`
 - Module `main`, line 319, in `run()`


[Runtime Error]

Module `lib.hal`, line 17, in `syscall(_,_)`:
    
    var rptr = base + offset
	                ^
Error: The left operand is a Num, but the right operand is a Null.
	
## Traceback:

 - Module `lib.hal`, line 218, in `sleep(_)`
 - Module `main`, line 319, in `run()`

```c
```


-----------

[Runtime Error]

Module `lib.hal`, line 17, in `syscall(_,_)`:

Error: You may not use decimal numbers as syscall parameters.

## Traceback:

 - Module `lib.hal`, line 218, in `sleep(_)`
 - Module `main`, line 211, in `run()`
 - Module `main`, line 1099, in `<script>`

-----------

[Syntax Error]

Module `lib.hal`, line 19:

	var class = "System" // Start here
	    ^^^^^

Error: The word `class` is reserved; you cannot use it as a variable 
	   name.

-----------

[Syntax Error]

Module `actions`, line 237:

	return "You must be a wizard to do that.
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Error: This string is missing a closing quotation mark.
Note:  There is a quotation mark on line 238. However, strings cannot span
       multiple lines unless they are triple-quoted.

------------

[Syntax Error]

Module `test`, line 1034:

    
