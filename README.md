# <img src="./icon.svg" alt="crumb icon" height="20"/> Crumb
Crumb is a high level, functional, interpreted, dynmaically typed, general-purpose programming language, with a terse syntax, and a verbose standard library.

It features:
- Strictly no side effects to help you write functional code.
- The ability to localize the effects of imported crumb files.
- Dyanmic typing and garbage collection.
- 0 keywords, everything is a function.

```
table = (map (range 10) {_ y ->
  <- (map (range 10) {item x ->
    <- (multiply (add x 1) (add y 1))
  })
})
```
From `examples/mult-table.crumb`

## Development
### Compile
```bash 
gcc src/* -Wall -lm -o crumb
```

### run
```bash
./crumb path_to_file
```

### Debug with Valgrind
```bash
gcc src/* -g -Wall -lm -o crumb && valgrind --leak-check=full -s ./crumb _test.crumb
```

## Syntax
### EBNF
```ebnf
program = start, statement, end;
statement = {return | assignment | value};
return = "<-", value;
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
"<-"
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
  - A list command line arguments, like argv in C.

- `(print arg1 arg2 arg3 ...)` ✅
  - Prints all arguments to stdout, returns nothing.

- `(input)` ✅
  - Gets a line of input from stdin.

- `(rows)` ✅
  - Returns the number of rows in the terminal.

- `(columns)` ✅
  - Returns the number of columns in the terminal.

- `(read_file path)` ✅
  - Returns the contents of the file designated by `path`, in a string
  - `path`: `string`

- `(write_file path contents)` ✅
  - Writes the string `contents` into the file designated by `path`, returns nothing.
  - `path`: `string`
  - `contents`: `string`

- `(event)` ✅
  - Returns the ANSI string corresponding with the current event. This may block for up to 0.1 seconds.

- `(use path1 path2 path3 ... fn)` ✅
  - Crumb's code splitting method. Runs code in file paths, in order, on a new scope. Then uses said scope to apply `fn`.
  - `path1`, `path2`, `path3`, ...: `string`
  - `fn`: `function`
  
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
  - `a`: `integer` or `float`
  - `b`: `integer` or `float`
  
- `(power a b)` ✅
  - Returns `a` to the power of `b`.
  - `a`: `integer` or `float`
  - `b`: `integer` or `float`

- `(random)` ✅
  - Returns a random number from 0 to 1.

### Control
- `(loop count fn)` ✅
  - Applys `fn`, `count` times. If `fn` returns, the loop breaks, and `loop` returns whatever `fn` returned, else repeats until loop is completed.
  - `count`: `integer`, which is greater than or equal to `0`
  - `fn`: `function`, which is in the form `{n -> ...}`, where n is the current loop index (starting at `0`).

- `(until stop fn initial_state)` or `(until stop fn)` ✅
  - Applys `fn`, and repeats until `fn` returns `stop`. `until` returns whatever `fn` returned, before `stop`.
  - The return value of every past itteration is passed on to the next. The initial itteration uses `initial_state` if supplied, or returns `void` if not.
  - `fn`: `function`, which is in the form `{state n -> ...}`, where n is the current loop index (starting at `0`), and `state` is the current state.

- `(if condition fn1)` or `(if condition fn1 fn2)` ✅
  - If `condition` is `1`, applys `fn1`. (like the "then" part in an if statement).
  - If `condition` is `0`, and `fn2` was supplied, apply `fn2`. (the "else" part in an if statement).
  - Returns whatever `fn1` or `fn2` return
  - `condition`: `integer`, which is `1` or `0`
  - `fn1`: `function`, which takes no arguments
  - `fn2`: `function`, which takes no arguments

- `(wait time)` ✅
  - Blocks execution for `time` amount of seconds.
  - `time`: `integer` or `float`.

### Types
- `void` ✅
  - A value of type `void`

- `(integer a)` ✅
  - Returns `a` as an `integer`.
  - `a`: `string`, `float`, or `integer`.

- `(string a)` ✅
  - Returns `a` as a `string`.
  - `a`: `string`, `float`, or `integer`.

- `(float a)` ✅
  - Returns `a` as a `float`.
  - `a`: `string`, `float`, or `integer`.

- `(type a)` ✅
  - Returns the type of `a` as a `string`.

### List and String
- `(list arg1 arg2 arg3 ...)` ✅
  - Returns a `list`, with the arguments as it's contents.

- `(length x)` ✅
  - Returns the length of `x`
  - `x`: `string` or `list`.

- `(join arg1 arg2 arg3 ...)` ✅
  - Returns all args joined together.
  - All args must be of the same type.
  - `arg1`, `arg2`, `arg3`, ...: `string` or `list`.

- `(get x index1)` or `(get x index1 index2)` ✅
  - Returns the item in `x` at `index1`. If x is a `string`, this is a single char.
  - If `index2` is supplied, returns a subarray or substring from `index1` to `index2`, not including `index2`.
  - `x`: `string` or `list`.
  - `index1`: `int`.
  - `index2`: `int`.

- `(insert x item)` or `(insert x item index)` ✅
  - Returns a `list` or `string`, in which `item` was inserted into `x` at `index`. Does not overwrite any data.
  - If `index` not supplied, `item` is assumed to be put at the end of `x`.
  - `x`: `string` or `list`.
  - `item`: `string` if `x` is `string`, else any
  - `index`: `int`.
  
- `(set x item index)` ✅
  - Returns a `list` or `string`, in which the item located at `index` in `x`, was replaced by `item`.Overwrites data.
  - `x`: `string` or `list`.
  - `item`: `string` if `x` is `string`, else any
  - `index`: `int`.

- `(delete x index1)` or `(delete x index1 index2)` ✅
  - Returns a `string` or `list`, where `index1` was removed from `x`.
  - If `index2` is supplied, all items from `index1` to `index2` are removed, not including `index2`.
  - `x`: `string` or `list`.
  - `index1`: `int`.
  - `index2`: `int`.

- `(map arr fn)` ✅
  - Returns a list created by calling `fn` on every item of `arr`, and using the values returned by `fn` to populate the returned array.
  - `arr`: `list`
  - `fn`: `function`, which is in the form `{item i -> ...}`, where `item` is the current item, and `i` is the current index.

- `(reduce arr fn initial_acc)` or `(reduce arr fn)` ✅
  - Returns a value, computed via running `fn` on every item in `arr`. With every iteration, the last return from `fn` is passed to the next application of `fn`. The final returned value from `fn` is the value returned from `reduce`.
  - `arr`: `list`.
  - `fn`: `function`, which is in the form `{item acc item i -> ...}`, where `item` is the current item, `acc` is the accumulator (the result of `fn` from the last item), and `i` is the current index. `acc` is `initial_acc` if supplied, or `void` if not.

- `(range n)` ✅
  - Returns a list with the integers from `0` to `n`, not including `n`.
  - `n`: `integer`, which is greater than or equal to 0.

- `(find x item)` ✅
  - Returns the index of `item` in `x`. Returns `void` if not found.
  - `x`: `string` or `list`
  - `item`: `string` if `x` is `string`, else any