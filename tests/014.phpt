--TEST--
Checks lua::compile()
--SKIPIF--
<?php if (!extension_loaded("lua")) print "skip"; ?>
--FILE--
<?php
$lua=new lua;
$x=lua::compile("print(\"lumpen\")");
$lua->evaluate($x);
?>
--EXPECTF--
lumpen
