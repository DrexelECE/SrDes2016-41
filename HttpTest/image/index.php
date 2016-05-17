<?php

header("Content-Type: image/jpeg");
header("Content-Length: " . (string)filesize('image.jpg'));


echo file_get_contents('image.jpg');

echo "\r\n"; //\r\n";