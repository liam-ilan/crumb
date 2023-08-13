# Crumb Programming Language
## Development
### Compile
```bash
gcc src/* -Wall -o crumb
```

### run
```bash
./crumb path_to_file
```

### Debug with Valgrind
```bash
gcc src/* -g -Wall -o crumb && valgrind --leak-check=full -s ./crumb _test.crumb
```

## Syntax
### EBNF
```ebnf
program = start, statement, end;
statement = {return | assignment | value};
return = "return", value;
assignment = identifier, "=", value;
value = application | function | int | float | string | identifier;
application = "(", {value}, ")";
function = "{", [{identifier}, "->"], statement, "}";
```

### Tokens
```
"="
"("
")"
"{"
"}"
"->"
"return"
identifier
int
float
string
start
end
```

## Standard Libary
### IO
- `arguments` ✅
  - A list of arguments passed into the terminal

- `arguments_count` ✅
  - The number of items in `arguments`

- `(print arg1 arg2 arg3 ...)` ✅
  - Prints all arguments to stdout, returns nothing.

### Comparisons
- `(is a b)` ✅
  - Checks if `a` and `b` are equal, returns `1` if so, else returns `0`.

- `(less_than a b)` ✅
  - Checks if `a` is less than `b`, returns `1` if so, else returns `0`.
  - `a`: `integer` or `float`
  - `b`: `integer` or `float`

- `(greater_than a b)` ✅
  - Checks if `a` is greater than `b`, returns `1` if so, else returns `0`.
  - `a`: `integer` or `float`
  - `b`: `integer` or `float`

### Logical Operators
- `(not a)` ✅
  - Returns `0` if `a` is `1`, and `1` if `a` is `0`.
  - `a`: `integer`, which is `1` or `0`

- `(and a b)` ✅
  - Returns `1` if `a` and `b` are both `1`, else returns `0`
  - `a`: `integer`, which is `1` or `0`
  - `b`: `integer`, which is `1` or `0`

- `(or a b)` ✅
  - Returns `1` if `a` or `b` are `1`, else returns `0`
  - `a`: `integer`, which is `1` or `0`
  - `b`: `integer`, which is `1` or `0`

### Arithmetic
- `(add arg1 arg2 arg3 ...)` ✅
  - Returns `arg1` + `arg2` + `arg3` + ...
  - Requires a minimum of two args
  - `arg1`, `arg2`, `arg3`, ...: `integer` or `float`

- `(subtract arg1 arg2 arg3 ...)` ✅
  - Returns `arg1` - `arg2` - `arg3` - ...
  - Requires a minimum of two args
  - `arg1`, `arg2`, `arg3`, ...: `integer` or `float`

- `(divide arg1 arg2 arg3 ...)` ✅
  - Returns `arg1` / `arg2` / `arg3` / ...
  - Requires a minimum of two args
  - `arg1`, `arg2`, `arg3`, ...: `integer` or `float`

- `(multiply arg1 arg2 arg3 ...)` ✅
  - Returns `arg1` * `arg2` * `arg3` * ...
  - Requires a minimum of two args
  - `arg1`, `arg2`, `arg3`, ...: `integer` or `float`

- `(remainder a b)` ✅
  - Returns the remainder of `a` and `b`.
  - `a`: `integer`
  - `b`: `integer`

- `(negative a)` ✅
  - Returns -`a`.
  - `a`: `integer` or `float`

### Control
- `(loop count fn)` ✅
  - Applys `fn`, `count` times. If `fn` returns, the loop breaks, and `loop` returns whatever `fn` returned, else repeats until loop is completed.
  - `count`: `integer`, which is greater than or equal to `0`
  - `fn`: `function`, which is in the form `{n -> ...}`, where n is the current loop index (starting at `0`).

- `(if condition fn1)` or `(if condition fn1 fn2)` ✅
  - If `condition` is `1`, applys `fn1`. (like the "then" part in an if statement).
  - If `condition` is `0`, and `fn2` was supplied, apply `fn2`. (the "else" part in an if statement).
  - Returns whatever `fn1` or `fn2` return
  - `condition`: `integer`, which is `1` or `0`
  - `fn1`: `function`
  - `fn2`: `function`

### File
- `(read_file path)` ✅
  - Returns the contents of the file designated by `path`, in a string
  - `path`: `string`

