```
[Syntax Error]
In module `main`, on line 376:

    System.pront("Hello World!")
               ^^^^^

Warning: `pront(_)` is never defined anywhere in the program.
Note:    Did you mean `print(_)`?
```

```
[Syntax Error]
In module `main`, on line 376:

    var x = b + a * d
        ^

Error:  This is a duplicate declaration of `x`.
Note:   The previous definition is on line 75:

    var x = 0
        ^
```

```
[Runtime Error]
In module `main`, on line 376:

    var f = System.open("test.txt", "a")
                  ^^^^^^^^^^^^^^^^^^^^^^

Error:  The `System` class does not implement a static method `open(_,_)`.
```

```
[Runtime Error]
In module `main`, on line 376:

    var f = File.open("test.txt", File.READ)
                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Error:  The file `test.txt` either does not exist or is access-controlled.

```
