<?php

namespace project;

use core\db\Z_PgSQL;
use Exception;

require_once "/usr/share/apache2/default-site/htdocs/engine/core/db/PgSQL.php";

/**
 * Synchronization with web server: contractor's info and other changed information
 * @package project
 */
class Sync
{
    private $conn;
    private const WEB_URL = "http://test.hvac.z-soft.am/engine/index.php";
    private const WEB_MAIN = "http://test.hvac.z-soft.am/";

    /**
     * @throws Exception
     */
    public function __construct()
    {
        $this->conn = Z_PgSQL::connection();
    }

    public function changeContractorInfo(string $serial_number) {
        $data = json_encode(
            [
                'request' =>
                    [
                        'class' => 'sync',
                        'method' => 'getContractorInfo',
                        'params' => [$serial_number]
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
            ]
        );
        $result = $this->sendCurlRequest($data);
        $query = "UPDATE timing SET contractor_info_timestamp = current_timestamp;";
        if (!empty($result)) {
            $is_service_titan = (int)$result->is_service_titan;
            $query .= "UPDATE device_config SET is_service_titan = '{$is_service_titan}',
                                                    phone = '{$result->phone}',
                                                    technical_access_link = '{$result->tech_link}',
                                                    timezone = '{$result->timezone->text}',
                                                    timezone_number = '{$result->timezone->number}',
                                                    url = '{$result->url}',
                                                    user_guide = '{$result->user_guide}',
                                                    contractor_name = '{$result->name}';";
            $file_name = basename($result->logotype);
            $dir = '/usr/share/apache2/default-site/htdocs/img/upload';
            if (!file_exists($dir)) {
                mkdir($dir, 0777, true);
            }
            if (file_put_contents('/usr/share/apache2/default-site/htdocs/img/upload/' . $file_name, file_get_contents(self::WEB_MAIN . $result->logotype))) {
                $img = 'img/upload/' . $file_name;
                $query .= "UPDATE device_config SET logo = '{$img}';";
            }
        }
        $this->conn->setQuery($query, true);
    }

    /**
     * Change Contractor info if time passed
     * @return void
     * @throws Exception
     */
    private function checkContractorInfoTime()
    {
        $configs = $this->conn->getRow("SELECT
                                                (SELECT serial_number FROM device_config)         AS serial_number,
                                                        CASE
                                                            WHEN EXTRACT(EPOCH FROM (current_timestamp - contractor_info_timestamp))/1440 >= contractor_info_interval THEN 1
                                                            ELSE 0
                                                        END                                       AS time_passed
                                                   FROM timing;");
        //var_dump($configs["time_passed"]);
        if ((int)$configs["time_passed"] === 1) {
            $this->changeContractorInfo($configs['serial_number']);
        }
    }

    /**
     * Send Curl request to web server
     * @param string $data
     * @return array
     */
    public function sendCurlRequest(string $data)
    {
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
            error_log("CURL error:" . $error_msg);
        }
       // return $data;
        //return $response;
        curl_close($curl);
        if (!isset($error_msg)) {
            error_log('no error message');
            if ($response) {
                error_log('and response');
                $result = json_decode($response)->result->result;
                if (json_last_error() === JSON_ERROR_NONE) {
                    return $result;
                }
                unset($result);
            }
        }
        return [];
    }

    /**
     * Send changed data to web server
     * @return array
     * @throws Exception
     */
    public function sendChangedDataToServer()
    {
        $query = '';
        $configs = $this->conn->getRow("SELECT
                                              (SELECT serial_number FROM device_config) AS serial_number,
                                              current_timestamp,
                                                 CASE
                                                    WHEN EXTRACT(EPOCH FROM (current_timestamp - info_update_timestamp)) >= info_update_interval THEN 1
                                                    ELSE 0
                                                 END AS update_time_passed
                                              FROM timing");
        if ((int)$configs['update_time_passed'] === 1) {
            $ip = $_SERVER['SERVER_ADDR'];

            //Schedules
            $schedules = $this->conn->getTable("SELECT schedule_id,
                                                             web_id,
                                                             name,
                                                             type_id,
                                                             start_time,
                                                             end_time,
                                                             is_enable,
                                                             temp,
                                                             json_build_array(d0,d1,d2,d3,d4,d5,d6) AS days,
                                                             is_remove,
                                                             is_sent
                                                       FROM schedules
                                                       WHERE is_sent is false");
            for ($i = 0; $i < count($schedules); $i++) {
                if ($schedules[$i]->is_remove === 't') {
                    $query .= "DELETE FROM schedules WHERE schedule_id = '{$schedules[$i]['schedule_id']}';";
                } else {
                    $query .= "UPDATE schedules SET is_sent = true WHERE schedule_id = '{$schedules[$i]['schedule_id']}';";
                }
            }

            // Sensors
            $sensors = $this->conn->getTable("SELECT sensor_id,
                                                           location_id,
                                                           name,
                                                           type_id,
                                                           is_main,
                                                           is_remove,
                                                           sensor
                                                     FROM sensors
                                                     WHERE is_sent = false AND is_paired = true");
            for ($i = 0; $i < count($sensors); $i++) {
                if ($sensors[$i]->is_remove === 't') {
                    $query .= "DELETE FROM sensors WHERE sensor_id = '{$sensors[$i]['sensor_id']}';";
                } else {
                    $query .= "UPDATE sensors SET is_sent = true WHERE sensor_id = '{$sensors[$i]['sensor_id']}';";
                }
            }

            // Sensor_logs
            $sensor_logs = $this->conn->getTable("SELECT log_id,sensor_id,timestamp,temp,humidity,co2,pressure,tof,ambiend,tvoc,etoh,aiq
                                                        FROM sensor_logs
                                                        WHERE is_sent = false");
            $slc = count($sensor_logs);
            error_log('count sensor logs= ' . strval($slc));
            for ($i = 0; $i < count($sensor_logs); $i++) {
                $query .= "DELETE FROM sensor_logs WHERE log_id = '{$sensor_logs[$i]['log_id']}';";
            }

            // Alerts
            $alerts = $this->conn->getTable("SELECT id,name,text,timestamp,error_code,status
                                                   FROM alerts
                                                   WHERE is_sent = false");
            for ($i = 0; $i < count($alerts); $i++) {
                if ($alerts[$i]->status === 't') {
                    $query .= "DELETE FROM alerts WHERE id = '{$alerts[$i]['id']}';";
                } else {
                    $query .= "UPDATE alerts SET is_sent = true WHERE id = '{$alerts[$i]['id']}';";
                }
            }

            // Vacation
            $vacation = $this->conn->getRow("SELECT min_temp,max_temp,min_humidity,max_humidity,is_enable 
                                                   FROM vacation WHERE is_sent = false");
            if (!empty($vacation)) {
                $query .= "UPDATE vacation SET is_sent = true;";
            }

            // Settings
            $settings = $this->conn->getRow("SELECT brightness,speaker,time,measure_id,system_delay,
                                                          (SELECT brightness_mode FROM device_config) AS brightness_mode
                                                   FROM settings
                                                   WHERE is_sent = false");
            if (!empty($settings)) {
                $query .= "UPDATE settings SET is_sent = true;";
            }

            // Main
            $main = $this->conn->getRow("SELECT temp,fan,mode_id,humidity,hold_status,co2_id,current_humidity,current_temp,state_id
                                               FROM current_state
                                               WHERE is_sent = false");
            if (!empty($main)) {
                $query .= "UPDATE current_state SET is_sent = true;";
                $main['hold_status'] = (int)($main['hold_status'] === 't');
            }

            // Wirings
            $wirings = $this->conn->getTable("SELECT id,name,type,order_id,alias FROM wirings WHERE alias != 'r' AND alias != 'c'");


            $data = json_encode(
                [
                    'request' =>
                        [
                            'class' => 'sync',
                            'method' => 'update',
                            'params' => [
                                $configs['serial_number'],
                                $schedules,
                                (object)$main,
                                $alerts,
                                (object)$vacation,
                                $sensor_logs,
                                $sensors,
                                (object)$settings,
                                $ip,
                                $wirings
                            ]
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
            $result = $this->sendCurlRequest($data);
            error_log('$data length= ' . strval(strlen($data)));
            $configs = null;
            $ip = null;
            $schedules = null;
            $sensors = null;
            $sensor_logs = null;
            $alerts = null;
            $vacation = null;
            $settings = null;
            $main = null;
            $wirings = null;
            $data = null;
            unset($configs, $ip, $schedules, $sensors, $sensor_logs, $alerts, $vacation, $settings, $main, $wirings, $data);
            if (!empty($result)) {
                error_log('Query executed sucessfully');
                $this->conn->setQuery($query, true);
                return $result;
            }
        }
        if ($slc > 0) {
            error_log('force query execution');
            $this->conn->setQuery($query, true);
        }
        unset($result);
        return false;
    }

    /**
     * Change data from server
     * @param $data
     * @return mixed
     * @throws Exception
     */
    private function changeDataFromServer($data)
    {
        $query = '';
        if ($data) {
            if (!empty($data->main)) {
                if ((int)$data->main->fan === 0) {
                    $fan_status = 0;
                } else {
                    $fan_status = 1;
                }
                $query .= "UPDATE current_state SET temp = '{$data->main->temp}',
                                                    fan = '{$data->main->fan}',
                                                    fan_status = '{$fan_status}',
                                                    hold_status = '{$data->main->hold_status}',
                                                    humidity = '{$data->main->humidity}',
                                                    mode_id = '{$data->main->mode_id}',
                                                    is_sent = true;";
            }
            if (!empty($data->vacation)) {
                $data->vacation->is_enable = (int)($data->vacation->is_enable === 't');
                $query .= "UPDATE vacation SET min_temp = '{$data->vacation->min_temp}',
                                               max_temp = '{$data->vacation->max_temp}',
                                               min_humidity = '{$data->vacation->min_humidity}',
                                               max_humidity = '{$data->vacation->max_humidity}',
                                               is_enable = '{$data->vacation->is_enable}',
                                               is_sent = true;";
            }
            if (!empty($data->settings)) {
                $data->settings->brightness_mode = (int)($data->settings->brightness_mode === 't');
                $query .= "UPDATE settings SET brightness = '{$data->settings->brightness}',
                                               speaker = '{$data->settings->speaker}',
                                               time = '{$data->settings->time}',
                                               measure_id = '{$data->settings->measure_id}',
                                               system_delay = '{$data->settings->system_delay}',
                                               is_sent = true;
                           UPDATE device_config SET brightness_mode = '{$$data->settings->brightness_mode}';";
            }
            if (isset($data->next_update)) {
                $query .= "UPDATE timing SET info_update_interval = '{$data->next_update}',info_update_timestamp = current_timestamp;";
            }

            for ($i = 0; $i < count($data->schedules); $i++) {
                $data->schedules[$i]->state = (int)($data->schedules[$i]->state === 't');
                $data->schedules[$i]->weekdays = ltrim($data->schedules[$i]->weekdays, '[');
                $data->schedules[$i]->weekdays = rtrim($data->schedules[$i]->weekdays, ']');
                $data->schedules[$i]->weekdays = explode(', ', $data->schedules[$i]->weekdays);
                if ((int)$data->schedules[$i]->device_schedule_id != 0 && $data->schedules[$i]->is_remove != 't') {
                    $query .= "UPDATE schedules SET name = '{$data->schedules[$i]->name}',
                                                    type_id = '{$data->schedules[$i]->type_id}',
                                                    start_time = '{$data->schedules[$i]->start_time}',
                                                    end_time = '{$data->schedules[$i]->end_time}',
                                                    is_enable = '{$data->schedules[$i]->state}',
                                                    temp = '{$data->schedules[$i]->temp}',
                                                    web_id = '{$data->schedules[$i]->schedule_id}',
                                                    is_sent = true,";
                    for ($j = 0; $j < count($data->schedules[$i]->weekdays); $j++) {
                        if ($data->schedules[$i]->weekdays[$j] === 'true') {
                            $data->schedules[$i]->weekdays[$j] = 1;
                        } else {
                            $data->schedules[$i]->weekdays[$j] = 0;
                        }
                        $query .= "d" . $j . "='{$data->schedules[$i]->weekdays[$j]}',";
                    }
                    $query = rtrim($query, ',');
                    $query .= " WHERE schedule_id = '{$data->schedules[$i]->device_schedule_id}';";
                } elseif ($data->schedules[$i]->is_remove != 't' && $data->schedules[$i]->is_sent === 'f') {
                    for ($j = 0; $j < count($data->schedules[$i]->weekdays); $j++) {
                        if ($data->schedules[$i]->weekdays[$j] === 'true') {
                            $data->schedules[$i]->weekdays[$j] = 1;
                        } else {
                            $data->schedules[$i]->weekdays[$j] = 0;
                        }
                    }
                    $query .= "INSERT INTO schedules(name,type_id,start_time,end_time,is_enable,temp,d0,d1,d2,d3,d4,d5,d6,web_id,is_sent,is_remove)
                               VALUES('{$data->schedules[$i]->name}','{$data->schedules[$i]->type_id}','{$data->schedules[$i]->start_time}',
                                      '{$data->schedules[$i]->end_time}','{$data->schedules[$i]->state}','{$data->schedules[$i]->temp}',
                                      '{$data->schedules[$i]->weekdays[0]}','{$data->schedules[$i]->weekdays[1]}','{$data->schedules[$i]->weekdays[2]}',
                                      '{$data->schedules[$i]->weekdays[3]}','{$data->schedules[$i]->weekdays[4]}','{$data->schedules[$i]->weekdays[5]}',
                                      '{$data->schedules[$i]->weekdays[6]}','{$data->schedules[$i]->schedule_id}',false,false);";
                } elseif ($data->schedules[$i]->is_remove == 't') {
                    $query .= "DELETE FROM schedules WHERE schedule_id = '{$data->schedules[$i]->schedule_id}';";
                }
            }

            for ($i = 0; $i < count($data->messages); $i++) {
                $query .= "INSERT INTO alerts(type_id,sensor_id,error_code,level,name,text,from_id,status,timestamp,is_sent)
                           VALUES(3,0,0,0,'{$data->messages[$i]->title}','{$data->messages[$i]->description}',3,true,'{$data->messages[$i]->created}',true);";
            }

        }
        $this->conn->setQuery($query, true);
        return $data;
    }

    /**
     * Start update all info
     * @return void
     * @throws Exception
     */
    public function startUpdate()
    {
        $this->checkContractorInfoTime();
        $web_data = $this->sendChangedDataToServer();
        $this->changeDataFromServer($web_data);
    }
}