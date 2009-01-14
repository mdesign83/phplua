--TEST--
Call lua functions
--SKIPIF--
<?php if (!extension_loaded("lua")) print "skip"; ?>
--FILE--
<?php 
$l = new lua();
$l->evaluate(<<<CODE
function phptest_no_param()
    print("No parameters!\\n")
end

function phptest_1_param(param)
    print("Parameter: `", param, "'!\\n")
end

function phptest_2_params(param1, param2)
    print("Parameter 1: `", param1, "', Parameter 2: `", param2, "'!\\n")
end

function multiret()
    return "a", "b", "c"
end
CODE
);

$l->print("Hello world!\n");
$l->phptest_no_param();

$l->phptest_1_param();
$l->phptest_1_param("foobar");

$l->phptest_2_params();
$l->phptest_2_params("foo");
$l->phptest_2_params("foo", "bar");

echo "\n";

var_dump($l->{"math.sin"}(3/2*pi()));
var_dump($l->{"math.cos"}(pi()));

echo "\n";

$l->notexisting(2423);
$l->{"not.existing.function"}(432, 342);

echo "\n";

var_dump($l->type("fobar"));
var_dump($l->multiret());

echo "\n";

var_dump($l->__call("multiret", array(), 0));
var_dump($l->__call("multiret", array(), 1));
var_dump($l->__call("multiret", array(), 2));
var_dump($l->__call("multiret", array(), LUA::MULTRET));

$l->__call("print");
$l->__call("print", "foo");
?>
--EXPECTF--
Hello world!
No parameters!
Parameter: `(null)'!
Parameter: `foobar'!
Parameter 1: `(null)', Parameter 2: `(null)'!
Parameter 1: `foo', Parameter 2: `(null)'!
Parameter 1: `foo', Parameter 2: `bar'!

array(1) {
  [0]=>
  float(-1)
}
array(1) {
  [0]=>
  float(-1)
}


Warning: lua::__call(): error running lua function `notexisting': attempt to call a nil value in %s on line %i

Warning: lua::__call(): error looking up lua function `not.existing.function': [string "return not.existing.function"]:1: unexpected symbol near '.' in %s on line %i

array(1) {
  [0]=>
  string(6) "string"
}
array(3) {
  [0]=>
  string(1) "a"
  [1]=>
  string(1) "b"
  [2]=>
  string(1) "c"
}

NULL
array(1) {
  [0]=>
  string(1) "a"
}
array(2) {
  [0]=>
  string(1) "a"
  [1]=>
  string(1) "b"
}
array(3) {
  [0]=>
  string(1) "a"
  [1]=>
  string(1) "b"
  [2]=>
  string(1) "c"
}

Warning: lua::__call() expects at least 2 parameters, 1 given in %s on line %i

Warning: lua::__call() expects parameter 2 to be array, string given in %s on line %i
