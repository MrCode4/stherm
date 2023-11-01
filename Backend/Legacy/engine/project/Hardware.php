<?php

namespace project;

use core\db\Z_PgSQL;
use Exception;
use project\System;


/**
 * Work with hardware part
 * @package project
 */
class Hardware
{
    private $conn;
    private $sync;
    private const WRAP = "/usr/share/apache2/default-site/htdocs/engine/";
    private const TECHNIC_QR = "https://test.hvac.z-soft.am/#EN/USA/technician/view/";
    private const TECHNIC_EDIT_QR = "https://test.hvac.z-soft.am/#EN/USA/technician/edit/";
    private const EXEC_TIMEOUT_INTERVAL = 30;
    private const DELETE_INFO_INTERVAL = 7; // 1 week

    /**
     * @throws Exception
     */
    public function __construct()
    {
        $this->conn = Z_PgSQL::connection();
        $this->sync = new Sync();
        $this->wifi = new Wifi();
        $this->system = new System();
    }

    /**
     * Set device's config and timing default values if DB empty
     * @return void
     * @throws Exception
     */
    private function setDefaultValues($uid)
    {
        $technic_qr = self::TECHNIC_QR;
        $technic_edit_qr = self::TECHNIC_EDIT_QR;
        $delete_info_interval = self::DELETE_INFO_INTERVAL;
        $this->conn->setQuery("TRUNCATE TABLE  device_config; TRUNCATE TABLE  timing;");
        $version_info = parse_ini_file($_SERVER['DOCUMENT_ROOT'] . DIRECTORY_SEPARATOR . 'configuration' . DIRECTORY_SEPARATOR . 'version.ini');
        $s_version = $version_info['SOFTWARE_VERSION'];
        $h_version = $version_info['HARDWARE_VERSION'];
        $this->conn->setQuery("INSERT INTO device_config(soft_v, hard_v, mode, brightness, brightness_mode, serial_number, uid, timezone,
                          technical_access_link, backlight_rgb, backlight_type, backlight_status, last_update,
                          server_last_update, current_speed, logo, phone, url, user_guide, start_pairing, wiring_check,
                          is_service_titan, timezone_number, qa_test, forget_sensor,
                          contractor_name, ventilator,start_mode,shut_down,scheme_backlight_rgb,humidifier_id,hum_wiring,system_type,emergency_heating,ob_state,technical_edit_link)
                            VALUES ('{$s_version}', '{$h_version}', 1, '80', 0, '', '{$uid}', 'Pacific/Midway',
        '{$technic_qr}', '{0,0,0}', 0, false, current_timestamp, current_timestamp, null,
        'img/upload/nexgen.png', '', '', '', false, true, null, null, false, null,'NextGen',0,0,false,'{0,0,0}',3,'',1,0,'cool','{$technic_edit_qr}');
                            INSERT INTO timing(s1uptime, uptime, s2uptime, s2hold, s3hold, alerts, set_backlight_time, wiring_check_interval, wiring_check_timestamp, contractor_info_interval, contractor_info_timestamp, info_update_interval, info_update_timestamp, soft_update_timestamp,fan_time,start_fan_timing,delete_info_timestamp,delete_info_interval)
                            VALUES(current_timestamp,current_timestamp,current_timestamp,false,false,false,current_timestamp,10,current_timestamp,1,'2023-01-01 00:00:00',15,current_timestamp,'2023-01-01 00:00:00',current_timestamp - interval '5 minute',0,current_timestamp,'{$delete_info_interval}');
                            DELETE FROM current_stage WHERE 1=1; INSERT INTO current_stage(mode,stage,timestamp,blink_mode,s2offtime) VALUES(0,0,current_timestamp,0,current_timestamp - interval '5 minute')", true);
    }

    /**
     * Set system timezone
     * @return void
     * @throws Exception
     */
    private function setTimezone() {
        $config = $this->conn->getRow("SELECT timezone_number AS timezone FROM device_config");
        if(!empty($config)) {
            $timezone = $config['timezone'];
            if($timezone != '') {
                exec("timeout " . self::EXEC_TIMEOUT_INTERVAL . " " . self::WRAP . "setTimezone " . $timezone, $result, $retval);
                if ((int)$result[0] != 0) {
                    self::setAlert('alert', 0, 16, 0, 'Timezone:', "Can't set system timezone", 'system');
                }
            } else {
                self::setAlert('alert', 0, 20, 0, 'Timezone:', 'Unknown Timezone', 'system');
            }
        }
    }

