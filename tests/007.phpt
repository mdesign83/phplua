--TEST--
Array handling test php->lua
--SKIPIF--
<?php if (!extension_loaded("lua")) print "skip"; ?>
--FILE--
<?php
$arr = array(3,4,5,array("test1","test2","test3",array(1=>1,2=>2.5,3=>false,4=>NULL)));
$l   = new lua();
$l->arr=$arr;
$l->evaluate(<<<CODE
function dump_table(t,lvl)
  print('==============\\n')
  for i,v in ipairs(t)
  do
    print(string.rep('  ',lvl),i,'=>')
    if type(v)=='table' then
      print('SUBTABLE\\n')
      dump_table(v,lvl+1)
    else
      print(v,'(',type(v),')\\n')
    end
  end
  print('==============\\n')
end

dump_table(arr,0)
CODE
);
?>
--EXPECTF--
Warning: main(): Trying to push array index 0 to lua which is unsupported in lua. Element has been discarded in %s

Warning: main(): Trying to push array index 0 to lua which is unsupported in lua. Element has been discarded in %s
==============
1=>4(number)
2=>5(number)
3=>SUBTABLE
==============
  1=>test2(string)
  2=>test3(string)
  3=>SUBTABLE
==============
    1=>1(number)
    2=>2.5(number)
    3=>(null)(boolean)
==============
==============
==============
