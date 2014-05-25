<?php

create_graph("humidity-hour.gif", "-1h", "Hourly humidity values");
create_graph("humidity-day.gif", "-1d", "Daily humidity values");
create_graph("humidity-week.gif", "-1w", "Weekly humidity values");
create_graph("humidity-month.gif", "-1m", "Monthly humidity values");
create_graph("humidity-year.gif", "-1y", "Yearly humidity values");

echo "<img src='humidity-hour.gif' alt='Generated RRD image'>";
echo "<img src='humidity-day.gif' alt='Generated RRD image'>";
echo "<img src='humidity-week.gif' alt='Generated RRD image'>";
echo "<img src='humidity-month.gif' alt='Generated RRD image'>";
echo "<img src='humidity-year.gif' alt='Generated RRD image'>";
exit;

function create_graph($output, $start, $title) {
  $options = array(
    "-w 320",
    "-h 200",
    "--slope-mode",
    "--start", $start,
    "--title=$title",
    "--vertical-label=Humidity Values",
    "--lower=0",
    "DEF:s1=sensors.rrd:sensor1:AVERAGE",
    "LINE2:s1#00FF00:Sensor 1 Humidity Values",
  );

  $ret = rrd_graph($output, $options);
  if (! $ret) {
    echo "<b>Graph error: </b>".rrd_error()."\n";
  }
}

?>
