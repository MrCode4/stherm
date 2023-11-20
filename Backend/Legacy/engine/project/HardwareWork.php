<?php

namespace project;
use core\db\Z_PgSQL;
use project\Scheme;
use Exception;
require_once "/usr/share/apache2/default-site/htdocs/engine/core/db/PgSQL.php";
require_once "Scheme.php";


/**
 * Functional which called by hardware part
 * @package project
 */
class HardwareWork
{
    private $conn;

    /**
     * @throws Exception
     */
    public function __construct() {
        $this->conn = Z_PgSQL::connection();
    }

    /**
     * Set ventilator's value
     * @param int $value
     * @return void
     * @throws Exception
     */
    public function setVentilator(int $value) {
        return $this->conn->setQuery("UPDATE device_config SET ventilator = '{$value}'",true)['result'];
    }

    /**
     * Add new pending sensor
     * @param array $sensors
     * @return mixed
     * @throws Exception
     */
    public function addPendingSensor(array $sensors) {
        $query = '';
        for($i = 0; $i < count($sensors); $i++) {
            if((int)$sensors[$i]->external_sensor_type != 255) {
                $query .= "INSERT INTO sensors(type_id,is_main,sensor,is_paired,is_sent)
                           VALUES('{$sensors[$i]->external_sensor_type}',false,'{$sensors[$i]->external_sensor_id}',false,false)
                              ON CONFLICT (sensor) DO UPDATE SET type_id = '{$sensors[$i]->external_sensor_type}',
                                                                 is_remove = false,
                                                                 is_main = false,
                                                                 is_paired = false;";
            }
        }
        return $this->conn->setQuery($query,true)['result'];
    }

    /**
     * Get brightness mode
     * @return string
     */
    public function getBrightnessMode() {
        return $this->conn->getItem("SELECT brightness_mode FROM device_config");
    }

