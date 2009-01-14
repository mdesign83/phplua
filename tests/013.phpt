--TEST--
Tests ini setting load_standard_libs
--SKIPIF--
<?php if (!extension_loaded("lua")) print "skip"; ?>
--FILE--
<?php
ini_set("lua.load_standard_libs",0);
$lua=new lua;
var_dump($lua->math);
ini_set("lua.load_standard_libs",1);
$lua=new lua;
var_dump($lua->math);
?>
--EXPECTF--
NULL

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7

Warning: main(): invalid type `6' passed from lua. in /home/mop/projects/phplua/tests/013.php on line 7
array(31) {
  ["log"]=>
  NULL
  ["max"]=>
  NULL
  ["acos"]=>
  NULL
  ["huge"]=>
  float(INF)
  ["ldexp"]=>
  NULL
  ["pi"]=>
  float(3.14159265359)
  ["cos"]=>
  NULL
  ["tanh"]=>
  NULL
  ["pow"]=>
  NULL
  ["deg"]=>
  NULL
  ["tan"]=>
  NULL
  ["cosh"]=>
  NULL
  ["sinh"]=>
  NULL
  ["random"]=>
  NULL
  ["randomseed"]=>
  NULL
  ["frexp"]=>
  NULL
  ["ceil"]=>
  NULL
  ["floor"]=>
  NULL
  ["rad"]=>
  NULL
  ["abs"]=>
  NULL
  ["sqrt"]=>
  NULL
  ["modf"]=>
  NULL
  ["asin"]=>
  NULL
  ["min"]=>
  NULL
  ["mod"]=>
  NULL
  ["fmod"]=>
  NULL
  ["log10"]=>
  NULL
  ["atan2"]=>
  NULL
  ["exp"]=>
  NULL
  ["sin"]=>
  NULL
  ["atan"]=>
  NULL
}
