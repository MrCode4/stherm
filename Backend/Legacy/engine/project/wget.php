<?php
$file = 'http://test.hvac.z-soft.am/update/02r038f/part3';
exec('sudo wget -O ' . '/mnt/mmcblk1p3' . substr($file, strrpos($file, '/')) . ' ' . $file, $output, $result);
$file = 'http://test.hvac.z-soft.am/update/02r038f/uboot';
exec('sudo wget -O ' . '/mnt/mmcblk1p3' . substr($file, strrpos($file, '/')) . ' ' . $file, $output, $result);
$file = 'http://test.hvac.z-soft.am/update/02r038f/part1';
exec('sudo wget -O ' . '/mnt/mmcblk1p3' . substr($file, strrpos($file, '/')) . ' ' . $file, $output, $result);
$file = 'http://test.hvac.z-soft.am/update/02r038f/part2';
exec('sudo wget -O ' . '/mnt/mmcblk1p3' . substr($file, strrpos($file, '/')) . ' ' . $file, $output, $result);