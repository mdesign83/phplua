--TEST--
Calling a PHP function from within lua
--SKIPIF--
<?php if (!extension_loaded("lua")) print "skip"; ?>
--FILE--
<?php
function test($x,$y)
{
  return $x+$y;
}

$lua=new lua;
$lua->expose_function("php_test","test");
$lua->expose_function("php_test2","test");
$lua->evaluate('print(php_test(5,4)..php_test(5,13))');
?>
--EXPECTF--
918
