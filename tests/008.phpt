--TEST--
Array handling test lua->php
--SKIPIF--
<?php if (!extension_loaded("lua")) print "skip"; ?>
--FILE--
<?php
$l = new lua();
$l->evaluate("
arr={1,test=\"asdf\",{4.5,false,nil}}
arr[100]={4,5,6}
");
var_dump($l->arr);
?>
--EXPECTF--
array(4) {
  [1]=>
  float(1)
  [2]=>
  array(2) {
    [1]=>
    float(4.5)
    [2]=>
    bool(false)
  }
  ["test"]=>
  string(4) "asdf"
  [100]=>
  array(3) {
    [1]=>
    float(4)
    [2]=>
    float(5)
    [3]=>
    float(6)
  }
}
