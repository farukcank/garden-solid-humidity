<?php
    $ret = rrd_update("sensors.rrd", array(htmlspecialchars($_GET["value"])));
?>
