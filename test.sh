#!/bin/bash
assert() {
  input="$1"
  expected="$2"

  ./cmz "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 0
assert 42 42
assert "5+20-9" 16
assert " 12 + 34 - 5 " 41
assert "5+6*7" 47
assert "5 * (9 - 6)" 15
assert "( 3 + 5 ) / 2" 4
assert "10* 10 -10  +  (10 +10 ) /10" 92
assert "((-0)--(31)++100-+50+(-(+(-(((1))))))*-0)/+(+1++1)*(-4--6)" 80

echo OK
