<?php
class testung
{
  public function hass($string)
  {
    print "HAHAHAHA";
  }
}
try {
  $l = new lua();
  $l->print("hmm");
  $l->evaluatefile($_SERVER["argv"][1]);
    
}

catch (Exception $e) {

    print_r($e);
}

?>
