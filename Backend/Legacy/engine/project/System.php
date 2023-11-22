<?php

namespace project;

use DateTime;
use Exception;
use core\db\Z_PgSQL;
use project\Sync;
use ZipArchive;

require_once "/usr/share/apache2/default-site/htdocs/engine/project/Sync.php";

/**
 * System work functional in main screen and sets wirings
 * @package project
 */
class System
{
    private $conn;
    private $sync;
    private const WRAP = "/usr/share/apache2/default-site/htdocs/engine/";
    private const WEB_URL = "http://test.hvac.z-soft.am/engine/index.php";
    private const WEB_MAIN = "http://test.hvac.z-soft.am/update/";
    private const LOG_ALERT_INTERVAL = 7;                   // days

    /**
     * @throws Exception
     */
    public function __construct()
    {
        $this->conn = Z_PgSQL::connection();
        $this->sync = new Sync();
    }

    /**
     * Get ventilator's value
     * @return int
     */
    public function getVentilator()
    {
        return (int)$this->conn->getItem("SELECT ventilator FROM device_config");
    }

    /**
     *
     * @return string
     */
    public function getTechnicEditQR() {
        return $this->conn->getItem("SELECT technical_edit_link FROM device_config");
    }

    /**
     * Get technic's url and serial number
     * @return string                     // url/serial_number
     * @throws Exception
     */
    public function getQR()
    {
        $access = $this->conn->getRow("SELECT technical_access_link, serial_number, uid FROM device_config");
        $serial_number = $access['serial_number'];
        if ($access['serial_number'] === '' || is_null($access['serial_number'])) {
            $data = json_encode(
                [
                    'request' =>
                        [
                            'class' => 'sync',
                            'method' => 'getSN',
                            'params' => [$access['uid']]
                        ],
                    'user' =>
                        [
                            'lang_id' => 0,
                            'user_id' => 0,
                            'type_id' => 0,
                            'host_id' => 0,
                            'region_id' => 0,
                            'token' => ''
                        ]
                ]);
            $curl = curl_init();
            curl_setopt_array($curl, array(
                CURLOPT_URL => self::WEB_URL,
                CURLOPT_FAILONERROR => true,
                CURLOPT_RETURNTRANSFER => true,
                CURLOPT_ENCODING => '',
                CURLOPT_MAXREDIRS => 10,
                CURLOPT_TIMEOUT => 0,
                CURLOPT_FOLLOWLOCATION => true,
                CURLOPT_HTTP_VERSION => CURL_HTTP_VERSION_1_1,
                CURLOPT_CUSTOMREQUEST => 'POST',
                CURLOPT_POSTFIELDS => $data,
            ));
            $response = curl_exec($curl);
            if (curl_errno($curl)) {
                $error_msg = curl_error($curl);
            }
            curl_close($curl);
            if (!isset($error_msg)) {
                if ($response) {
                    $result = json_decode($response)->result->result;
                    $serial_number = $result[0];
                    $this->conn->setQuery("UPDATE device_config SET serial_number = '{$serial_number}'");
                }
            }
        }
        if ($serial_number === '') {
            return '';
        }
        return $access['technical_access_link'] . $serial_number;
    }

    /**
     * Get QR's answer
     * @return bool
     */
    public function getQRAnswer()
    {
        return true;
    }