    /**
     * Get device's config
     * @return false|string
     * @throws Exception
     */
    public function getConfig() {
        $config = $this->conn->getTable("SELECT type_id           AS external_sensor_type,
                                                      poll_response     AS poll_if_responded,
                                                      poll_not_response AS poll_if_not_responded,
                                                      internal_poll_time
                                               FROM sensor_config
                                               WHERE is_external = true");

        for($i = 0; $i < count($config); $i++) {
            $config[$i]['external_sensor_type'] = (int)$config[$i]['external_sensor_type'];
            $config[$i]['poll_if_responded'] = (int)$config[$i]['poll_if_responded'];
            $config[$i]['poll_if_not_responded'] = (int)$config[$i]['poll_if_not_responded'];
            $config[$i]['internal_poll_time'] = (int)$config[$i]['internal_poll_time'];
        }
        $data = ['data' => $config];
        return json_encode($data);
    }

    /**
     * Get dynamic info // relay_state, pairing_mode, wiring_check, backlight, brightness_mode, last_update
     * @return false|string
     * @throws Exception
     */
    public function getDynamic() {
        (new Scheme)->startWork();
        $relays = $this->conn->getTable("SELECT type
                                               FROM relays
                                               ORDER BY order_id");
        for($i = 0; $i < count($relays); $i++) {
            if($relays[$i]['type'] === 't') {
                $relay_state[] = 1;
            } else {
                $relay_state[] = 0;
            }
        }
        //$relay_state = [0,0,0,0,0,0,0,0,0,0];
        $config = $this->conn->getRow("SELECT start_pairing,
                                                    wiring_check,
                                                    backlight_status, // Updated in setBacklight (Hardware class) (bool)
                                                    backlight_rgb,    // Updated in setBacklight (Hardware class) (ARRAY[{R},{G},{B}])
                                                    backlight_type,   // Updated in setBacklight (Hardware class) (int)
                                                    brightness_mode,
                                                    (SELECT 
                                                         CASE 
                                                             WHEN start_mode = 0 OR start_mode = 2 THEN true
                                                             WHEN EXTRACT(MINUTE FROM (current_timestamp - last_update)) > 5 THEN true
                                                             ELSE true
                                                         END AS browser_ok),
                                                    last_update,
                                                    scheme_backlight_rgb, // changed based on relay_state, updated in Relay.php and backlight() function
                                                    forget_sensor,
                                                    shut_down
                                             FROM device_config");

        $this->conn->setQuery("UPDATE timing SET wiring_check_timestamp = current_timestamp");

        if(!empty($config)) {
            if ($config['shut_down'] === 't') {
                $config['shut_down'] = true;
                $this->conn->setQuery("UPDATE device_config set shut_down = false");
            } else {
                $config['shut_down'] = false;
            }
            if ($config['start_pairing'] === 't') {
                $config['start_pairing'] = true;
            } else {
                $config['start_pairing'] = false;
            }
            if ($config['forget_sensor'] === 't') {
                $config['forget_sensor'] = true;
            } else {
                $config['forget_sensor'] = false;
            }

            if ($config['wiring_check'] === 't') {
                $config['wiring_check'] = true;
            } else {
                $config['wiring_check'] = false;
            }

            // When the scheme_backlight_result is not black ([0, 0, 0]),
            // employ the scheme_backlight_rgb as the backlight_result.
            $scheme_backlight_rgb = explode(",", $config['scheme_backlight_rgb']);
            $scheme_backlight_result[] = (int)trim($scheme_backlight_rgb[0], '{'); // Color: R
            $scheme_backlight_result[] = (int)$scheme_backlight_rgb[1]; // Color: G
            $scheme_backlight_result[] = (int)rtrim($scheme_backlight_rgb[2], '}'); // Color: B

            if ($scheme_backlight_result[0] === 0 &&
            $scheme_backlight_result[1] === 0 &&
            $scheme_backlight_result[2] === 0) {

                if ($config['backlight_status'] === 't') { // backlight_status is true

                    // Splits the string stored in $config['backlight_rgb'] into an array using the comma (,)
                    // as the delimiter and assigns it to the $backlight_rgb array.
                    // backlight_rgb data: {"r", "g", "b"}
                    $backlight_rgb = explode(",", $config['backlight_rgb']);

                    // Removes any leading opening brace {, converts the first element of the $backlight_rgb array
                    // (after trimming) to an integer, and adds it as a new element to the $backlight_result array.
                    $backlight_result[] = (int)trim($backlight_rgb[0], '{');
                    $backlight_result[] = (int)$backlight_rgb[1];
                    $backlight_result[] = (int)rtrim($backlight_rgb[2], '}');

                } else { // backlight_status is false
                    $backlight_result = [0, 0, 0];
                }

            } else {
                $backlight_result = $scheme_backlight_result;
            }

            // backlight_type Appended to the end of the array.
            $backlight_result[] = (int)$config['backlight_type'];

            $brightness_mode = (int)$config['brightness_mode'];

            if ($config['browser_ok'] === 't') {
                $browser_ok = true;
            } else {
                $browser_ok = false;
            }

            $data = ['relay_state' => $relay_state,
                'pairing_mode' => $config['start_pairing'],
                'wiring_check' => $config['wiring_check'],
                'backlight' => $backlight_result,
                'brightness_mode' => $brightness_mode,
                'browser_ok' => $browser_ok,
                'forget_sensor' => $config['forget_sensor'],
                'shut_down' => $config['shut_down'],
                ];
        } else {
            $data = ['relay_state' => $relay_state,
                'pairing_mode' => false,
                'wiring_check' => false,
                'backlight' => [0, 0, 0, 0], //["R", "G", "G", "backlight_type"]
                'brightness_mode' => 0,
                'browser_ok' => true,
                'forget_sensor' => false,
                'shut_down' => false,
                ];
        }
        //$this->conn->setQuery("UPDATE timing SET wiring_check_timestamp = current_timestamp");
        return json_encode($data);
    }

    /**
     * Get paired sensors
     * @return false|string
     * @throws Exception
     */
    public function getPaired() {
        $paired_sensors = $this->conn->getTable("SELECT sensor  AS external_sensor_id,
                                                              type_id AS external_sensor_type
                                                       FROM sensors
                                                       WHERE is_paired = true AND is_remove = false");

        for($i = 0; $i < count($paired_sensors); $i++) {
            $paired_sensors[$i]['external_sensor_type'] = (int)$paired_sensors[$i]['external_sensor_type'];
        }
        $data = ['data' => $paired_sensors];
        $this->conn->setQuery("UPDATE device_config SET forget_sensor = false");
        return json_encode($data);
    }

    /**
     * Get sensor's config
     * @return false|string
     * @throws Exception
     */
    public function getTreshold() {
        $config = $this->conn->getTable("SELECT type_id AS sensor_type,
                                                      min_alert_value,
                                                      max_alert_value
                                               FROM sensor_config
                                               WHERE is_external = false");

        for($i = 0; $i < count($config); $i++) {
            $config[$i]['sensor_type']     = (int)$config[$i]['sensor_type'];
            $config[$i]['min_alert_value'] = (int)$config[$i]['min_alert_value'];
            $config[$i]['max_alert_value'] = (int)$config[$i]['max_alert_value'];
        }
        $data = ['data' => $config];
        return json_encode($data);
    }

    /**
     * @TODO set texts and name
     * Set new alert
     * @return int|mixed
     * @throws Exception
     */
    public function setAlert($alert) {
        if(!empty($alert)) {
            $alert->error_code = (int)$alert->error_code;
            switch($alert->error_code) {
                case 1:
                case 2:
                     $name = "Temperature Sensor Issue:";
                     $text = "Sensor malfunction; inaccurate temperature data.";
                     $from_alias = 'system';
                     break;
                case 6:
                case 7:
                    $name = "Humidity Sensor Issue:";
                    $text = "Sensor malfunction; inaccurate humidity data.";
                    $from_alias = 'system';
                    break;
                case 8:
                    $name = "Pressure Sensor Issue:";
                    $text = "Sensor malfunction; inaccurate pressure data.";
                    $from_alias = 'system';
                    break;
                case 9:
                    $name = "High CO2 Level:";
                    $text = "High CO2 detected; please ventilate the room.";
                    $from_alias = 'system';
                    break;
                case 10:
                    $name = "Incorrect Wiring Connection:";
                    $text = "Wiring problem causing sensor malfunction.";
                    $from_alias = 'hardware';
                    break;
                default:
                    $name = 'Danger:';
                    $text = 'System is not working correctly.';
                    $from_alias = 'system';
            }

//            if((int)$alert->error_code === 10 || (int)$alert->error_code === 11) {
//                $from_alias = 'hardware';
//            } else {
//                $from_alias = 'system';
//            }
            $status = (int)$this->conn->getItem("SELECT
                                                            CASE
                                                                WHEN EXTRACT(Day FROM (current_timestamp - (SELECT timestamp
                                                                                                            FROM alerts
                                                                                                            WHERE status = false AND error_code = '{$alert->error_code}'
                                                                                                            AND from_id = (SELECT from_id FROM alerts_from WHERE alias = '{$from_alias}')
                                                                                                            ORDER BY timestamp DESC
                                                                                                            LIMIT 1))) >= 1 THEN 1
                                                                ELSE 0
                                                            END
                                                        FROM timing");

            $alerts_count = (int)$this->conn->getItem("SELECT count(*) FROM alerts");
            if ($alerts_count === 0) {
                $status = 1;
            }
            return $this->conn->setQuery("INSERT INTO alerts(type_id,sensor_id,error_code,level,from_id,status,name,text,timestamp,is_sent)
                                                VALUES('{$alert->level}',(SELECT sensor_id FROM sensors WHERE sensor = '{$alert->device_id}'),'{$alert->error_code}',0,
                                                (SELECT from_id FROM alerts_from WHERE alias = '{$from_alias}'),'{$status}','{$name}','{$text}',current_timestamp,false)")['result'];
        } else {
            return 0;
        }
    }

    /**
     * Set brightness data
     * @param $brightness
     * @return mixed
     * @throws Exception
     */
    public function setBrightnessData($brightness) {
        $set = $this->conn->setQuery("UPDATE device_config SET brightness = '{$brightness}';
                                            UPDATE settings      SET brightness = '{$brightness}',is_sent = false",true)['result'];
        return $set;
    }

    /**
     * Set sensor's data
     * @param $sensor_data
     * @return mixed
     * @throws Exception
     */
    public function setSensorData($sensor_data) {
        if(!empty($sensor_data) && isset($sensor_data->data)) {
            if (!isset($sensor_data->data->temperature)) {
                $sensor_data->data->temperature = null;
            } else {
                $sensor_data->data->temperature = $sensor_data->data->temperature / 10;
            }
            if (!isset($sensor_data->data->humidity)) {
                $sensor_data->data->humidity = null;
            }
            if (!isset($sensor_data->data->co2)) {
                $sensor_data->data->co2 = null;
            }
            if (!isset($sensor_data->data->pressure)) {
                $sensor_data->data->pressure = null;
            }
            if (!isset($sensor_data->data->tof)) {
                $sensor_data->data->tof = null;
            }
            if (!isset($sensor_data->data->ambiend)) {
                $sensor_data->data->ambiend = null;
            }
            if (!isset($sensor_data->data->tvoc)) {
                $sensor_data->data->tvoc = null;
            }
            if (!isset($sensor_data->data->etoh)) {
                $sensor_data->data->etoh = null;
            }
            if(isset($sensor_data->external_sensor_type)) {
                if($sensor_data->external_sensor_type == 1) {
                    $sensor_id = $this->conn->getItem("SELECT sensor_id FROM sensors
                                                             WHERE sensor = '{$sensor_data->external_sensor_id}' AND is_main = true
                                                             LIMIT 1");
                } else {
                    $sensor_id = $this->conn->getItem("SELECT sensor_id FROM sensors
                                                             WHERE sensor = '{$sensor_data->external_sensor_id}'
                                                             LIMIT 1");
                }
            } else {
                $sensor_id = 0;
            }
            $sensor_data->data->temperature = ($sensor_data->data->temperature - 32) * 5/9;
            $set = (int)$this->conn->setQuery("INSERT INTO sensor_logs(sensor_id,timestamp,temp,humidity,co2,pressure,tof,ambiend,tvoc,etoh,aiq,is_sent)
                                VALUES('{$sensor_id}', current_timestamp,
                                    NULLIF('{$sensor_data->data->temperature}','')::double precision,NULLIF('{$sensor_data->data->humidity}','')::double precision,
                                    NULLIF('{$sensor_data->data->co2}','')::double precision,NULLIF('{$sensor_data->data->pressure}','')::double precision,
                                    NULLIF('{$sensor_data->data->tof}','')::double precision, NULLIF('{$sensor_data->data->ambiend}','')::double precision,
                                    NULLIF('{$sensor_data->data->tvoc}','')::double precision,NULLIF('{$sensor_data->data->etoh}','')::double precision,
                                    '{$sensor_data->data->aiq}',false)")['result'];
            if ($set === 1) {
                $qa_test = $this->conn->getItem("SELECT qa_test FROM device_config");
                if($qa_test === 'f') {
                    return $this->conn->setQuery("UPDATE current_state
                                                        SET is_sent = false,
        current_temp = (SELECT avg(temp) FROM (SELECT temp
                                               FROM sensor_logs
                                                        LEFT JOIN sensors ON sensor_logs.sensor_id = sensors.sensor_id
                                               WHERE is_remove = false 
                                                 AND timestamp BETWEEN current_timestamp - interval '1 minute' AND current_timestamp
                                                 AND temp > (SELECT min_alert_value
                                                             FROM sensor_config
                                                                      LEFT JOIN sensor_types ON sensor_config.type_id = sensor_types.type_id
                                                             WHERE alias = 'temp' AND is_external = false) AND
                                                       temp < (SELECT max_alert_value
                                                               FROM sensor_config
                                                                        LEFT JOIN sensor_types ON sensor_config.type_id = sensor_types.type_id
                                                               WHERE alias = 'temp' AND is_external = false)
                                               GROUP BY sensor_logs.sensor_id,timestamp,sensor_logs.temp ORDER BY timestamp DESC
                                            ) AS temp),
        current_humidity = (SELECT avg(humidity) FROM (SELECT round(humidity) AS humidity
                                                       FROM sensor_logs
                                                                LEFT JOIN sensors ON sensor_logs.sensor_id = sensors.sensor_id
                                                       WHERE humidity > (SELECT min_alert_value
                                                                         FROM sensor_config
                                                                                  LEFT JOIN sensor_types ON sensor_config.type_id = sensor_types.type_id
                                                                         WHERE alias = 'hum' AND is_external = false) AND
                                                               humidity < (SELECT max_alert_value
                                                                           FROM sensor_config
                                                                                    LEFT JOIN sensor_types ON sensor_config.type_id = sensor_types.type_id
                                                                           WHERE alias = 'hum' AND is_external = false)
                                                         AND is_remove = false AND timestamp BETWEEN current_timestamp - interval '1 minute' AND current_timestamp
                                                       GROUP BY sensors.sensor_id,timestamp,sensor_logs.humidity
                                                       ORDER BY timestamp DESC
                                                       ) AS hum),
        co2_id = (SELECT
                        CASE
                            WHEN co2 <= 2.9 THEN (SELECT co2_id FROM co2 WHERE alias = 'good')
                            WHEN co2 >= 3 AND co2 <= 3.9 THEN (SELECT co2_id FROM co2 WHERE alias = 'moderate')
                            WHEN co2 >= 4 THEN (SELECT co2_id FROM co2 WHERE alias = 'poor')
                        END AS co2
                  FROM (SELECT avg(aiq::float) AS co2
                        FROM sensor_logs
                            LEFT JOIN sensors ON sensor_logs.sensor_id = sensors.sensor_id
                        WHERE is_remove = false AND timestamp BETWEEN current_timestamp - interval '1 minute' AND current_timestamp) AS co2);")['result'];
                } else {
                    return $set;
                }
            } else {
                return 0;
            }
        }
    }

    /**
     * Set wirings state
     * @param $wirings
     * @return mixed
     * @throws Exception
     */
    public function setWiring($wirings) {
        return true;
//        $wirings = substr($wirings,16);
//        $wirings = rtrim($wirings,']}"');
//        $wirings = trim($wirings,'[');
//        $wirings = explode(',',$wirings);
//
//        $query = "";
//        for($i = 0; $i < count($wirings); $i++) {
//            if($wirings[$i] == 1) {
//                $wirings[$i] = 0;
//            } else {
//                $wirings[$i] = 1;
//            }
//            if($i !== 0) {                                                          // no ob wiring
//                $query .= "UPDATE wirings_temp SET type = '{$wirings[$i]}' WHERE order_id = '{$i}';";
//            }
//            if($wirings[$i] === 0) {
//                $query .= "UPDATE relays SET type = '{$wirings[$i]}' WHERE order_id = '{$i}';";
//            }
//        }
//        $query .= "UPDATE wirings_temp SET type = true WHERE order_id = 0;";        // ob is always on
//        $query .= "UPDATE timing SET wiring_check_timestamp = current_timestamp;
//                   UPDATE device_config SET wiring_check = false;";
//
//        return $this->conn->setQuery($query,true)['result'];
    }

    /**
     * Set current wifi signal
     * @param int $signal
     * @return mixed
     * @throws Exception
     */
    public function setCurrentWifiSignal(int $signal) {
        if($signal != 0 && $signal != -1) {
            $signal = round(4 * $signal / 100);
            if ($signal == 4) {
                $signal = 3;
            }
            $signal = $signal + 1;
        }
        $this->conn->setQuery("UPDATE current_state SET wifi_status = '{$signal}'")['result'];
        return $signal;
    }

}
