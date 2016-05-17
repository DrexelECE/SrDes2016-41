<?php

$q = explode(",",$_SERVER['QUERY_STRING']);

$set = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0];

foreach($q as $k => $v) {
	$set[$k] = dechex(hexdec($v));
}

header('Content-Type: application/json');

echo json_encode($set);

echo "\r\n";