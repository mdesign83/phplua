--TEST--
lua::evaluateFile()
--SKIPIF--
<?php if (!extension_loaded("lua")) print "skip"; ?>
--FILE--
<?php 
$filename = __FILE__.'.tmp';

$code = array(
  'fine'   => 'print "Hello PHP"',
  'broken' => 'dgdr fdrg erb rxdgre tews< df hxfdxgfc gsdgxvrsgrg.ve4w',
);

$l = new lua();

foreach ($code as $n => $c) {
    echo "\nTesting $n\n";
    file_put_contents($filename, $c);
    $l->evaluateFile($filename);
    @unlink($filename);
}
?>
--EXPECTF--

Testing fine
Hello PHP
Testing broken

Warning: lua::evaluatefile(): lua error: %s.tmp:1: '=' expected near 'fdrg' in %s on line %i

