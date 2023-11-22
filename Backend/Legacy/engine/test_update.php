<?php
$files = fopen($_SERVER['DOCUMENT_ROOT'].'/engine/update_list.txt', 'r');
$data = json_decode(fread($files, filesize($_SERVER['DOCUMENT_ROOT'].'/engine/update_list.txt')));
$error_counter = 0;
fclose($files);
print_r($data);
//exec('sudo rm -f /mnt/mmcblk0p3/*', $output, $result);
//
//for($i=0; $i<count($data->list); $i++){
//    $file = $data->list[$i]->url;
//    $hash = $data->list[$i]->hash;
//    $filename = $data->list[$i]->filename;
//    echo 'start ' . $filename;
//    exec('sudo wget -O ' . '/mnt/mmcblk0p3' . substr($file, strrpos($file, '/')) . ' ' . $file, $output, $result);
//    $c_hash = md5_file('/mnt/mmcblk0p3/' . substr($file, strrpos($file, '/')));
//    if($c_hash!==$hash){
//        exec('sudo rm -f /mnt/mmcblk0p3/'.substr($file, strrpos($file, '/')));
//        if($error_counter>3){
//            $error_counter++;
//            $i--;
//        }
//    }
//    sleep(1);
//    echo 'end ' . $filename;
//}
//
//exec("sudo reboot", $result, $result);