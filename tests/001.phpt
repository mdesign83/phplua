--TEST--
Basic lua check
--SKIPIF--
<?php if (!extension_loaded("lua")) print "skip"; ?>
--FILE--
<?php 
$l = new lua();
$l->b = 12;
$l->evaluate("print(b)");
$l->evaluate('print("\n")');
$l->evaluate("print(math.sin(b))");
$l->evaluate("invalid code");
--EXPECTF--
12
-0.53657291800043
Warning: lua::evaluate(): lua error: %s in %s on line %i