    /**
     * Get static info
     * @return array                        // measure: 0 - celsius, 1 - fahrenheit
     * @throws Exception
     */
    public function getMainStatic()
    {
        return $this->conn->getRow("SELECT logo AS img,
                                                 (SELECT measure_id FROM settings limit 1)
                                          FROM device_config");
    }

    public function getUpdate(string $serial_number)
    {
        static $count = 0;
        $soft_v = parse_ini_file($_SERVER['DOCUMENT_ROOT'] . '/configuration/version.ini')['SOFTWARE_VERSION'];
        $data = json_encode(
            [
                'request' =>
                    [
                        'class' => 'sync',
                        'method' => 'getSystemUpdate',
                        'params' => [$serial_number, $soft_v]
                    ],
                'user' =>
                    [
                        'lang_id' => 0,
                        'user_id' => 0,
                        'type_id' => 0,
                        'host_id' => 0,
                        'region_id' => 0,
                        'token' => ''
                    ]
            ]);
        $curl = curl_init();
        curl_setopt_array($curl, array(
            CURLOPT_URL => self::WEB_URL,
            CURLOPT_FAILONERROR => true,
            CURLOPT_RETURNTRANSFER => true,
            CURLOPT_ENCODING => '',
            CURLOPT_MAXREDIRS => 10,
            CURLOPT_TIMEOUT => 0,
            CURLOPT_FOLLOWLOCATION => true,
            CURLOPT_HTTP_VERSION => CURL_HTTP_VERSION_1_1,
            CURLOPT_CUSTOMREQUEST => 'POST',
            CURLOPT_POSTFIELDS => $data,
        ));
        $response = curl_exec($curl);
        if (curl_errno($curl)) {
            $error_msg = curl_error($curl);
        }
        curl_close($curl);
        if (!isset($error_msg)) {
            if ($response) {
                $result = json_decode($response)->result->result;
                if ($result !== []) {
                    if ($result->require === 'r') {
                        if ($result->type === 'f') {              // full_update
                            exec('sudo rm -f /mnt/mmcblk1p3/*', $output, $result1);
                            //exec('sudo rm -f /mnt/mmcblk0p3/*', $output, $result1);
                            $connected_wifi = $this->conn->getRow("SELECT essid,password,unique_name FROM connected_wifi WHERE is_connected = true LIMIT 1");
                            if (!empty($connected_wifi)) {
                                chmod('/mnt/mmcblk1p3', 0777);
                                //chmod('/mnt/mmcblk0p3', 0777);
                                $content = "[wi-fi]\n";
                                $path = "/mnt/mmcblk1p3/wifi.ini";
                                //$path = "/mnt/mmcblk0p3/wifi.ini";
                                $assoc_arr = [
                                    'ESSID' => $connected_wifi['essid'],
                                    'UNIQUE_NAME' => $connected_wifi['unique_name'],
                                    'PASSWORD' => $connected_wifi['password']];
                                foreach ($assoc_arr as $key => $elem) {
                                    $content .= $key . " = " . $elem . "\n";
                                }
                                $handle = fopen($path, 'w+');
                                fwrite($handle, $content);
                                fclose($handle);
                            }
                            $path = "/usr/share/apache2/default-site/htdocs/configuration/device_settings.ini";
                            exec('sudo chmod 777 ' . $path, $result1, $retval);
                            $content = "UPDATED = 1";
                            $handle = fopen($path, 'w+');
                            fwrite($handle, $content);
                            fclose($handle);

                            $data = json_decode(json_encode($result));
                            $error_counter = 0;

                            $content = "";
                            $content .= "list = ". json_encode($data->list). "\n";
                            $download_log = "/usr/share/apache2/default-site/htdocs/configuration/update_data.ini";
                            $error = fopen($download_log, 'w+');
                            fwrite($error, $content);
                            fclose($error);
                            sleep(1);
                            $arguments = '';
                            for ($i = 0; $i < count($data->list); $i++) {
                                $filename = $data->list[$i]->filename;
                                if($filename === 'checksum.txt'){
                                    $last = $data->list[$i]->url;
                                } else {
                                    $arguments .= $data->list[$i]->url . ' ';
                                }
//                                $hash = $data->list[$i]->hash;

//                                exec('sudo wget -O ' . '/mnt/mmcblk1p3' . substr($file, strrpos($file, '/')) . ' ' . $file, $output, $result);
//                                //exec('sudo wget -O ' . '/mnt/mmcblk0p3' . substr($file, strrpos($file, '/')) . ' ' . $file, $output, $result);
//                                $c_hash = md5_file('/mnt/mmcblk1p3/' . substr($file, strrpos($file, '/')));
//                                //$c_hash = md5_file('/mnt/mmcblk0p3/' . substr($file, strrpos($file, '/')));
//
//                                $content = "";
//                                $content .= $data->list[$i]->url. "\n";
//                                $content .= $hash . "\n";
//                                $content .= $c_hash. "\n";
//                                $download_log = "/usr/share/apache2/default-site/htdocs/configuration/update_data.ini";
//                                $error = fopen($download_log, 'w+');
//                                fwrite($error, $content);
//                                fclose($error);
//
//                                if ($c_hash !== $hash) {
//                                    exec('sudo rm -f /mnt/mmcblk1p3/' . substr($file, strrpos($file, '/')));
//                                    //exec('sudo rm -f /mnt/mmcblk0p3/' . substr($file, strrpos($file, '/')));
//                                    if ($error_counter > 5) {
//                                        $error_counter++;
//                                        $i--;
//                                    }
//                                    $content = "";
//                                    $content .= "Count = $error_counter" . "\n";
//                                    $content .= "file = $filename" . "\n";
//                                    $download_log = "/usr/share/apache2/default-site/htdocs/configuration/error.ini";
//                                    $error = fopen($download_log, 'w+');
//                                    fwrite($error, $content);
//                                    fclose($handle);
//                                    sleep(1);
//                                }
//                                sleep(1);
                            }
                            if($arguments !== '') {
                                $arguments .= $last . ' ';
                                $arguments = rtrim($arguments, ' ');
                                exec("sudo /usr/share/apache2/default-site/htdocs/engine/wget " . $arguments, $result, $result);
                            }

//                            sleep(3);
//
//                            exec("sudo reboot", $result, $result);
                        } else { // partial update
                            $data = json_decode(json_encode($result));
                            $web_file = self::WEB_MAIN . $data->hv . $data->require . $data->sv . $data->type . '/' . $data->list[0]->filename;
                            //var_dump(file_get_contents($web_file));
                            $file_name = basename($data->list[0]->url);
                            $dir = '/usr/share/apache2/default-site/htdocs/';
                            if (!file_put_contents($dir . $file_name, file_get_contents($web_file))) {
                                return false;
                            }
                            $zip = new ZipArchive;
                            $res = $zip->open($dir . $file_name);
                            if ($res === TRUE) {
                                $zip->extractTo($dir);
                                $zip->close();
                                // return true;
                                chmod($dir . 'update/prupdate', 0777);
                                exec('sudo ' . $dir . 'update/prupdate', $output, $retval);
                                return $output;
                            } else {
                                return false;
                            }
                        }
                    }
                }
            }
        }
    }

    /**
     * Get dynamic main data
     * Call from script.js
     * @throws Exception
     */
    public function getMainData()
    {
        if ($this->getIsDeviceUpdated() === 0) {
            exec($this->sync->startUpdate() . " > /dev/null &", $output, $result);
        }

        /////////////////////////////// Get current data
        $this->conn->setQuery("UPDATE device_config SET last_update = current_timestamp");
        $current_data = $this->conn->getRow("SELECT current_temp                                    AS current_temp,
                                                    temp,
                                                    current_state.mode_id                                 AS mode,
                                                    mode.alias                                            AS mode_alias,
                                                    wifi_status                                           AS wifi_signal,
                                                    (SELECT humidifier_id FROM device_config)             AS has_hum,
                                                    (SELECT brightness FROM device_config)                AS brightness,
                                                    (SELECT brightness_mode FROM device_config)           AS brightness_mode,
                                                    current_humidity,
                                                    co2.alias                                             AS co2,
                                                    hold_status,
                                                    fan_status                                            AS fan,
                                                    (SELECT 
                                                         CASE 
                                                             WHEN time = 0 THEN to_char(to_timestamp(current_timestamp::varchar, 'yyyy-mm-dd HH24:MI:SS'), 'yyyy-mm-dd HH12:MI:SS PM')
                                                             ELSE current_timestamp::varchar
                                                         END
                                                     FROM settings)                                       AS date,
                                                    (SELECT measure_id FROM settings),
                                                    (SELECT time       FROM settings),
                                                    (SELECT is_enable  FROM vacation)                     AS is_vacation,
                                                    state_id,
                                                    (SELECT blink_mode FROM current_stage)                AS blink_mode,
                                                    COALESCE(state_next_check::varchar,'')                AS state_next_check,
                                                    (SELECT json_build_object('id',id,'type_id',type_id,'name',name,'text',text) AS alert 
                                                        FROM alerts WHERE status = true LIMIT 1) AS alert,
                                                    current_timestamp,
                                                    (SELECT system_type FROM device_config)               AS system_type
                                            FROM current_state
                                                LEFT JOIN co2  ON current_state.co2_id = co2.co2_id
                                                LEFT JOIN mode ON current_state.mode_id = mode.mode_id");

        if (!empty($current_data)) {
            $current_data["brightness_mode"] = ($current_data["brightness_mode"] === 't');
            $current_data['hold_status'] = ($current_data['hold_status'] === 't');
            if ($current_data['hold_status']) {
                $this->conn->setQuery("UPDATE current_state SET state_id = 0,is_sent = false");
            }

            //echo (int)$current_data['state_id']." - 2"."\n";
            //echo 'state_next_check:'. $current_data['state_next_check']."\n";
            // echo 'current:'. $current_data['date']."\n";
            if (!(int)$current_data['hold_status'] && $current_data['mode_alias'] !== 'off' && $current_data['state_next_check'] <= $current_data['current_timestamp'] && $current_data['state_next_check'] !== '') {
                $schedule = self::checkCurrentSchedule('');
                //var_dump($schedule);
                if (!empty($schedule)) {
                    $current_data['temp'] = $schedule['temp'];
                    if ((int)$current_data['system_type'] !== 3 && (int)$current_data['system_type'] !== 4) { // isn't cool or heat only
                        $current_data['mode'] = (int)$this->conn->getItem("SELECT mode_id FROM mode WHERE alias = 'auto'");
                    }
                    $this->conn->setQuery("UPDATE current_state SET state_id = 2,is_sent = false");
                } else {
                    $this->conn->setQuery("UPDATE current_state SET state_id = 0,is_sent = false");
                }
            } elseif (!$current_data['hold_status']) { // without hold
                $this->conn->setQuery("UPDATE current_state SET state_id = 0,is_sent = false");
            }

            $current_data['is_vacation'] = ($current_data['is_vacation'] === 't');
            $current_data['fan'] = ($current_data['fan'] === 't');
            $current_data['has_hum'] = (int)$current_data['has_hum'];
            $current_data['current_humidity'] = (int)$current_data['current_humidity'];
            $current_data['brightness'] = (int)$current_data['brightness'];
            $current_data['measure_id'] = (int)$current_data['measure_id'];
            $current_data['time'] = (int)$current_data['time'];
            $current_data['mode'] = (int)$current_data['mode'];
            $current_data['blink_mode'] = (int)$current_data['blink_mode'];
            $current_data['state_id'] = (int)$current_data['state_id'];
            $current_data['wifi_signal'] = (int)$current_data['wifi_signal'];

            if ($current_data['measure_id'] === 1) {                                                  // if measure is fahrenheit
                $current_data['current_temp'] = round($current_data['current_temp'] * 9 / 5 + 32);
                $current_data['temp'] = round($current_data['temp'] * 9 / 5 + 32);
            } else {
                $current_data['temp'] = (int)$current_data['temp'];
                $current_data['current_temp'] = (int)$current_data['current_temp'];
            }
            $current_data['current_weekday'] = (int)date('w', strtotime($current_data['date']));
            //$current_data['alert'] = json_decode($current_data['alert']);
            $current_data['alert'] = null;
            unset($current_data['acc1n']);
            unset($current_data['acc2']);
            unset($current_data['current_timestamp']);
            unset($current_data['system_type']);
            return $current_data;
        } else {
            return [];
        }
    }

    /**
     * Check if exist current schedule by timestamp
     * @param string $time
     * @return array
     * @throws Exception
     */
    public function checkCurrentSchedule(string $time = '')
    {
        if ($time === '') {
            $time = $this->conn->getItem("SELECT current_timestamp FROM current_state");
        }
        $date = $time;
        $dayOfWeek = (int)date("N", strtotime($date));
        if ($dayOfWeek === 7) {
            $dayOfWeek = 0;
        }
        $column_name = 'd' . $dayOfWeek;
        return $this->conn->getRow("SELECT temp,schedule_id
                                          FROM schedules
                                          WHERE is_remove = false AND is_enable = true AND {$column_name} = true AND current_timestamp > (SELECT state_next_check FROM current_state)
                                                AND ((start_time <= cast(timestamp '{$time}' as time)
                                                AND end_time > cast(timestamp '{$time}' as time)) OR (start_time > end_time AND start_time <= cast(timestamp '{$time}' as time)
                                                    AND end_time < cast(timestamp '{$time}' as time)))");
    }

    /**
     * Set next schedule's start time
     * @return bool
     * @throws Exception
     */
    public function setNextSchedule()
    {
        $info = $this->conn->getRow("SELECT
                                                CASE
                                                    WHEN state_next_check >= current_timestamp THEN state_next_check
                                                    ELSE current_timestamp
                                                END AS date,
                                                current_time
                                           FROM current_state");
        $date = $info['date'];
        $dayOfWeek = (int)date("N", strtotime($date));
        if ($dayOfWeek === 7) {
            $dayOfWeek = 0;
        }
        $schedules = $this->conn->getTable("SELECT schedule_id,start_time,end_time,d0,d1,d2,d3,d4,d5,d6
                                                  FROM schedules
                                                  WHERE is_enable = true AND is_remove = false
                                                  ORDER BY start_time");
        if (!empty($schedules)) {
            $stop = false;
            $j = $dayOfWeek;
            while ($j <= 6 && !$stop) {
                for ($i = 0; $i < count($schedules); $i++) {
                    if ($schedules[$i]['d' . $j] === 't') {
                        if ($info['current_time'] < $schedules[$i]['start_time']) {
                            $next_sch_time = $schedules[$i]['start_time'];
                            $next_sch_weekday = $j;
                            $stop = true;
                            break;
                        }
                    }
                }
                $j++;
            }
            for ($j = 0; $j < $dayOfWeek && !$stop; $j++) {
                for ($i = 0; $i < count($schedules); $i++) {
                    if ($schedules[$i]['d' . $j] === 't') {
                        if ($info['current_time'] < $schedules[$i]['start_time']) {
                            $next_sch_time = $schedules[$i]['start_time'];
                            $next_sch_weekday = $j;
                            $stop = true;
                            break;
                        }
                    }
                }
            }
            if (isset($next_sch_weekday)) {
                if ($next_sch_weekday - $dayOfWeek < 0) {
                    $days = 6 - $dayOfWeek + $next_sch_weekday + 1;
                } else {
                    $days = $next_sch_weekday - $dayOfWeek;
                }
                $timestamp = $days * 60 * 60 * 24 + strtotime($date);
                $date = new DateTime(date('Y-m-d', $timestamp));
                $next_sch = $date->format('Y-m-d') . ' ' . $next_sch_time;
                return (bool)$this->conn->setQuery("UPDATE current_state SET state_next_check = '{$next_sch}'")['result'];
            }
        } else {
            return (bool)$this->conn->setQuery("UPDATE current_state SET state_next_check = null")['result'];
        }
    }


    /**
     * Set current schedule's start time
     * @return bool
     * @throws Exception
     */
    public function setCurrentSchedule()
    {
        $info = $this->conn->getRow("SELECT
                                                CASE
                                                    WHEN state_next_check >= current_timestamp THEN state_next_check
                                                    ELSE current_timestamp
                                                END AS date,
                                                current_time
                                           FROM current_state");
        $date = $info['date'];
        $dayOfWeek = (int)date("N", strtotime($date));
        if ($dayOfWeek === 7) {
            $dayOfWeek = 0;
        }
        $schedules = $this->conn->getTable("SELECT schedule_id,start_time,end_time,d0,d1,d2,d3,d4,d5,d6
                                                  FROM schedules
                                                  WHERE is_enable = true AND is_remove = false
                                                  ORDER BY start_time");
        if (!empty($schedules)) {
            $stop = false;
            $j = $dayOfWeek;
            while ($j <= 6 && !$stop) {
                for ($i = 0; $i < count($schedules); $i++) {
                    if ($schedules[$i]['d' . $j] === 't') {
                        //if ($info['current_time'] < $schedules[$i]['start_time']) {
                        $next_sch_time = $schedules[$i]['start_time'];
                        $next_sch_weekday = $j;
                        $stop = true;
                        break;
                        //}
                    }
                }
                $j++;
            }
            for ($j = 0; $j < $dayOfWeek && !$stop; $j++) {
                for ($i = 0; $i < count($schedules); $i++) {
                    if ($schedules[$i]['d' . $j] === 't') {
                        //if ($info['current_time'] < $schedules[$i]['start_time']) {
                        $next_sch_time = $schedules[$i]['start_time'];
                        $next_sch_weekday = $j;
                        $stop = true;
                        break;
                        //}
                    }
                }
            }
            if (isset($next_sch_weekday)) {
                if ($next_sch_weekday - $dayOfWeek < 0) {
                    $days = 6 - $dayOfWeek + $next_sch_weekday + 1;
                } else {
                    $days = $next_sch_weekday - $dayOfWeek;
                }
                $timestamp = $days * 60 * 60 * 24 + strtotime($date);
                $date = new DateTime(date('Y-m-d', $timestamp));
                $next_sch = $date->format('Y-m-d') . ' ' . $next_sch_time;
                return (bool)$this->conn->setQuery("UPDATE current_state SET state_next_check = '{$next_sch}'")['result'];
            }
        } else {
            return (bool)$this->conn->setQuery("UPDATE current_state SET state_next_check = null")['result'];
        }
    }

    /**
     * Set temperature
     * @param int $temp
     * @return bool
     * @throws Exception
     */
     // call from script.js
    public function setTemperature(int $temp)
    {
        $settings = $this->conn->getRow("SELECT measure_id,
                                                      (SELECT state_id FROM current_state)                         AS state_id,
                                                      (SELECT mode.alias
                                                       FROM current_state 
                                                           LEFT JOIN mode ON current_state.mode_id = mode.mode_id) AS mode
                                               FROM settings");
        if ((int)$settings['measure_id'] === 1) {                                      // if measure is fahrenheit
            if ($temp < 65 || $temp > 85) {
                throw new Exception("Incorrect data", 1);
            }
            $temp = number_format(((int)$temp - 32) * 5 / 9, 2, '.', '');
        } else if ((int)$settings['measure_id'] === 0) {                               // if measure is celsius
            if ($temp < 18 || $temp > 30) {
                throw new Exception("Incorrect data", 1);
            }
        } else {
            throw new Exception("Something went wrong", 1);
        }
        $query = "UPDATE current_state SET temp = '{$temp}', is_sent = false";
        if ($settings['mode'] === 'off') {
            $query .= ",mode_id = (SELECT mode_id FROM mode WHERE alias = 'auto')";
        }
        //return self::checkCurrentSchedule();
        if (empty(self::checkCurrentSchedule())) {
            self::setNextSchedule();
        } else {
            $this->conn->setQuery("UPDATE current_state SET is_sent = false,state_next_check = null,state_id = 0");
        }
        $query .= ";UPDATE timing SET s1uptime = current_timestamp,uptime = current_timestamp,s2uptime = current_timestamp,
                                      s2hold = false,s3hold = false,alerts = false;";
        return (bool)$this->conn->setQuery($query, true)['result'];
    }

    /**
     * Get Humidity
     * @return int[]                     // type: 1 - humidifier, 2 - dehumidifier, 3 - no
     * @throws Exception
     */
    public function getHumidity()
    {
        $humidity = $this->conn->getRow("SELECT humidity AS set_humidity,
                                                      (SELECT humidifier_id FROM device_config) AS type
                                               FROM current_state");


//        if($humidity['type'] === 't') {
//            $humidity['type'] = 0;
//        } else {
//            $humidity['type'] = 1;
//        }
        $humidity['set_humidity'] = (int)$humidity['set_humidity'];
        $humidity['type'] = (int)$humidity['type'];
        return $humidity;
    }

    /**
     * Set humidity
     * @param int $humidity
     * @return bool
     * @throws Exception
     */
    public function setHumidity(int $humidity)
    {
        if ($humidity < 20 || $humidity > 80) {
            throw new Exception("Incorrect data", 1);
        }
        $query = "UPDATE current_state SET humidity = '{$humidity}',is_sent = false";
        $mode = $this->conn->getItem("SELECT mode.alias
                                            FROM current_state
                                                LEFT JOIN mode ON current_state.mode_id = mode.mode_id");
        if ($mode === 'off') {
            $query .= ",mode_id = (SELECT mode_id FROM mode WHERE alias = 'auto')";
        }
        return (bool)$this->conn->setQuery($query)['result'];
    }

    /**
     * Set hold status
     * @param bool $hold
     * @return bool
     * @throws Exception
     */
    public function setHold(bool $hold)
    {
        if ($hold === true) {
            $query = "UPDATE current_state SET hold_status = true,is_sent = false";
            $state = (int)$this->conn->getItem("SELECT state_id FROM current_state");
            if ($state === 2) {
                $schedule = $this->checkCurrentSchedule('');
                if (!empty($schedule)) {
                    $query .= ",temp = '{$schedule['temp']}',mode_id = (SELECT mode_id FROM mode WHERE alias = 'auto'),state_id = 0";
                }
            }
            //return $query;
            return (bool)$this->conn->setQuery($query)["result"];     // change state to set
        } else {
            $query = "UPDATE current_state SET hold_status = false,is_sent = false";
            $is_schedule = self::checkCurrentSchedule('');
            if (!empty($is_schedule)) {
                $query .= ",state_id = 0";                            // change state to set
            }
//            if (!empty($is_schedule)) {
//                $query .= ",state_id = 2";                                  // change state to schedule
//            }
            return (bool)$this->conn->setQuery($query)["result"];                      // change set to schedule
        }
    }

    /**
     * Set fan
     * @param int $fan // 0 - auto
     * @return bool
     * @throws Exception
     */
    public function setFan(int $fan)
    {
        $query1 = '';
        if ($fan === 0 || $fan % 5 === 0) {
            $query = "UPDATE current_state SET fan = '{$fan}',is_sent = false";
            if ($fan === 0) {
                $query .= ",fan_status = false";
            } else {
                $query .= ",fan_status = true";
                $query1 .= "UPDATE timing SET fan_time = current_timestamp;";
            }
            return (bool)$this->conn->setQuery($query1 . $query, true)['result'];
        } else {
            throw new Exception("Incorrect data", 1);
        }
    }

    /**
     * Get fan value
     * @return int
     */
    public function getFan()
    {
        return (int)$this->conn->getItem("SELECT fan FROM current_state");
    }

    /**
     * Change measure
     * @param int $measure // 0 - celsius, 1 - fahrenheit
     * @return bool
     * @throws Exception
     */
    public function setMeasure(int $measure)
    {
        if ($measure !== 0 && $measure !== 1) {
            throw new Exception("Incorrect data", 1);
        }
        return (bool)$this->conn->setQuery("UPDATE settings SET measure_id = '{$measure}',is_sent = false")['result'];
    }

    /**
     * Set mode
     * @param int $mode // 1 - cooling, 2 - heating, 3 - auto
     * @return bool
     * @throws Exception
     */
    public function setMode(int $mode)
    {
        $query = '';
        if (empty(self::checkCurrentSchedule())) {
            self::setNextSchedule();
        } else {
            $query .= "UPDATE current_state SET is_sent = false,state_next_check = null,state_id = 0;";
        }
        $query .= "UPDATE current_state SET mode_id = '{$mode}',is_sent = false;
                   UPDATE timing SET s1uptime = current_timestamp,
                                     uptime = current_timestamp,s2uptime = current_timestamp,
                                     s2hold = false,s3hold = false,alerts = false";
        return (bool)$this->conn->setQuery($query)["result"];
    }

    /**
     * Get enable modes
     * @return array
     * @throws Exception
     */
    public function getMode()
    {
        $mode = [['alias' => 'cooling', 'enable' => false], ['alias' => 'heating', 'enable' => false], ['alias' => 'auto', 'enable' => false], ['alias' => 'vacation', 'enable' => true]];
        $wirings = $this->conn->getTable("SELECT alias,type
                                                FROM wirings
                                                WHERE alias = 'w1' OR alias = 'y1' OR alias = 'o/b'");

        $is_ob = false;
        $is_w = false;
        $is_y = false;

        for ($i = 0; $i < count($wirings); $i++) {
            if ($wirings[$i]['alias'] === 'o/b') {
                $is_ob = $wirings[$i]['type'] === 't';
            }
            if ($wirings[$i]['alias'] === 'w1') {
                $is_w = $wirings[$i]['type'] === 't';
            }
            if ($wirings[$i]['alias'] === 'y1') {
                $is_y = $wirings[$i]['type'] === 't';
            }
        }

        if ($is_ob) {
            $mode[0] = ['alias' => 'cooling', 'enable' => true];
            $mode[1] = ['alias' => 'heating', 'enable' => true];
            $mode[2] = ['alias' => 'auto', 'enable' => true];
        } else {
            if ($is_y) {
                $mode[0] = ['alias' => 'cooling', 'enable' => true];
            }
            if ($is_w) {
                $mode[1] = ['alias' => 'heating', 'enable' => true];
            }
            if ($is_y && $is_w) {
                $mode[2] = ['alias' => 'auto', 'enable' => true];
            }
            if (!$is_y && !$is_w) {
                $mode[3] = ['alias' => 'vacation', 'enable' => false];
            }
        }
        return $mode;
    }

    /**
     * Set off
     * @return bool
     * @throws Exception
     */
    public function setOff()
    {
        self::setNextSchedule();
        return $this->conn->setQuery("UPDATE current_state SET mode_id = (SELECT mode_id FROM mode WHERE alias = 'off'),
                                                                     is_sent = false,
                                                                     state_id = 0;
                                            UPDATE relays SET type = false 
                                            WHERE alias IN ('g','y1','y2','acc2','w1','w2','w3','o/b','acc1p','acc1n');", true)['result'];
    }

    /**
     * Get vacation data
     * @return int[]                             // [temperature_min,temperature_max,humidity_min,humidity_max]
     * @throws Exception
     */
    public function getVacationData()
    {
        $vacation_data = [];
        $vacation = $this->conn->getRow("SELECT min_temp,
                                                      max_temp,
                                                      min_humidity,
                                                      max_humidity,
                                                      (SELECT measure_id FROM settings),
                                                      (SELECT humidifier_id FROM device_config) AS humidifier_id
                                               FROM vacation");
        if (!empty($vacation)) {
            if ((int)$vacation['measure_id'] === 1) {                                   // if measure is fahrenheit
                $vacation_data[] = round($vacation['min_temp'] * 9 / 5 + 32);
                $vacation_data[] = round($vacation['max_temp'] * 9 / 5 + 32);
            } else {
                $vacation_data[] = round($vacation['min_temp']);
                $vacation_data[] = round($vacation['max_temp']);
            }
            if ($vacation_data[0] < 65) {
                $vacation_data[0] = 65;
            }
            if ($vacation_data[1] > 85) {
                $vacation_data[1] = 85;
            }
            $vacation_data[] = (int)$vacation['min_humidity'];
            $vacation_data[] = (int)$vacation['max_humidity'];
            $vacation_data[] = (int)$vacation['humidifier_id'];
            return $vacation_data;
        } else {
            throw new Exception("Incorrect data");
        }
    }

    /**
     * Set vacation data
     * @param int $min_temp
     * @param int $max_temp
     * @param int $min_humidity
     * @param int $max_humidity
     * @return bool
     * @throws Exception
     */
    public function setVacation(int $min_temp, int $max_temp, int $min_humidity, int $max_humidity)
    {
        $measure = (int)$this->conn->getItem("SELECT measure_id FROM settings");
        if ($measure === 1) {                                            // if measure is fahrenheit
            if ($max_temp - $min_temp < 10) {                         // range is 10.8 in fahrenheit
                throw new Exception("Incorrect data", 1);
            }
            $min_temp = number_format(((int)$min_temp - 32) * 5 / 9, 2, '.', '');
            $max_temp = number_format(((int)$max_temp - 32) * 5 / 9, 2, '.', '');
        } else if ($measure === 0) {                                     // if measure is celsius
            if ($max_temp - $min_temp < 6) {
                throw new Exception("Incorrect data", 1);
            }
        } else {
            throw new Exception("Something went wrong", 1);
        }
        if ($max_humidity - $min_humidity < 10) {
            throw new Exception("Incorrect data", 1);
        }
        return $this->conn->setQuery("UPDATE vacation SET is_enable = true,min_temp = '{$min_temp}',max_temp = '{$max_temp}',min_humidity = '{$min_humidity}',
                                                                max_humidity = '{$max_humidity}',is_sent = false;
                                            UPDATE current_state SET state_id = 1,is_sent = false", true)['result'];
    }

    /**
     * Enable or disable vacation
     * @param bool $status
     * @return bool
     * @throws Exception
     */
    public function enableVacation(bool $status)
    {
        if ($status === true) {
            return $this->conn->setQuery("UPDATE current_state SET state_id = 1,is_sent = false;
                                                UPDATE vacation SET is_enable = true", true)['result'];
        } else {
            $is_schedule = self::checkCurrentSchedule('');
            if (empty($is_schedule)) {
                $query = "UPDATE current_state SET state_id = 0,is_sent = false;";          // change state to set
            } else {
                $query = "UPDATE current_state SET state_id = 2,is_sent = false;";          // change state to schedule
            }
            $result = (bool)$this->conn->setQuery($query . "UPDATE vacation SET is_enable = false", true)['result'];
            if ($result) {
                return $this->conn->getItem("SELECT contractor_name FROM device_config");
            } else {
                return $result;
            }
        }
    }

    /**
     * Get all alerts
     * @return array[]                            // type: 1 - alert, 2 - reminder, 3 - notification
     * @throws Exception
     */
     // * call from script.js
    public function getAlerts()
    {
        $alerts = $this->conn->getTable("SELECT id,
                                                      alerts.name,
                                                      alerts.type_id AS type
                                               FROM alerts
                                                    LEFT JOIN alert_types ON alerts.type_id = alert_types.type_id");
        for ($i = 0; $i < count($alerts); $i++) {
            $alerts[$i]['id'] = (int)$alerts[$i]['id'];
            $alerts[$i]['type'] = (int)$alerts[$i]['type'];
        }
        return $alerts;
    }

    /**
     * Get alert data
     * @param int $id
     * @return array                              // [info,name,type] // type: 1 - alert, 2 - reminder, 3 - notification
     * @throws Exception
     */
    public function getAlertData(int $id)
    {
        $alert = $this->conn->getRow("SELECT text    AS info,
                                                   name,
                                                   type_id AS type
                                            FROM alerts
                                            WHERE id = '{$id}'");
        $alert['type'] = (int)$alert['type'];
        return $alert;
    }

    /**
     * Set alert's status as read
     * @param int $id
     * @return bool
     * @throws Exception
     */
    public function setAlertAsRead(int $id)
    {
        return (bool)$this->conn->setQuery("UPDATE alerts SET status = false WHERE id = '{$id}'")['result'];
    }

    /**
     * Get about info
     * @return string[]                           // [logo,phone,url]
     * @throws Exception
     */
    public function about()
    {
        $about = $this->conn->getRow("SELECT logo,phone,url,is_service_titan FROM device_config");
        if (!empty($about)) {
            if ($about['is_service_titan'] === 't') {
                $about['is_service_titan'] = true;
            } else {
                $about['is_service_titan'] = false;
            }
            return [$about['logo'], $about['phone'], $about['url'], $about['is_service_titan']];
        } else {
            return [];
        }
    }

    /**
     * Send request job to web server
     * @param string $type
     * @return false
     */
    public function requestJob(string $type)
    {
        $serial_number = $this->conn->getItem("SELECT serial_number FROM device_config");
        $data = json_encode(
            [
                'request' =>
                    [
                        'class' => 'sync',
                        'method' => 'requestJob',
                        'params' => [$serial_number, $type]
                    ],
                'user' =>
                    [
                        'lang_id' => 0,
                        'user_id' => 0,
                        'type_id' => 0,
                        'host_id' => 0,
                        'region_id' => 0,
                        'token' => ''
                    ]
            ]);
        $curl = curl_init();
        curl_setopt_array($curl, array(
            CURLOPT_URL => self::WEB_URL,
            CURLOPT_FAILONERROR => true,
            CURLOPT_RETURNTRANSFER => true,
            CURLOPT_ENCODING => '',
            CURLOPT_MAXREDIRS => 10,
            CURLOPT_TIMEOUT => 0,
            CURLOPT_FOLLOWLOCATION => true,
            CURLOPT_HTTP_VERSION => CURL_HTTP_VERSION_1_1,
            CURLOPT_CUSTOMREQUEST => 'POST',
            CURLOPT_POSTFIELDS => $data,
        ));
        $response = curl_exec($curl);
        if (curl_errno($curl)) {
            $error_msg = curl_error($curl);
        }
        curl_close($curl);
        if (!isset($error_msg)) {
            if ($response) {
                $result = json_decode($response)->result->result;
                return $result;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }


    /**
     * Get user guide link
     * @return string
     */
    public function userGuide()
    {
        return $this->conn->getItem("SELECT user_guide FROM device_config");
    }

//    /**
//     * Set current device's wirings
//     * @param array $wirings
//     * @return bool
//     * @throws Exception
//     */
//    public function setCurrentWiring(array $wirings)
//    {
//        $query = '';
//        for ($i = 0; $i < count($wirings); $i++) {
//            $wirings[$i] = (int)$wirings[$i];
////            if($wirings[$i] == 1) {
////                $wirings[$i] = 0;
////            } else {
////                $wirings[$i] = 1;
////            }
//            $query .= "UPDATE wirings SET type = '{$wirings[$i]}' WHERE order_id = '{$i}';";
//            $query .= "UPDATE wirings_temp SET type = '{$wirings[$i]}' WHERE order_id = '{$i}';";
//            if ($wirings[$i] === 0) {
//                $query .= "UPDATE relays SET type = '{$wirings[$i]}' WHERE order_id = '{$i}';";
//            }
//
//        }
//        return (bool)$this->conn->setQuery($query, true)['result'];
//    }

    /**
     * Shut down device
     * @return bool
     * @throws Exception
     */
    public function shutDown()
    {
        return (bool)$this->conn->setQuery("UPDATE device_config set shut_down = true")['result'];
        //exec("sudo shutdown -h now", $result, $retval);
    }

    /**
     * Reboot device
     * @return void
     */
    public function rebootDevice()
    {
        exec("sudo reboot", $result, $retval);
    }

    /**
     * Get current data for test
     * @return array
     * @throws Exception
     */
    public function getCurrentDataTest()
    {
        $current_data = $this->conn->getRow("SELECT (SELECT qa_test FROM device_config),
                                                           current_temp,
                                                           current_humidity,
                                                           (SELECT round(avg(co2)) FROM sensor_logs) AS co2     
                                                    FROM current_state");
        if ($current_data['qa_test'] === 't') {
            $current_data['qa_test'] = true;
        } else {
            $current_data['qa_test'] = false;
        }
        $current_data['current_temp'] = (int)$current_data['current_temp'];
        $current_data['current_humidity'] = (int)$current_data['current_humidity'];
        $current_data['co2'] = (int)$current_data['co2'];
        return $current_data;
    }

    /**
     * Set current data for test
     * @param bool $qa_test
     * @param int $temp
     * @param int $humidity
     * @param int $co2
     * @return bool
     * @throws Exception
     */
    public function setCurrentDataTest(bool $qa_test, float $temp, int $humidity, int $co2)
    {
        $qa_test = (int)$qa_test;
        $query = "UPDATE device_config SET qa_test = '{$qa_test}';";
        if ($qa_test === 1) {
            $query .= "UPDATE current_state SET is_sent = false,
                                                current_temp = '{$temp}',
                                                current_humidity = '{$humidity}',
                                                co2_id = ((SELECT
                                                               CASE
                                                                   WHEN '{$co2}' >= 0 AND '{$co2}' <= 600 THEN (SELECT co2_id FROM co2 WHERE alias = 'good')
                                                                   WHEN '{$co2}' >= 601 AND '{$co2}' <= 1100 THEN (SELECT co2_id FROM co2 WHERE alias = 'moderate')
                                                                   WHEN '{$co2}' >= 1101 THEN (SELECT co2_id FROM co2 WHERE alias = 'poor')
                                                               END
                                                            FROM co2
                                                            LIMIT 1))";
        } else {
            $query .= "UPDATE current_state SET is_sent = false,
                                                current_temp = (SELECT avg(temp) FROM sensor_logs),
                                                current_humidity = (SELECT avg(humidity) FROM sensor_logs),
                                                co2_id = (SELECT
                                                               CASE
                                                                   WHEN round(avg(co2)) >= 0 AND round(avg(co2)) <= 600 THEN (SELECT co2_id FROM co2 WHERE alias = 'good')
                                                                   WHEN round(avg(co2)) >= 601 AND round(avg(co2)) <= 1100 THEN (SELECT co2_id FROM co2 WHERE alias = 'moderate')
                                                                   WHEN round(avg(co2)) >= 1101 THEN (SELECT co2_id FROM co2 WHERE alias = 'poor')
                                                               END
                                                          FROM sensor_logs
                                                          LIMIT 1)";
        }
        return (bool)$this->conn->setQuery($query, true)['result'];
    }

    /**
     * Get brightness value
     * @return int
     */
    public function getBrightness()
    {
        return (int)$this->conn->getItem("SELECT brightness FROM settings");
    }

    /**
     * Get device's uid and software version
     * @return array
     */
    public function getUidSV()
    {
        $versions = parse_ini_file($_SERVER['DOCUMENT_ROOT'] . '/configuration/version.ini');
        $uid = $this->conn->getItem("SELECT uid FROM device_config");
        return ['soft_v' => $versions['SOFTWARE_VERSION'],
                'uid' => $uid,
                'hard_v' => $versions['HARDWARE_VERSION']];
    }

    /**
     * Get device's soft_v, hard_v, sn and ip
     * @return array
     */
    public function getVersionsIpSN()
    {
        $versions = parse_ini_file($_SERVER['DOCUMENT_ROOT'] . '/configuration/version.ini');
        $serial_number = $this->conn->getItem("SELECT serial_number FROM device_config");
        $ip_address=$_SERVER['REMOTE_ADDR'];
        return ['hard_v' => $versions['HARDWARE_VERSION'],
                'soft_v' => $versions['SOFTWARE_VERSION'],
                'ip' => $ip_address,
                'serial_number' => $serial_number];
    }

    /**
     * Get is device updated
     * @return int
     */
    public function getIsDeviceUpdated()
    {
        return (int)parse_ini_file($_SERVER['DOCUMENT_ROOT'] . DIRECTORY_SEPARATOR . 'configuration' . DIRECTORY_SEPARATOR . 'device_settings.ini')['UPDATED'];
    }

    /**
     * Get system type
     * @return int
     */
    public function getSystemType()
    {
        return (int)$this->conn->getItem("SELECT system_type FROM device_config");
    }

    /**
     * Set system type
     * @param int $type
     * @return bool
     * @throws Exception
     */
    public function setSystemType(int $type)
    {
        $query = '';
        if (empty($this->checkCurrentSchedule())) {
            $this->setNextSchedule();
        } else {
            $query .= "UPDATE current_state SET is_sent = false,state_next_check = null,state_id = 0;";
        }

        switch ($type) {
            case 1: // traditional
                $wiring_cool_count = (int)$this->conn->getItem("SELECT count(*) FROM wirings WHERE (alias = 'y1' OR alias = 'y2') AND type = true");
                if ($wiring_cool_count === 0) {
                    $wiring_cool_count = (int)$this->conn->getItem("SELECT value FROM default_wirings WHERE alias = 'traditional_cool_stages'");
                }
                if ($wiring_cool_count === 0) {
                    $wiring_cool_count = 1;
                }

                $wiring_heat_count = (int)$this->conn->getItem("SELECT count(*) FROM wirings WHERE (alias = 'w1' OR alias = 'w2' OR alias = 'w3') AND type = true");
                if ($wiring_heat_count === 0) {
                    $wiring_heat_count = (int)$this->conn->getItem("SELECT value FROM default_wirings WHERE alias = 'traditional_heat_stages'");
                }
                if ($wiring_heat_count === 0) {
                    $wiring_heat_count = 1;
                }

                $query .= "UPDATE wirings SET type = false WHERE alias = 'o/b' OR alias = 'w1' OR alias  = 'w2' OR alias  = 'w3' OR alias = 'y1' OR alias  = 'y2';";

                if ($wiring_cool_count === 1) {
                    $query .= "UPDATE wirings SET type = true WHERE alias = 'y1';";
                } else {
                    $query .= "UPDATE wirings SET type = true WHERE alias = 'y1' OR alias  = 'y2';";
                }

                if ($wiring_heat_count === 1) {
                    $query .= "UPDATE wirings SET type = true WHERE alias = 'w1';";
                } elseif ($wiring_heat_count === 2) {
                    $query .= "UPDATE wirings SET type = true WHERE alias = 'w1' OR alias  = 'w2';";
                } elseif ($wiring_heat_count === 3) {
                    $query .= "UPDATE wirings SET type = true WHERE alias = 'w1' OR alias  = 'w2' OR alias  = 'w3';";
                }
                break;
            case 2: // heat pump
                $wiring_count = (int)$this->conn->getItem("SELECT count(*) FROM wirings WHERE (alias = 'y1' OR alias = 'y2') AND type = true");
                if ($wiring_count === 0) {
                    $wiring_count = (int)$this->conn->getItem("SELECT value FROM default_wirings WHERE alias = 'traditional_cool_stages'");
                }
                if ($wiring_count === 0) {
                    $wiring_count = 1;
                }

                $query .= "UPDATE wirings SET type = false WHERE alias = 'o/b' OR alias = 'w1' OR alias  = 'w2' OR alias  = 'w3' OR alias = 'y1' OR alias  = 'y2';";

                if ($wiring_count === 1) {
                    $query .= "UPDATE wirings SET type = true WHERE alias = 'y1';";
                } else {
                    $query .= "UPDATE wirings SET type = true WHERE alias = 'y1' OR alias  = 'y2';";
                }
                $query .= "UPDATE wirings SET type = true WHERE alias = 'o/b';";

                break;
            case 3: // cool only
                $wiring_cool_count = (int)$this->conn->getItem("SELECT count(*) FROM wirings WHERE (alias = 'y1' OR alias = 'y2') AND type = true");
                if ($wiring_cool_count === 0) {
                    $wiring_cool_count = (int)$this->conn->getItem("SELECT value FROM default_wirings WHERE alias = 'traditional_cool_stages'");
                }
                if ($wiring_cool_count === 0) {
                    $wiring_cool_count = 1;
                }

                $query .= "UPDATE wirings SET type = false WHERE alias = 'o/b' OR alias = 'w1' OR alias  = 'w2' OR alias  = 'w3' OR alias = 'y1' OR alias  = 'y2';";

                if ($wiring_cool_count === 1) {
                    $query .= "UPDATE wirings SET type = true WHERE alias = 'y1';";
                } else {
                    $query .= "UPDATE wirings SET type = true WHERE alias = 'y1' OR alias  = 'y2';";
                }
                break;
            case 4: // heat only
                $wiring_heat_count = (int)$this->conn->getItem("SELECT count(*) FROM wirings WHERE (alias = 'w1' OR alias = 'w2' OR alias = 'w3') AND type = true");
                if ($wiring_heat_count === 0) {
                    $wiring_heat_count = (int)$this->conn->getItem("SELECT value FROM default_wirings WHERE alias = 'traditional_heat_stages'");
                }
                if ($wiring_heat_count === 0) {
                    $wiring_heat_count = 1;
                }

                $query .= "UPDATE wirings SET type = false WHERE alias = 'o/b' OR alias = 'w1' OR alias  = 'w2' OR alias  = 'w3' OR alias = 'y1' OR alias  = 'y2';";

                if ($wiring_heat_count === 1) {
                    $query .= "UPDATE wirings SET type = true WHERE alias = 'w1';";
                } elseif ($wiring_heat_count === 2) {
                    $query .= "UPDATE wirings SET type = true WHERE alias = 'w1' OR alias  = 'w2';";
                } elseif ($wiring_heat_count === 3) {
                    $query .= "UPDATE wirings SET type = true WHERE alias = 'w1' OR alias  = 'w2' OR alias  = 'w3';";
                }

                break;
        }
        return (bool)$this->conn->setQuery($query . "UPDATE device_config SET system_type = '{$type}';")["result"];
//        if ($type === 1 || $type === 3) {
//            $count = (int)$this->conn->getItem("SELECT count(*) FROM wirings WHERE (alias = 'y1' OR alias = 'y2') AND type = true");
//            if ($count === 0) {
//                $wirings_count = (int)$this->conn->getItem("SELECT value FROM default_wirings WHERE alias = 'traditional_cool_stages'");
//                if ($wirings_count === 2) {
//                    $query .= "UPDATE wirings SET type = true WHERE alias = 'y1' OR alias  = 'y2';";
//                } else if ($wirings_count === 1) {
//                    $query .= "UPDATE wirings SET type = true WHERE alias = 'y1';";
//                }
//            }
//            if ($type !== 3) {
//                $wirings_count = (int)$this->conn->getItem("SELECT value FROM default_wirings WHERE alias = 'traditional_heat_stages'");
//                if ($wirings_count === 3) {
//                    $query .= "UPDATE wirings SET type = true WHERE alias = 'w1' OR alias  = 'w2' OR alias = 'w3';";
//                } else if ($wirings_count === 2) {
//                    $query .= "UPDATE wirings SET type = true WHERE alias = 'w1' OR alias = 'w2';";
//                } else if ($wirings_count === 1) {
//                    $query .= "UPDATE wirings SET type = true WHERE alias = 'w1';";
//                }
//            }
//        }
//        if ($type === 1 || $type === 4) {
//            $count = (int)$this->conn->getItem("SELECT count(*) FROM wirings WHERE (alias = 'w1' OR alias = 'w2' OR alias = 'w3') AND type = true");
//            if ($count === 0) {
//                $wirings_count = (int)$this->conn->getItem("SELECT value FROM default_wirings WHERE alias = 'traditional_heat_stages'");
//                if ($wirings_count === 3) {
//                    $query .= "UPDATE wirings SET type = true WHERE alias = 'w1' OR alias  = 'w2' OR alias = 'w3';";
//                } else if ($wirings_count === 2) {
//                    $query .= "UPDATE wirings SET type = true WHERE alias = 'w1' OR alias = 'w2';";
//                } else if ($wirings_count === 1) {
//                    $query .= "UPDATE wirings SET type = true WHERE alias = 'w1';";
//                }
//            }
//            if ($type !== 4) {
//                $wirings_count = (int)$this->conn->getItem("SELECT value FROM default_wirings WHERE alias = 'traditional_cool_stages'");
//                if ($wirings_count === 2) {
//                    $query .= "UPDATE wirings SET type = true WHERE alias = 'y1' OR alias  = 'y2';";
//                } else if ($wirings_count === 1) {
//                    $query .= "UPDATE wirings SET type = true WHERE alias = 'y1';";
//                }
//            }
//        }
//        if ($type === 3) {         // cool only
//            $query .= "UPDATE wirings SET type = false WHERE alias = 'w1' OR alias = 'w2' OR alias = 'w3' OR alias = 'o/b';
//                       UPDATE current_state SET is_sent = false,mode_id = 1;";
//        } else if ($type === 4) {  // heat only
//            $query .= "UPDATE wirings SET type = false WHERE alias = 'y1' OR alias = 'y2' OR alias = 'o/b';
//                       UPDATE current_state SET is_sent = false,mode_id = 2;";
//        }
//        if ($type === 2) {
//            $query .= "UPDATE wirings SET type = false WHERE alias = 'o/b';";
//        }
//        if ($type === 2) {
//            $query .= "UPDATE wirings SET type = false WHERE alias = 'w1' OR alias  = 'w2' OR alias = 'w3';";
//            $query .= "UPDATE wirings SET type = true WHERE alias = 'o/b';";
//        }
//
//        return (bool)$this->conn->setQuery($query . "UPDATE device_config SET system_type = '{$type}';")["result"];
    }

    /**
     * Get traditional default or set stages
     * @throws Exception
     */
    public function getTraditionalStages()
    {
        $count = (int)$this->conn->getItem("SELECT count(*) FROM wirings WHERE (alias = 'w1' OR alias = 'w2' OR alias = 'w3' OR alias = 'y1' OR alias = 'y2') AND type = true");
        if ($this->getSystemType() === 1 && $count > 0) {
            return $this->conn->getRow("SELECT (SELECT count(*) FROM wirings WHERE (alias = 'w1' OR alias = 'w2' OR alias = 'w3') AND type = true) AS heating,
                                                     (SELECT count(*) FROM wirings WHERE (alias = 'y1' OR alias = 'y2') AND type = true)                 AS cooling");
        } else {
            return $this->conn->getRow("SELECT (SELECT value FROM default_wirings WHERE alias = 'traditional_cool_stages') AS cooling,
                                                     (SELECT value FROM default_wirings WHERE alias = 'traditional_heat_stages') AS heating");
        }
    }

    /**
     * Set traditional stages
     * @param int $cool
     * @param int $heat
     * @return bool
     * @throws Exception
     */
    public function setTraditionalStages(int $cool, int $heat)
    {
        $query = "UPDATE wirings SET type = false WHERE alias = 'o/b' OR alias = 'w2' OR alias = 'w3' OR alias = 'y2';
                  UPDATE relays SET type = false WHERE alias = 'o/b' OR alias = 'w2' OR alias = 'w3' OR alias = 'y2';
                  UPDATE wirings SET type = true WHERE ";
        if ($cool === 1) {
            $query .= "alias = 'y1'";
        } elseif ($cool === 2) {
            $query .= "alias = 'y1' OR alias = 'y2'";
        }
        if ($heat === 1) {
            $query .= "OR alias = 'w1'";
        } elseif ($heat === 2) {
            $query .= "OR alias = 'w1' OR alias = 'w2'";
        } elseif ($heat === 3) {
            $query .= "OR alias = 'w1' OR alias = 'w2' OR alias = 'w3'";
        }
        return (bool)$this->conn->setQuery($query, true)["result"];
    }

    /**
     * Get cool stages set or default
     * @return int
     */
    public function getCoolStages()
    {
        $count = (int)$this->conn->getItem("SELECT count(*) FROM wirings WHERE (alias = 'y1' OR alias = 'y2') AND type = true");
        if ($this->getSystemType() === 3 && $count > 0) {
            return $count;
        } else {
            return (int)$this->conn->getItem("SELECT value FROM default_wirings WHERE alias = 'cool_only_stages'");
        }
    }

    /**
     * Set cool stages
     * @param int $count
     * @return bool
     * @throws Exception
     */
    public function setCoolStages(int $count)
    {
        $query = "UPDATE wirings SET type = false WHERE alias = 'o/b' OR alias = 'w1' OR alias = 'w2' OR alias = 'w3' OR alias = 'y2';
                  UPDATE relays SET type = false WHERE alias = 'o/b' OR alias = 'w1' OR alias = 'w2' OR alias = 'w3' OR alias = 'y2';
                  UPDATE wirings SET type = true WHERE ";
        if ($count === 1) {
            $query .= "alias = 'y1'";
        } elseif ($count === 2) {
            $query .= "alias = 'y1' OR alias = 'y2'";
        }
        return (bool)$this->conn->setQuery($query, true)["result"];
    }

    /**
     * Get heat stages set or default
     * @return int
     */
    public function getHeatStages()
    {

        $count = (int)$this->conn->getItem("SELECT count(*) FROM wirings WHERE (alias = 'w1' OR alias = 'w2' OR alias = 'w3') AND type = true");
        if ($this->getSystemType() === 4 && $count > 0) {
            return $count;
        } else {
            return (int)$this->conn->getItem("SELECT value FROM default_wirings WHERE alias = 'heat_only_stages'");
        }
    }

    /**
     * Set heat stages
     * @param int $count
     * @return bool
     * @throws Exception
     */
    public function setHeatStages(int $count)
    {
        $query = "UPDATE wirings SET type = false WHERE alias = 'o/b' OR alias = 'y1' OR alias = 'w2' OR alias = 'w3' OR alias = 'y2';
                  UPDATE relays SET type = false WHERE alias = 'o/b' OR alias = 'y1' OR alias = 'w2' OR alias = 'w3' OR alias = 'y2';
                  UPDATE wirings SET type = true WHERE ";
        if ($count === 1) {
            $query .= "alias = 'w1'";
        } elseif ($count === 2) {
            $query .= "alias = 'w1' OR alias = 'w2'";
        } elseif ($count === 3) {
            $query .= "alias = 'w1' OR alias = 'w2' OR alias = 'w3'";
        }
        return (bool)$this->conn->setQuery($query, true)["result"];
    }

    /**
     * Get heat pump stages set or default
     * @return array
     * @throws Exception
     */
    public function getHeatPumpStages()
    {
        $count = $this->conn->getItem("SELECT count(*) FROM wirings WHERE (alias = 'w1' OR alias = 'w2' OR alias = 'y1' OR alias = 'y2') AND type = true");
        if ($this->getSystemType() === 2 && $count > 0) {
            $data = $this->conn->getRow("SELECT emergency_heating,ob_state,
                                                    (SELECT count(*) FROM wirings WHERE (alias = 'w1' OR alias = 'w2') AND type = true) AS em_stages,
                                                    (SELECT count(*) FROM wirings WHERE (alias = 'y1' OR alias = 'y2') AND type = true) AS heat_pump_stages 
                                              FROM device_config");
            if ((int)$data['em_stages'] === 0) {
                $data['em_stages'] = (int)$this->conn->getItem("SELECT value FROM default_wirings WHERE alias = 'heat_pump_em_stages'");
            }
            if ((int)$data['heat_pump_stages'] === 0) {
                $data['heat_pump_stages'] = (int)$this->conn->getItem("SELECT value FROM default_wirings WHERE alias = 'heat_pump_hp_stages'");
            }
            return $data;
        } else {
            $result = [];
            $heat_pump = $this->conn->getTable("SELECT alias,value 
                                                      FROM default_wirings 
                                                      WHERE alias = 'heat_pump_emergency' OR alias = 'heat_pump_em_stages' OR alias = 'heat_pump_hp_stages' 
                                                            OR alias = 'heat_pump_state'");
            for ($i = 0; $i < count($heat_pump); $i++) {
                switch ($heat_pump[$i]['alias']) {
                    case 'heat_pump_emergency':
                        $result['emergency_heating'] = $heat_pump[$i]['value'];
                    case 'heat_pump_em_stages':
                        $result['em_stages'] = $heat_pump[$i]['value'];
                    case 'heat_pump_hp_stages':
                        $result['heat_pump_stages'] = $heat_pump[$i]['value'];
                    case 'heat_pump_state':
                        $result['ob_state'] = $heat_pump[$i]['value'];
                }
            }
            return $result;
        }
    }

    /**
     * Set heat pump stages
     * @param int $emergency_heating
     * @param int $em_stages
     * @param int $heat_pump_stages
     * @param string $ob_state
     * @return bool
     * @throws Exception
     */
    public function setHeatPumpStages(int $emergency_heating, int $em_stages, int $heat_pump_stages, string $ob_state)
    {
        $query = "UPDATE wirings SET type = false WHERE alias = 'w1' OR alias = 'w2' OR alias = 'w3' OR alias = 'y2';
                  UPDATE relays SET type = false WHERE alias = 'w1' OR alias = 'w2' OR alias = 'w3' OR alias = 'y2';
                  UPDATE wirings SET type = true WHERE alias = 'o/b' OR ";
        if ($emergency_heating === 1) {
            if ($em_stages === 1) {
                $query .= "alias = 'w1' OR ";
            } elseif ($em_stages === 2) {
                $query .= "alias = 'w1' OR alias = 'w2' OR ";
            }
        }
        if ($heat_pump_stages === 1) {
            $query .= "alias = 'y1'";
        } elseif ($heat_pump_stages === 2) {
            $query .= "alias = 'y1' OR alias = 'y2'";
        }
        $query .= ";UPDATE device_config SET emergency_heating = '{$emergency_heating}',ob_state = '{$ob_state}';";
        //return $query;
        return (bool)$this->conn->setQuery($query, true)["result"];
    }

    /**
     * Get accessories (humidifier and wiring)
     * @return array
     * @throws Exception
     */
    public function getAccessories()
    {
        return $this->conn->getRow("SELECT humidifier_id, hum_wiring FROM device_config");
    }

    /**
     * Set accessories (humidifier and hum. wiring)
     * @param int $humidifier_id
     * @param string $hum_wiring
     * @return bool
     * @throws Exception
     */
    public function setAccessories(int $humidifier_id, string $hum_wiring)
    {
        $query = '';
        if ($humidifier_id === 3) {
            $query .= "UPDATE wirings SET type = false WHERE alias = 'acc2' OR alias = 'acc1p' OR alias = 'acc1n';
                       UPDATE relays SET type = false WHERE alias = 'acc2' OR alias = 'acc1p' OR alias = 'acc1n';";
        } else {
            if ($hum_wiring === 'acc2') {
                $query .= "UPDATE wirings SET type = false WHERE alias = 'acc1p' OR alias = 'acc1n';
                           UPDATE relays SET type = false WHERE alias = 'acc1p' OR alias = 'acc1n';";
            } elseif ($hum_wiring === 'acc1p') {
                $query .= "UPDATE wirings SET type = false WHERE alias = 'acc2' OR alias = 'acc1n';
                           UPDATE relays SET type = false WHERE alias = 'acc2' OR alias = 'acc1n';";
            } elseif ($hum_wiring === 'acc1n') {
                $query .= "UPDATE wirings SET type = false WHERE alias = 'acc2' OR alias = 'acc1p';
                           UPDATE relays SET type = false WHERE alias = 'acc2' OR alias = 'acc1p';";
            }
            $query .= "UPDATE wirings SET type = true WHERE alias = '{$hum_wiring}';";
        }
        return (bool)$this->conn->setQuery($query . "UPDATE device_config SET humidifier_id = '{$humidifier_id}',
                                                                                    hum_wiring = '{$hum_wiring}';", true)["result"];
    }

    /**
     * Get system delay
     * @return int
     */
    public function getSystemDelay()
    {
        return (int)$this->conn->getItem("SELECT system_delay FROM settings");
    }

    /**
     * Set system delay minutes
     * @param int $min
     * @return bool
     * @throws Exception
     */
    public function setSystemDelay(int $min)
    {
        return (bool)$this->conn->setQuery("UPDATE settings SET system_delay = '{$min}'")["result"];
    }

}
