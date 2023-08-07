# Crumb Programming Language
## Compile and Run
```bash
gcc src/* -o main && ./main code.crumb
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
- `print`
- `is`
- `apply`
- `loop`
- `if`
- `%`
- `!`
- `+`
- `-`
- `*`
- `/`
- `read_file`
- `write_file`

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