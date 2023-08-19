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

- `(random)` ✅
  - Returns a random number from 0 to 1.

### Control
- `(loop count fn)` ✅
  - Applys `fn`, `count` times. If `fn` returns, the loop breaks, and `loop` returns whatever `fn` returned, else repeats until loop is completed.
  - `count`: `integer`, which is greater than or equal to `0`
  - `fn`: `function`, which is in the form `{n -> ...}`, where n is the current loop index (starting at `0`).

- `(until fn)` ✅
  - Applys `fn`, and repeats until `fn` returns. `until` returns whatever `fn` returned.
  - `fn`: `function`, which is in the form `{n -> ...}`, where n is the current loop index (starting at `0`).

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

- `(event)` ✅
  - Returns the ANSI string corresponding with the current event. This may block for up to 0.1 seconds.

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
  - `index`: `int`.
  
- `(set x item index)` ✅
  - Returns a `list` or `string`, in which the item located at `index` in `x`, was replaced by `item`.Overwrites data.
  - `x`: `string` or `list`.
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

- `(reduce arr fn)` ✅
  - Returns a value, computed via running `fn` on every item in `arr`
  - `arr`: `list`.
  - `fn`: `function`, which is in the form `{item acc item i -> ...}`, where `item` is the current item, `acc` is the accumulator (the result of `fn` from the last item), and `i` is the current index. `acc` is assumed to be the first item for the first iteration.

- `(range n)` ✅
  - Returns a list with the integers from `0` to `n`, not including `n`.
  - `n`: `integer`, which is greater than or equal to 0.