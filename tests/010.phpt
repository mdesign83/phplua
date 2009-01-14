--TEST--
Calling LUA functions from within PHP
--SKIPIF--
<?php if (!extension_loaded("lua")) print "skip"; ?>
--FILE--
<?php
class hund
{
  public static function test($x,$y)
  {
    var_dump($x,$y);
  }
}

call_user_func_array(array("hund","test"),array(1,2));

$lua=new lua;
$lua->evaluatefile(dirname(__FILE__)."/events.lua");
$lua->peter2="hunde";
$lua->peter=array("asdf"=>"b");
var_dump($lua->call_function("hund",array()));
var_dump($lua->call_function("hund",array()));
var_dump($lua->call_function("hund",array()));
var_dump($lua->call_table(array("events","OnLoad"),array()));
$lua->call_table(array("events","OnLoad"),array());
$lua->call_table(array("events","OnLoad"),array());

var_dump($lua->call_table_self(array("events","OnTest"),array()));
$lua->call_table_self(array("events","OnTest"),array());
$lua->call_table_self(array("events","OnTest"),array());
$lua->call_table_self(array("events","OnFinish"),array(1,2,3));
?>
--EXPECTF--
int(1)
int(2)
ONTEST
1ONTEST
1ONTEST
1ONTEST
1ONTEST
1ONTEST
1ONTEST
1ONTEST
1ONTEST
1hund
array(2) {
  [0]=>
  float(1)
  [1]=>
  float(2)
}
hund
array(2) {
  [0]=>
  float(1)
  [1]=>
  float(2)
}
hund
array(2) {
  [0]=>
  float(1)
  [1]=>
  float(2)
}
ONLOAD
array(2) {
  [0]=>
  array(3) {
    [1]=>
    float(13)
    [2]=>
    float(13)
    [3]=>
    array(1) {
      [1]=>
      string(5) "peter"
    }
  }
  [1]=>
  string(2) "ha"
}
ONLOAD
ONLOAD
ONTEST
1array(0) {
}
ONTEST
1ONTEST
1ONFINISH
