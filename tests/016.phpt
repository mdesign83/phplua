--TEST--
Check calling php functions with table arguments
--SKIPIF--
<?php if (!extension_loaded("lua")) print "skip"; ?>
--FILE--
<?php
function foo($a,$b) {
  var_dump($a);
}

$lua = new lua();
$lua->expose_function("php_foo", "foo");

$luacode =<<< EOF
php_foo({"qwerty",{"lala",{"test"}}},4,{"test"});
x={"asdf",{"xxx","yyy",{"aaa"}}}
EOF;

$lua->evaluate($luacode);
var_dump($lua->x);
?>
--EXPECTF--
array(2) {
  [1]=>
  string(6) "qwerty"
  [2]=>
  array(2) {
    [1]=>
    string(4) "lala"
    [2]=>
    array(1) {
      [1]=>
      string(4) "test"
    }
  }
}
array(2) {
  [1]=>
  string(4) "asdf"
  [2]=>
  array(3) {
    [1]=>
    string(3) "xxx"
    [2]=>
    string(3) "yyy"
    [3]=>
    array(1) {
      [1]=>
      string(3) "aaa"
    }
  }
}
