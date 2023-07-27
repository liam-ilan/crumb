# Crumb Programming Language
## Compile and Run
```bash
gcc main.c -o main && ./main code.txt
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
function = "{", [{identifier, ","}, identifier, "->"], statement, "}";
```

### Tokens
```
"="
"("
")"
"{"
"}"
","
"->"
"return"
identifier
int
float
string
start
end
```

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
! = {n ->
  res = 0

  (if (is n 0) {
    res = 1
  } {
    res = (* n (! (- n 1)))
  })

  return res
}

(print (! (input)))
```