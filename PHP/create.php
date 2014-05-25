<?php

$options = array(
 "--step", "20",            
 "DS:sensor1:GAUGE:40:0:U",
 "RRA:AVERAGE:0.5:1:129600",
 "RRA:AVERAGE:0.5:15:1051200",
 );

$ret = rrd_create("sensors.rrd", $options);
if (! $ret) {
 echo "<b>Creation error: </b>".rrd_error()."\n";
}

?>
