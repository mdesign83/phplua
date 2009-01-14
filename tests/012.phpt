--TEST--
Calling PHP class methods from LUA
--SKIPIF--
<?php if (!extension_loaded("lua")) print "skip"; ?>
--FILE--
<?php
class test
{
  protected $x=0;

  public function test()
  {
    print ($this->x++).": ".str_repeat("test",10)."\n";
  }
}

function two()
{
  print str_repeat("two",10)."\n";

}

function one($x,$y,$z)
{
  var_dump($x,$y,$z);
  print str_repeat("one",10)."\n";
  return "test";
}
$test=new test;

$lua=new lua;
$lua->expose_function("php_one","one");
$lua->expose_function("php_two","two");
$lua->expose_function("php_three",array($test,"test"));
$lua->evaluatefile(dirname(__FILE__)."/012.lua");
var_dump($lua->peter);
?>
--EXPECTF--
0: testtesttesttesttesttesttesttesttesttest
string(7) "string1"
float(5)
string(7) "string2"
oneoneoneoneoneoneoneoneoneone
test
string(4) "hmmm"
float(5)
string(4) "test"
oneoneoneoneoneoneoneoneoneone
test
1: testtesttesttesttesttesttesttesttesttest
twotwotwotwotwotwotwotwotwotwo
2: testtesttesttesttesttesttesttesttesttest
bool(true)