- `(write_file path contents)` ✅
  - Writes the string `contents` into the file designated by `path`, returns nothing.
  - `path`: `string`
  - `contents`: `string`

### Typecasting
- `(integer a)` ✅
  - Returns `a` as an `integer`.
  - `a`: `string`, `float`, or `integer`.

- `(string a)` ✅
  - Returns `a` as a `string`.
  - `a`: `string`, `float`, or `integer`.

- `(float a)` ✅
  - Returns `a` as a `float`.
  - `a`: `string`, `float`, or `integer`.

### List and String Methods
- `(list arg1 arg2 arg3 ...)`
  - Returns a `list`, with the arguments as it's contents.

- `(join arg1 arg2 arg3 ...)`
  - Returns all args joined together.
  - All args must be of the same type.
  - `arg1`, `arg2`, `arg3`, ...: `string` or `list`.

- `(get x index1)` or `(get x index1 index2)`
  - Returns the item in `x` at `index1`. If x is a `string`, this is a single char.
  - If `index2` is supplied, returns a subarray or substring from `index1` to `index2`, not including `index2`.
  - `x`: `string` or `list`.
  - `index1`: `int`.
  - `index2`: `int`.

- `(delete x index1)` or `(delete x index1 index2)`
  - Returns a `string` or `list`, where `index1` was removed from `x`.
  - If `index2` is supplied, all items from `index1` to `index2` are removed, not including `index2`.
  - `x`: `string` or `list`.
  - `index1`: `int`.
  - `index2`: `int`.

- `(put x item)` or `(put x item index)`
  - Returns a `list` or `string`, in which `item` was inserted into `x` at `index`.
  - If `index` not supplied, `item` is assumed to be put at the end of `x`.
  - `x`: `string` or `list`.
  - `index`: `int`.

- `(length x)`
  - Returns the length of `x`
  - `x`: `string` or `list`.

## Sample Programs
### Hello World
```
(print "hello world")
```

### Fizzbuzz
```
(loop 100 {i -> 
  (if (is (% i 15) 0) {
    (print "fizzbuzz")
  } {
    (if (is (% i 3) 0) {
      (print "fizz")
    } {
      (if (is (% i 5) 0) {
        (print "buzz")
      } {
        (print i)
      })
    })
  })
})
```

### Factorial
```
fact = {n ->
  return (if (is n 0) {
    return 1
  } {
    return (* n (fact (- n 1)))
  })
}

(print (fact 7))
```

### Fibonacci
```
fib = {n ->
  return (if (is n 0) {
    return 0
  } {
    return (if (is n 1) {
      return 1
    } {
      return (+ (fib (- n 1)) (fib (- n 2)))
    })
  })
}

(print (fib 26))
```

### Mean of Three Items
```
mean = {a b c -> return (/ (+ (+ a b) c) 3)}
(print (mean 7 8 8))
```

### Collatz
```
// divides 2 if divisible by 2
// multiplies by 3 and adds 1 if not
f = {n ->
  return (if (is (% n 2) 0) {
    return (/ n 2)
  } {
    return (+ (* n 3) 1)
  })
}

// returns stopping_time of n
stopping_time = {n ->
  return (if (is n 1) {
    return 0
  } {
    return (+ 1 (stopping_time (f n)))
  })
}

// run for first 10000 positive integers
(loop 10000 {n ->
  (print "Stopping time for" (+ n 1) "-" (stopping_time (+ n 1)))
})
```

### Cheating Quine
```
(print (read_file "main.crumb"))
```

### Euclidean Algorithim for Finding the GCD
```
gcd = {a b ->
  return (if (is a 0) {
    return b
  } {
    return (gcd (% b a) a)
  })
}

(loop 20 {x ->
  (loop 20 {y ->
    (print "The gcd of" x "and" y "is" (gcd x y))
  })
})
```

### Multiplication Table
```
// returns a row of the multiplication table as a string
// x is the number of values to generate
// n is the number to multiply by (for example the 2nd row in the multiplication table has n = 2)
createRow = {x n ->
  return (if (is x 1) {
    return (str (* n x))
  } {
    return (join (join (createRow (- x 1) n) " ") (str (* n x)))
  })
}

// print table
(loop 10 {n -> 
  (print (createRow 10 (+ n 1)))
})
```