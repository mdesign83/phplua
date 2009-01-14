<?php

try {
  $l = new lua();
  $l->evaluatefile("script.lua");
    
}

catch (Exception $e) {

    print_r($e);
}
?>