    /**
     * Start device configuration
     * @return mixed device uid
     * @throws Exception
     */
    private function runDevice()
    {
        $device_config = $this->conn->getRow("SELECT uid,serial_number FROM device_config");
        if (empty($device_config)) {
            $device_config = ['uid' => '', 'serial_number' => ''];
        }
        exec("timeout " . self::EXEC_TIMEOUT_INTERVAL . " " . self::WRAP . "getUid.o", $result, $retval);
        $uid = $result[0];
        $this->conn->setQuery("UPDATE device_config SET uid = '{$uid}'");
        if ($device_config['uid'] === '' || is_null($device_config['uid'])) {
            $this->setDefaultValues($uid);
        } else {
            $this->conn->setQuery("UPDATE timing SET s1uptime = current_timestamp,
                                                           uptime = current_timestamp,
                                                           s2uptime = current_timestamp,
                                                           set_backlight_time = current_timestamp,
                                                           wiring_check_timestamp = current_timestamp,
                                                           contractor_info_timestamp = '2023-01-01 00:00:00',
                                                           info_update_timestamp = current_timestamp,
                                                           soft_update_timestamp = '2023-01-01 00:00:00',
                                                           fan_time = current_timestamp - interval '5 minute';
                                         UPDATE current_stage SET timestamp = current_timestamp");
        }
        $this->conn->setQuery("UPDATE sensors SET sensor = '{$uid}',is_sent = false WHERE is_main = true;", true);
        if (file_exists("/mnt/mmcblk1p3/wifi.ini")) {
        //if (file_exists("/mnt/mmcblk0p3/wifi.ini")) {
            $conn_wifi = parse_ini_file('/mnt/mmcblk1p3/wifi.ini');
            //$conn_wifi = parse_ini_file('/mnt/mmcblk0p3/wifi.ini');
            $this->wifi->connect($conn_wifi["ESSID"], $conn_wifi["UNIQUE_NAME"], $conn_wifi["PASSWORD"]);
            unlink("/mnt/mmcblk1p3/wifi.ini");
            //unlink("/mnt/mmcblk0p3/wifi.ini");
        }
        exec('sudo rm -f /mnt/mmcblk1p3/*', $output, $result);
        //exec('sudo rm -f /mnt/mmcblk0p3/*', $output, $result);

        if(file_exists('/usr/share/apache2/default-site/htdocs/update.zip')) {
            unlink('/usr/share/apache2/default-site/htdocs/update.zip');
        }
        if(file_exists('/usr/share/apache2/default-site/htdocs/update')) {
            rmdir('/usr/share/apache2/default-site/htdocs/update');
        }

        if (file_exists('need_recover.txt')) {
            exec('sudo rm -f need_recover.txt', $output, $result);
        }
        $this->setTimezone();
        return $uid;
    }

    /**
     * Get serial number from server by uid
     * @param string $uid
     * @return array
     */
    private function getSN(string $uid)
    {
        $data = json_encode(
            [
                'request' =>
                    [
                        'class' => 'sync',
                        'method' => 'getSN',
                        'params' => [$uid]
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
        $result = $this->sync->sendCurlRequest($data);
        return $result;
    }

    /**
     * Get start mode
     * @return int               // 0 - test, 1 - normal, 2 - first run
     * @throws Exception
     */
    public function getStartMode()
    {
        $uid = $this->runDevice();
        exec("timeout " . self::EXEC_TIMEOUT_INTERVAL . " " . 'sudo ' . self::WRAP . "getStartMode", $output, $retval);
        $result = (int)$output[0];
        if ($result === 0) {                                                                                            // PRODUCTION
            $this->conn->setQuery("UPDATE device_config SET start_mode = 0");
            return 0;
        } else if ($result === 1) {                                                                                     // FIRST RUN OR NORMAL
            $new_updated = $this->system->getIsDeviceUpdated();
            if($new_updated === 1) {
                $this->getWiringsFromServer($uid);
                $path = "/usr/share/apache2/default-site/htdocs/configuration/device_settings.ini";
                exec('sudo chmod 777 '. $path,$result,$retval);
                $content = "UPDATED = 0";
                $handle = fopen($path, 'w+');
                fwrite($handle, $content);
                fclose($handle);
            }
            $sn = $this->conn->getItem("SELECT serial_number from device_config");
            if($sn === '') {
                $config = $this->getSN($uid);
                if(!empty($config)) {
                    if ($config[1] === true) {
                        $this->conn->setQuery("UPDATE timing SET s1uptime = current_timestamp,
                                                                       uptime = current_timestamp,
                                                                       s2uptime = current_timestamp,
                                                                       s2hold = false, 
                                                                       s3hold = false,
                                                                       alerts = false,
                                                                       set_backlight_time = current_timestamp,
                                                                       wiring_check_interval = 10,
                                                                       wiring_check_timestamp = current_timestamp,
                                                                       contractor_info_interval = 1,
                                                                       contractor_info_timestamp = '2023-01-01 00:00:00',
                                                                       info_update_interval = 15,
                                                                       info_update_timestamp = current_timestamp,
                                                                       soft_update_timestamp = '2023-01-01 00:00:00',
                                                                       fan_time = current_timestamp - interval '5 minute',
                                                                       start_fan_timing = 0;
                                                     UPDATE current_stage SET timestamp = current_timestamp;
                                                     UPDATE device_config SET serial_number = '{$config[0]}',start_mode = 1",true);
                        return 1;                                                                                           // NORMAL mode
                    }
                }
                $this->setDefaultValues($uid);
                //$this->getWiringsFromServer($uid);
                //$this->updateWiring();
                $this->conn->setQuery("UPDATE device_config SET start_mode = 2");
                return 2;
            } else {
                return 1;
            }
        } else {
            self::setAlert('alert', 0, 17, 0, 'System:', 'Unknown system mode.', 'system');
            $this->conn->setQuery("UPDATE device_config SET start_mode = 0");
            return 0;
        }
    }

    /**
     * Get wirings from server
     * @param string $uid
     * @return array
     * @throws Exception
     */
    public function getWiringsFromServer(string $uid) {
        $data = json_encode(
            [
                'request' =>
                    [
                        'class' => 'sync',
                        'method' => 'getWirings',
                        'params' => [$uid]
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
        $result = $this->sync->sendCurlRequest($data);
        if(!empty($result)) {
            $query = '';
            for($i = 0; $i < count($result); $i++) {
                $result[$i]->type = (int)($result[$i]->type === 't');
                $query .= "UPDATE wirings SET type = '{$result[$i]->type}' WHERE alias = '{$result[$i]->alias}';";
            }
            $this->conn->setQuery($query);
        }

    }

    /**
     * Set backlight rgba with type and state
     * @param int $R
     * @param int $G
     * @param int $B
     * @param int $type // 0 - on, 1 - blink, 2 - fast blink
     * @param bool $state // on/off
     * @return bool
     * @throws Exception
     */
    public function setBacklight(int $R, int $G, int $B, int $type, bool $state): bool
    {
        $state = (int)$state;
        return (bool)$this->conn->setQuery("UPDATE device_config 
                                                  SET backlight_rgb = ARRAY[{$R},{$G},{$B}],
                                                      backlight_type = {$type},
                                                      backlight_status = '{$state}'
                                                  WHERE true;")['result'];
    }

    /**
     * Get backlight
     * @return array             // [R,G,B,type,state]
     * @throws Exception
     */
    public function getBacklight(): array
    {
        $backlight = $this->conn->getRow("SELECT backlight_rgb,
                                                       backlight_type,
                                                       backlight_status
                                                FROM device_config");
        $backlight_rgb = explode(",", $backlight['backlight_rgb']);
        $backlight_result[] = (int)trim($backlight_rgb[0], '{');
        $backlight_result[] = (int)$backlight_rgb[1];
        $backlight_result[] = (int)rtrim($backlight_rgb[2], '}');
        $backlight_result[] = (int)$backlight['backlight_type'];
        $backlight_result[] = (bool)($backlight['backlight_status'] === 't');
        return $backlight_result;
    }

    /**
     * Set alert
     * @param string $type
     * @param int $sensor_id
     * @param int $error_code
     * @param int $level
     * @param string $name
     * @param string $text
     * @param string $from
     * @return mixed
     * @throws Exception
     */
    public function setAlert(string $type, int $sensor_id, int $error_code, int $level, string $name, string $text, string $from)
    {
        if ($from != 'contractor') {
            $status = (int)$this->conn->getItem("SELECT
                                                        CASE
                                                            WHEN EXTRACT(Day FROM (current_timestamp - (SELECT timestamp
                                                                                                        FROM alerts   
                                                                                                        WHERE status = false AND error_code = '{$error_code}'
                                                                                                            AND from_id = (SELECT from_id FROM alerts_from WHERE alias = '{$from}')
                                                                                                        ORDER BY timestamp DESC
                                                                                                        LIMIT 1))) >= 1 THEN 1
                                                            ELSE 0
                                                        END
                                                   FROM timing");

            $alerts_count = (int)$this->conn->getItem("SELECT count(*) FROM alerts");
            if ($alerts_count === 0) {
                $status = 1;
            }
        } else {
            $status = 1;
        }
        if($status === 0) {
            return $this->conn->setQuery("INSERT INTO alerts(type_id,sensor_id,error_code,level,name,text,from_id,status,timestamp,is_sent)
                                            VALUES((SELECT type_id FROM alert_types WHERE alias = '{$type}' LIMIT 1),
                                                   '{$sensor_id}',
                                                   '{$error_code}',
                                                   '{$level}',
                                                   '{$name}',
                                                   '{$text}',
                                                   (SELECT from_id FROM alerts_from WHERE alias = '{$from}' LIMIT 1),
                                                   '{$status}',
                                                    current_timestamp,
                                                    false)")['result'];
        }
        return true;
    }

    /**
     * Get actual wiring from db without system request
     * @return array
     * @throws Exception
     */
    public function getActualWiring(): array
    {
        $wirings = $this->conn->getTable("SELECT type,alias FROM wirings ORDER BY id");
        $all_wirings = [];
        for ($i = 0; $i < count($wirings); $i++) {
            if ($wirings[$i]['type'] === 't') {
                $all_wirings[] = true;
            } else {
                $all_wirings[] = false;
            }
        }
        return $all_wirings;
    }

    /**
     * Get and order wirings
     * @return array|object                          // [R,C,G,Y1,Y2,ACC2,ACC1P,ACC1N,W1,W2,W3,OB]
     * @throws Exception
     */
    public function getWiring()
    {
        $all_wirings = [];
        $this->conn->setQuery("UPDATE device_config SET wiring_check = true");
        sleep(3);
        $this->conn->setQuery("UPDATE device_config SET wiring_check = false");

        $wirings = $this->conn->getTable("SELECT type FROM wirings ORDER BY id");
        $wirings_temp = $this->conn->getTable("SELECT type,alias FROM wirings_temp ORDER BY id");
        if(empty($wirings) || empty($wirings_temp)) {                                                                   // Empty wiring list
            return (object)["type" => 5, "message" => "Empty wiring list", "result" => false];
        }
        for($i = 0; $i < count($wirings); $i++) {
            if($wirings[$i]['type'] !== $wirings_temp[$i]['type']) {
                $this->setAlert('alert', 0, 21, 0, 'Wiring:', 'Wiring not same', 'system');
            }
            $all_wirings[] = ($wirings[$i]['type'] === 't');
        }
        //$this->updateWiring();
        $wire = [];
        for($i=0; $i<count($wirings_temp); $i++){
            $wire[$wirings_temp[$i]['alias']] = ($wirings_temp[$i]['type']==='t');
        }
//        if($wire['o/b'] && !$wire['y1']){                                                                               // NO Y1 WITH O/B
//            return (object)["type" => 5, "message" => "NO Y1 WITH O/B", "result" => false];
//        }
//        if(!$wire['o/b'] && !$wire['y1'] && !$wire['w1']){                                                              // NO Y1 NO W1 NO O/B
//            return (object)["type" => 5, "message" => "NO Y1 NO W1 NO O/B", "result" => false];
//        }
        if(!$wire['y1'] && $wire['y2']){                                                                                // NO Y1 ON Y2
            $this->conn->setQuery("UPDATE wirings SET type = false WHERE alias = 'y2'");
        }
        if(!$wire['w1'] && ($wire['w2'] || $wire['w3'])){                                                               // NO W1 ON W2 OR W3
            $this->conn->setQuery("UPDATE wirings SET type = false WHERE alias = 'w2' OR alias = 'w3'");
        }
//        if($wire['acc1p']){                                                                                             // ACC 1P ON
//            return (object)["type" => 5, "message" => "ACC 1P ON", "result" => false];
//        }
//        if($wire['acc1n'] && $wire['acc2']) {                                                                           // ACC1N ON ACC2 ON
//            return (object)["type" => 5, "message" => "ACC1N ON ACC2 ON", "result" => false];
//        }


        return $all_wirings;
    }

    /**
     * Check if device has client
     * @return bool
     * @throws Exception
     */
    public function checkClient() {
        $uid = $this->conn->getItem("SELECT uid FROM device_config");
        $config = $this->getSN($uid);
        if(!empty($config)) {
            if ($config[1] === true) {
                $this->sync->changeContractorInfo($config[0]);
                $this->conn->setQuery("UPDATE device_config SET serial_number = '{$config[0]}'");
                return true;
            } else {
                return false;
            }
        }
        return false;
    }

    /**
     * Get settings
     * @return array
     * @throws Exception
     */
    public function getSettings(): array
    {
        $settings = $this->conn->getRow("SELECT brightness,
                                                speaker,
                                                time,
                                                measure_id                                  AS measure,
                                                (SELECT brightness_mode FROM device_config) AS adaptive
                                          FROM settings");
        $settings['brightness'] = (int)$settings['brightness'];
       // $settings['system_delay'] = (int)$settings['system_delay'];
        $settings['speaker'] = (int)$settings['speaker'];
        $settings['time'] = (int)$settings['time'];
        $settings['measure'] = (int)$settings['measure'];
        $settings['adaptive'] = ((int)$settings['adaptive'] === 1);
        return $settings;
    }

    /**
     * Change brightness
     * @param int $brightness
     * @param bool $adaptive_brightness // true - auto, false - manual
     * @return bool
     * @throws Exception
     */
    public function setBrightness(int $brightness, bool $adaptive_brightness): bool
    {
        $brightness = (int)round($brightness);
        $brightness_mode = (int)$adaptive_brightness;
        exec("timeout " . self::EXEC_TIMEOUT_INTERVAL . " " . 'sudo ' . self::WRAP . "setBrightness " . $brightness, $result, $retval);
        if ((int)$result[0] === 0) {
            return $this->conn->setQuery("UPDATE settings SET brightness = '{$brightness}',is_sent = false; 
                                                UPDATE device_config SET brightness = '{$brightness}', brightness_mode = '{$brightness_mode}'", true)["result"];
        } else {
            return false;
        }
    }

    /**
     * Set all settings
     * @param int $brightness
     * @param int $speaker
     * @param int $temp
     * @param int $time
     * @param bool $reset
     * @param bool $adaptive_brightness // true - auto, false - manual
     * @param int $system_delay
     * @return bool
     * @throws Exception
     */
    public function setSettings(int $brightness, int $speaker, int $temp, int $time, bool $reset, bool $adaptive_brightness): bool
    {
        if ($reset === true) {
            $brightness = 100;
            $speaker = 50;
            $temp = 1;                                // measure to fahrenheit
            $time = 0;                                // time type to 12
        }
        $brightness_mode = (int)$adaptive_brightness;
        return (bool)$this->conn->setQuery("UPDATE settings      SET brightness = '{$brightness}',speaker = '{$speaker}',
                                                                           measure_id = '{$temp}',time = '{$time}',is_sent = false;
                                                  UPDATE device_config SET brightness = '{$brightness}',
                                                                           brightness_mode = '{$brightness_mode}'", true)['result'];
    }

    /**
     * Get Luminosity
     * @param int $brightness
     * @param bool $adaptive_brightness
     * @return false|float|int
     * @throws Exception
     */
    public function getLuminosity(int $brightness,bool $adaptive_brightness)
    {
        $this->setBrightness($brightness,$adaptive_brightness);
        exec("timeout " . self::EXEC_TIMEOUT_INTERVAL . " " . 'sudo ' . self::WRAP . "Luminosity.out", $result, $retval);
        if (empty($result) || (int)$result[0] === -1) {
            return false;
        } else {
            return (int)$result[0] / 10;
        }
    }
}