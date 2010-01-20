--TEST--
verifies that functions are objectbound and killing a lua instance doesn't influence a second
--SKIPIF--
<?php if (!extension_loaded("lua")) print "skip"; ?>
--FILE--
<?php
class x
{
  function y()
  {
    print "hehe";
  }
}
$x=new x;
$lua1=new lua;
$lua1->expose_function("test",array($x,"y"));
unset($lua1);
$lua2=new lua;
$lua2->expose_function("test",array($x,"y"));
$lua2->evaluate("test()");
?>
--EXPECTF--
hehe
