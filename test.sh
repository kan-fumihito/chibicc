#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./chibicc "$input" > tmp.s
  gcc -static -o tmp tmp.s func.c -g
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}
assert 1 "main(){return 1;}"

assert 3 "
main(){
  a=1;
  b=2;

  return a+b;
}
"

assert 10 "
func(){
  return 10;
}
main(){
  return func();
}
"

assert 5 "
func(){
  return 5;
}
main(){
  a=0;
  b=func();
  return b;
}
"

assert 2 "
func(n){
  return 2*n;
}
main(){
  a=1;
  b=func(a);
  return b;
}
"

echo OK
