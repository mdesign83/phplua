--TEST--
Array handling tests PHP=>LUA=>PHP
--SKIPIF--
<?php if (!extension_loaded("lua")) print "skip"; ?>
--FILE--
<?php
$pautz=array("hund"=>"ja","ne",array("test"),array("mann",array("ja","aber ne")),"rumms");
var_dump($pautz);
$lua=new lua;
$lua->pautz=$pautz;

$lua->katze="hundemann";
$lua->evaluate("
function dump_table(t,lvl)
  print('==============\\n')
  for i,v in ipairs(t)
  do
    print(string.rep('  ',lvl),i,'=>')
    if type(v)=='table' then
      print('isse table :S\\n')
      dump_table(v,lvl+1)
    else
      print(v,'\\n')
    end
    
  end
  print('==============\\n')
end

hund={}
def={1,2,3,5,{4.3,5},hass={peter=1,3,\"hund\"},20,30}
hass=2

print(\"Pautz\",pautz,\"\\n\")
print(\"Hund\",hund,\"\\n\")
print(\"Def\",def,\"\\n\")

pautz[100]='asdf'
dump_table(def,0)
dump_table(pautz,0)
print('HUNDEMANN IST => ',katze,'\\n')
");
var_dump($lua->def);
var_dump($lua->hund);
var_dump($lua->hass);
var_dump($lua->pautz);
?>
--EXPECTF--
array(5) {
  ["hund"]=>
  string(2) "ja"
  [0]=>
  string(2) "ne"
  [1]=>
  array(1) {
    [0]=>
    string(4) "test"
  }
  [2]=>
  array(2) {
    [0]=>
    string(4) "mann"
    [1]=>
    array(2) {
      [0]=>
      string(2) "ja"
      [1]=>
      string(7) "aber ne"
    }
  }
  [3]=>
  string(5) "rumms"
}

Warning: main(): Trying to push array index 0 to lua which is unsupported in lua. Element has been discarded in /home/mop/projekte/phplua/tests/011.php on line 5

Warning: main(): Trying to push array index 0 to lua which is unsupported in lua. Element has been discarded in /home/mop/projekte/phplua/tests/011.php on line 5

Warning: main(): Trying to push array index 0 to lua which is unsupported in lua. Element has been discarded in /home/mop/projekte/phplua/tests/011.php on line 5

Warning: main(): Trying to push array index 0 to lua which is unsupported in lua. Element has been discarded in /home/mop/projekte/phplua/tests/011.php on line 5
Pautz(null)
Hund(null)
Def(null)
==============
1=>1
2=>2
3=>3
4=>5
5=>isse table :S
==============
  1=>4.3
  2=>5
==============
6=>20
7=>30
==============
==============
1=>isse table :S
==============
==============
2=>isse table :S
==============
  1=>isse table :S
==============
    1=>aber ne
==============
==============
3=>rumms
==============
HUNDEMANN IST => hundemann
array(8) {
  [1]=>
  float(1)
  [2]=>
  float(2)
  [3]=>
  float(3)
  [4]=>
  float(5)
  [5]=>
  array(2) {
    [1]=>
    float(4.3)
    [2]=>
    float(5)
  }
  [6]=>
  float(20)
  [7]=>
  float(30)
  ["hass"]=>
  array(3) {
    [1]=>
    float(3)
    [2]=>
    string(4) "hund"
    ["peter"]=>
    float(1)
  }
}
array(0) {
}
float(2)
array(5) {
  [1]=>
  array(0) {
  }
  [2]=>
  array(1) {
    [1]=>
    array(1) {
      [1]=>
      string(7) "aber ne"
    }
  }
  [3]=>
  string(5) "rumms"
  [100]=>
  string(4) "asdf"
  ["hund"]=>
  string(2) "ja"
}
