<?php

namespace project;
//ini_set('display_errors', '1');
//ini_set('display_startup_errors', '1');
//error_reporting(E_ALL);

use core\db\Z_PgSQL;
use Exception;

require_once "/usr/share/apache2/default-site/htdocs/engine/core/db/PgSQL.php";
require_once "/usr/share/apache2/default-site/htdocs/engine/project/System.php";
require_once "/usr/share/apache2/default-site/htdocs/engine/project/Relay.php";

/**
 * Scheme work logic
 * @package project
 */
class Scheme
{
    private $conn;
    private $relay;
    private const ET = 40;               // 4.5 C
    private const ET_STAGE2 = 3.5;
    private const HPT = 60;                // 15 C
    private const HUM_MAX = 70;
    private const HUM_MIN = 20;
    private const HUM_STEP = 10;
    private const STAGE0_RANGE = 0;
    private const STAGE1_ON_RANGE = 1.9;
    private const STAGE2_ON_RANGE = 2.9;
    private const STAGE3_ON_RANGE = 5.9;
    private const STAGE1_OFF_RANGE = 1;
    private const STAGE2_OFF_RANGE = 1.9;
    private const STAGE3_OFF_RANGE = 4.9;
    private const ALERT_TIME = 120;
    private const CHANGE_STAGE_TIME = 40;
    private const CHANGE_STAGE_TIME_WO_OB = 10;
    private const S2OFF_TIME = 2;


    /**
     * @throws Exception
     */
    public function __construct()
    {
        $this->conn = Z_PgSQL::connection();
    }

    /**
     * Humidifier-dehumidifier working
     * @param int $humidifier
     * @param string $device_state
     * @param int $humidity
     * @param int $current_humidity
     * @param int $sth
     * @param int $stl
     * @return void
     * @throws Exception
     */
     // Not used
    private function startHumidifierWork(int $humidifier, string $device_state, int $humidity, int $current_humidity, int $sth, int $stl)
    {

    }

