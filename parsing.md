This is a page where we think about how to do expression parsing techniques. This is arguably the
last hard thing about the parser.

some representative expressions:

```wren

(1) x = a.b.c = 2

(2) 1 + 3e8 = x

(3) 1 + g = x

```

# Generating Example 1

```asm
?a
.b
2
.c=(_)
!x
```

Ops:  

Data:  

Codegen:
```asm
?a
.b
2
.c=(_)
!x
```


# Generating Example 2

```asm

```

# Example 3

## Code

```wren
1 + g = x
```

Ops: + 
Data: 1 