    /**
     * Get current state
     * @return array
     * @throws Exception
     */
    private static function getCurrentState(): array
    {
        // What is the diffrence between current_temp (ct) and temp (sp)

        $current_state = (new Scheme)->conn->getRow("SELECT current_temp                                          AS ct,
                                                                  mode.alias                                            AS mode,
                                                                  state_id,
                                                                  COALESCE(state_next_check::varchar,'')                AS state_next_check,
                                                                  temp                                                  AS sp,
                                                                  (SELECT humidifier_id FROM device_config)             AS humidifier_id,
                                                                  hold_status,
                                                                  humidity,
                                                                  current_humidity,
                                                                  current_timestamp                                     AS date,
                                                                  (SELECT max_humidity FROM vacation)                   AS sth,
                                                                  (SELECT min_humidity FROM vacation)                   AS stl,
                                                                  (SELECT max_temp FROM vacation)                       AS max_temp,
                                                                  (SELECT min_temp FROM vacation)                       AS min_temp,
                                                                  (SELECT 
                                                                        CASE 
                                                                            WHEN (SELECT is_enable FROM vacation) = true THEN 'vacation'
                                                                            ELSE 'normal'
                                                                        END)                                            AS device_state,
                                                                  fan,
                                                                  (SELECT EXTRACT(MINUTE from (current_timestamp - fan_time)) FROM timing) AS fan_time,
                                                                  (SELECT system_type FROM device_config)                                  AS system_type
                                                                   --EXTRACT(MINUTE FROM current_timestamp)               AS cur_minutes
                                                           FROM current_state
                                                                LEFT JOIN mode ON current_state.mode_id = mode.mode_id");
        $current_state['humidifier_id'] = (int)$current_state['humidifier_id']; // convert to int
        if ($current_state['hold_status'] === 't') {
            $current_state['hold_status'] = true; // convert to bool
        } else {
            $current_state['hold_status'] = false;
            if ($current_state['mode'] !== 'off' &&
            $current_state['state_next_check'] <= $current_state['date'] &&
            $current_state['state_next_check'] !== '') {                                               // if is schedule

                //$schedule = (new System)->checkCurrentSchedule($current_state['state_next_check']);
                $schedule = (new System)->checkCurrentSchedule('');
                if (!empty($schedule)) {
                    $current_state['sp'] = $schedule['temp'];
                    if((int)$current_state['system_type'] !== 3 && (int)$current_state['system_type'] !== 4) { // isn't cool or heat only
                        $current_state['mode'] = 'auto';
                    }
                    (new Scheme)->conn->setQuery("UPDATE current_state SET state_id = 2,is_sent = false");
                }
            }
        }
        return $current_state;
    }

    /**
     * Get s2hold
     * @return bool
     */
     // Not Used
    public function getS2Hold()
    {
        return ($this->conn->getItem("SELECT s2hold FROM timing") === 't');
    }

    /**
     * Get s3hold
     * @return bool
     */
     // Not Used
    public function getS3Hold()
    {
        return ($this->conn->getItem("SELECT s3hold FROM timing") === 't');
    }

    // Not Used
    public function setS2Hold(bool $type)
    {
        $type = (int)$type;
        return (bool)$this->conn->setQuery("UPDATE timing SET s2hold = '{$type}'")["result"];
    }

    /**
     *
     * @return bool
     * @throws Exception
     */
     // Not Used
    public function setS3Hold(bool $type)
    {
        $type = (int)$type;
        return (bool)$this->conn->setQuery("UPDATE timing SET s3hold = '{$type}'")["result"];
    }

    public function getS2OffTime()
    {
        $s2offtime = self::S2OFF_TIME; // 2
        // If the time difference in minutes is greater than the value in $s2offtime, it returns 1.
        // Otherwise, it returns 0.

        //INSERT INTO current_stage(mode,stage,timestamp,blink_mode,s2offtime)
        //VALUES(0, 0, current_timestamp, 0, current_timestamp - interval '5 minute');
        return (int)$this->conn->getItem("SELECT 
                                                        CASE 
                                                            WHEN EXTRACT(epoch FROM current_timestamp - s2offtime)/60 > {$s2offtime} THEN 1
                                                            ELSE 0
                                                        END
                                                 FROM current_stage");
    }

    public function setS2OffTime()
    {
        return (bool)$this->conn->setQuery("UPDATE current_stage SET s2offtime = current_timestamp");
    }

    // Not use
    public function setS2UpTime()
    {
        // s1uptime: update in runDevice and getStartMode (hardware class) - startTempTimer in Relay and used, update setMode in System
        return (bool)$this->conn->setQuery("UPDATE timing SET s2uptime = current_timestamp");
    }

    // Not use
    public function setS1UpTime()
    {
        return (bool)$this->conn->setQuery("UPDATE timing SET s1uptime = current_timestamp");
    }

    // Not used
    public function getAlerts()
    {
        return ($this->conn->getItem("SELECT alerts FROM timing") === 't');
    }

    // Not used
    public function setAlerts(bool $type)
    {
        $type = (int)$type;
        return $this->conn->setQuery("UPDATE timing SET alerts = '{$type}'");
    }

    // update timestamp
    public function setDelayTime()
    {
        return (bool)$this->conn->setQuery("UPDATE current_stage SET timestamp = current_timestamp")['result'];
    }

    public function getDelayTime()
    {
        $delay = (int)$this->conn->getItem("SELECT system_delay FROM settings");
        return (int)$this->conn->getItem("SELECT 
                                                        CASE 
                                                            WHEN EXTRACT(epoch FROM current_timestamp - timestamp)/60 > '{$delay}' THEN 1
                                                            ELSE 0
                                                        END
                                                FROM current_stage");
    }

    // Use for each mode
    public function setModeBlink(bool $type)
    {
        $type = (int)$type;
        return (bool)$this->conn->setQuery("UPDATE current_stage SET blink_mode = '{$type}'")['result'];
    }

    // Not used
    public function setBacklightTime()
    {
        return (bool)$this->conn->setQuery("UPDATE timing SET set_backlight_time = current_timestamp")['result'];
    }

    /**
     * Start work
     * @throws Exception
     */
    public function startWork()
    {
        // Relay usage:  getCoolingMaxStage, getHeatingMaxStage, ...
        $this->relay = new Relay();

        $current_state = self::getCurrentState();
        $current_temp = $current_state['ct'] * 9 / 5 + 32;
        $current_mode = $this->relay->getModeName(); // off, cooling, emergency, heating
        $current_stage = $this->relay->getCurrentStage(); // stage, 0, 1, 2,3 / update in updateStates()

        $set_temp = $current_state['sp'] * 9 / 5 + 32;
        $set_mode = $current_state['mode'];
        $set_stage = 0;

        $s2hold = $this->getS2Hold(); // Not Used
        $s3hold = $this->getS3Hold(); // Not Used

        $stage_1_off_time = (int)$this->getDelayTime(); // set at the end of this function and then use at the next iteration
        $stage_2_off_time = (int)$this->getS2OffTime();

        $real_set_mode = $set_mode;

        $range = $current_temp - $set_temp; // current_temp (average temperature) - temp (sp)
        if ($real_set_mode === 'auto') {
            if ($current_mode === 'cooling') {
                if ($current_temp > $set_temp - self::STAGE1_OFF_RANGE) { // before stage 1 off
                    $real_set_mode = 'cooling';
                } elseif ($current_temp > $set_temp - self::STAGE1_ON_RANGE) { // before stage 1 on
                    $real_set_mode = 'off';
                } else {  // stage 1 on
                    $real_set_mode = 'heating';
                }
            } elseif ($current_mode === 'heating') {
                if ($current_temp < $set_temp + self::STAGE1_OFF_RANGE) { // before stage 1 off
                    $real_set_mode = 'heating';
                } elseif ($current_temp < $set_temp + self::STAGE1_ON_RANGE) { // before stage 1 on
                    $real_set_mode = 'off';
                } else {  // stage 1 on
                    $real_set_mode = 'cooling';
                }
            } else { // OFF
                if ($current_temp < $set_temp - self::STAGE1_ON_RANGE) {
                    $real_set_mode = 'heating';
                } elseif ($current_temp > $set_temp + self::STAGE1_ON_RANGE) {
                    $real_set_mode = 'cooling';
                }
            }
        } elseif ($real_set_mode === 'vacation') {
            if ($current_mode === 'cooling') {
                if ($current_temp > $set_temp - self::STAGE1_OFF_RANGE) { // before stage 1 off
                    $real_set_mode = 'cooling';
                } elseif ($current_temp > $set_temp - self::STAGE1_ON_RANGE) { // before stage 1 on
                    $real_set_mode = 'off';
                } else {  // stage 1 on
                    $real_set_mode = 'heating';
                }
            } elseif ($current_mode === 'heating') {
                if ($current_temp < $set_temp + self::STAGE1_OFF_RANGE) { // before stage 1 off
                    $real_set_mode = 'heating';
                } elseif ($current_temp < $set_temp + self::STAGE1_ON_RANGE) { // before stage 1 on
                    $real_set_mode = 'off';
                } else {  // stage 1 on
                    $real_set_mode = 'cooling';
                }
            } else { // OFF
                if ($current_temp < $set_temp - self::STAGE1_ON_RANGE) {
                    $real_set_mode = 'heating';
                } elseif ($current_temp > $set_temp + self::STAGE1_ON_RANGE) {
                    $real_set_mode = 'cooling';
                }
            }

            if ($current_state['min_temp'] > $current_temp) {
                $real_set_mode = 'heating';
                $range = $current_temp - $current_state['min_temp'];
            } elseif ($current_state['max_temp'] < $current_temp) {
                $real_set_mode = 'cooling';
                $range = $current_temp - $current_state['max_temp'];
            }
        }

        if ($real_set_mode === 'heating') {
            if ($current_temp < self::ET) {
                $real_set_mode = 'emergency';
                $set_stage = 1;
                if ($current_temp < self::ET - self::ET_STAGE2) {
                    $set_stage = 2;
                }
            }
        }

        if ($real_set_mode !== $current_mode) { // mode changes
            $current_stage = 0;
        }

        if ($real_set_mode === 'heating') {
            if ($current_stage === 0) {
                if ($range < 0 && abs($range)>self::STAGE1_ON_RANGE &&
                $this->relay->getHeatingMaxStage() >= 1) { // calculate based on o/b, y1 , y2, w1, w2 and w3 in relay
                    $set_stage = 1;
                    if (abs($range) > self::STAGE2_ON_RANGE && $this->relay->getHeatingMaxStage() >= 2) {
                        $set_stage = 2;
                        if (abs($range) > self::STAGE3_ON_RANGE && $this->relay->getHeatingMaxStage() >= 3) {
                            $set_stage = 3;
                        }
                    }
                }
            } elseif ($current_stage === 1) {
                if ($range < self::STAGE1_OFF_RANGE && $this->relay->getHeatingMaxStage() >= 1) {
                    $set_stage = 1;
                    if (abs($range) > self::STAGE2_ON_RANGE && $this->relay->getHeatingMaxStage() >= 2) {
                        $set_stage = 2;
                        if (abs($range) > self::STAGE3_ON_RANGE && $this->relay->getHeatingMaxStage() >= 3) {
                            $set_stage = 3;
                        }
                    }
                }
            } elseif ($current_stage === 2) {
                if ($range < self::STAGE1_OFF_RANGE && $this->relay->getHeatingMaxStage() >= 1) {
                    $set_stage = 1;
                    if (abs($range) > self::STAGE2_OFF_RANGE && $this->relay->getHeatingMaxStage() >= 2) {
                        $set_stage = 2;
                        if (abs($range) > self::STAGE3_ON_RANGE && $this->relay->getHeatingMaxStage() >= 3) {
                            $set_stage = 3;
                        }
                    }
                }
            } elseif ($current_stage === 3) {
                if ($range < self::STAGE1_OFF_RANGE && $this->relay->getHeatingMaxStage() >= 1) {
                    if (abs($range) > self::STAGE2_OFF_RANGE && $this->relay->getHeatingMaxStage() >= 2) {
                        $set_stage = 2;
                        if (abs($range) > self::STAGE3_OFF_RANGE && $this->relay->getHeatingMaxStage() >= 3) {
                            $set_stage = 3;
                        }
                    }
                }
            }
        } elseif ($real_set_mode === 'cooling') {
            if ($current_stage === 0) {
                if ($range>0 && abs($range) > self::STAGE1_ON_RANGE && $this->relay->getCoolingMaxStage() >= 1) {
                    $set_stage = 1;
                    if (abs($range) > self::STAGE2_ON_RANGE && $this->relay->getCoolingMaxStage() >= 2) {
                        $set_stage = 2;
                        if (abs($range) > self::STAGE3_ON_RANGE && $this->relay->getCoolingMaxStage() >= 3) {
                            $set_stage = 3;
                        }
                    }
                }
            }elseif ($current_stage === 1){
                if (($range>0 || abs($range) < self::STAGE1_OFF_RANGE) && $this->relay->getCoolingMaxStage() >= 1) {
                    $set_stage = 1;
                    if (abs($range) > self::STAGE2_ON_RANGE && $this->relay->getCoolingMaxStage() >= 2) {
                        $set_stage = 2;
                        if (abs($range) > self::STAGE3_ON_RANGE && $this->relay->getCoolingMaxStage() >= 3) {
                            $set_stage = 3;
                        }
                    }
                }
            }elseif ($current_stage === 2){
                if (($range>0 || abs($range) < self::STAGE1_OFF_RANGE) && $this->relay->getCoolingMaxStage() >= 1) {
                    $set_stage = 1;
                    if (abs($range) > self::STAGE2_OFF_RANGE && $this->relay->getCoolingMaxStage() >= 2) {
                        $set_stage = 2;
                        if (abs($range) > self::STAGE3_ON_RANGE && $this->relay->getCoolingMaxStage() >= 3) {
                            $set_stage = 3;
                        }
                    }
                }
            }elseif ($current_stage === 3){
                if (($range>0 || abs($range) < self::STAGE1_OFF_RANGE) && $this->relay->getCoolingMaxStage() >= 1) {
                    $set_stage = 1;
                    if (abs($range) > self::STAGE2_OFF_RANGE && $this->relay->getCoolingMaxStage() >= 2) {
                        $set_stage = 2;
                        if (abs($range) > self::STAGE3_OFF_RANGE && $this->relay->getCoolingMaxStage() >= 3) {
                            $set_stage = 3;
                        }
                    }
                }
            }
        }

        // check timeout
        $is_blink = false;
        $real_set_stage = $set_stage;
        if ((int)$stage_2_off_time === 0 && $set_stage > 1) {
            $real_set_stage = 1;
        }
        if ((int)$stage_1_off_time === 0 && $set_stage > 0) {
            $real_set_stage = 0;
        }

        if (($real_set_mode !== $current_mode && $current_mode!=='off') || $set_stage > $current_stage) { // start stage 0 mode change
            if ($stage_1_off_time === 1 || $current_stage > 0) { // start stage 1
                $real_set_stage = 1;
                if ($set_stage > 1 && ($stage_2_off_time === 1 || $current_stage > 1)) { // start stage 2 or 3
                    $real_set_stage = $set_stage;
                } else { // wait stage 2 timer
                    $is_blink = true;
                }
            } else { // wait stage 1 timer
                $real_set_stage = $current_stage;
                $is_blink = true;
            }
        } else { // going down or stay current - without delay
            $real_set_stage = $set_stage;
        }
        if ($real_set_stage >= 1) {
            $this->setDelayTime(); // return bool but not catched
            if ($real_set_stage > 1) {
                $this->setS2OffTime();
            }
        }
        // set front blink
        $this->setModeBlink($is_blink);
        // set relay
        switch ($real_set_mode) {
            case 'heating':
                switch ($real_set_stage) {
                    case 0:
                        $this->relay->heatingStage0();
                        break;
                    case 1:
                        $this->relay->heatingStage1();
                        break;
                    case 2:
                        $this->relay->heatingStage2();
                        break;
                    case 3:
                        $this->relay->heatingStage3();
                        break;
                }
                break;
            case 'cooling':
                switch ($real_set_stage) {
                    case 0:
                        $this->relay->coolingStage0();
                        break;
                    case 1:
                        $this->relay->coolingStage1();
                        break;
                    case 2:
                        $this->relay->coolingStage2();
                        break;
                    case 3:
                        $this->relay->coolingStage3();
                        break;
                }
                break;
            case 'emergency':
                switch ($real_set_stage) {
                    case 1:
                        $this->relay->emergencyHeating1();
                        break;
                    case 2:
                        $this->relay->emergencyHeating2();
                        break;
                }
                break;
            case 'off':
            default:
                $this->relay->setAllOff();
                break;
        }


        //echo $real_set_mode.'-'.$real_set_stage.' '.$this->getDelayTime().'-'.$this->getS2OffTime();
        //$timer = $this->conn->getRow("SELECT timestamp, current_timestamp FROM  current_stage");
        //echo $timer['timestamp'].' '.$timer['current_timestamp'];

        // HUMIDIFIER - DEHUMIDIFIER
        if ($current_state['humidifier_id'] !== 3) {

            $humidifier = $current_state['humidifier_id'];
            $device_state = $current_state['device_state'];
            $humidity = $current_state['humidity'];
            $current_humidity = $current_state['current_humidity'];
            $sth = $current_state['sth'];
            $stl = $current_state['stl'];

            if ($device_state === 'normal') {
                $max_hum = self::HUM_MAX;
                $min_hum = self::HUM_MIN;
                if ($humidity + self::HUM_STEP < $max_hum) {
                    $max_hum = $humidity + self::HUM_STEP;
                }
                if ($humidity - self::HUM_STEP > $min_hum) {
                    $min_hum = $humidity - self::HUM_STEP;
                }
                if ($humidifier === 1) {                                                            // humidifier
                    if ($current_humidity < $humidity) {                      // on
                        $this->relay->humidifierOn();

                    } elseif ($current_humidity >= $max_hum) {           // off
                        $this->relay->humidifierOff();
                    }
                } else if ($humidifier === 2) {                                                      // dehumidifier
                    if ($current_humidity > $humidity) {
                        $this->relay->deHumidifierOn();
                    } elseif ($current_humidity <= $min_hum) {
                        $this->relay->deHumidifierOff();
                    }
                }
            } else if ($device_state === 'vacation') {
                if ($humidifier === 1) {
                    if ($current_humidity < $stl) {
                        $this->relay->humidifierOn();
                    } else {
                        $this->relay->humidifierOff();
                    }
                } else if ($humidifier === 2) {
                    if ($current_humidity > $sth) {
                        $this->relay->deHumidifierOn();
                        if ($current_humidity <= $stl) {
                            $this->relay->deHumidifierOff();
                        }
                    }
                }
            }
        }

        $this->relay->updateRelayState();
        $this->relay->fanWorkTime();
    }


    /**
     * Generate alert
     * @param string $name
     * @param string $text
     * @param int $error_code
     * @return void
     * @throws Exception
     */
     // Not used
    private function generateAlert(string $name, string $text, int $error_code)
    {
        (new Scheme)->conn->setQuery("INSERT INTO alerts(type_id, sensor_id, error_code, level, name, text, from_id, status,timestamp,is_sent) 
                                            VALUES (1,0,'{$error_code}',0,'{$name}','{$text}',(SELECT from_id FROM alerts_from WHERE alias = 'hardware'),true,current_timestamp,false)");
    }

}

//$obj = new Scheme();
//var_dump($obj->startWork());
