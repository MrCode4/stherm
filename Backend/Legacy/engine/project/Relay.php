<?php

namespace project;

use core\db\Z_PgSQL;
use Exception;


/**
 * Relays working process
 * @package project
 */
class Relay
{
    private const COOLING_RGBM = [0, 128, 255, 1];
    private const HEATING_RGBM = [255, 68, 0, 1];
    private const EMERGENCY_RGBM = [255, 0, 0, 2];
    private const EMERGENCY_ST_RGBM = [255, 0, 0, 0];
    private const OFF_RGBM = [10, 186, 181, 1];
    /** @var Z_PgSQL */
    private $conn;
    /** @var string[] */
    private $relay = [
        'g' => 'no wire',
        'y1' => 'no wire',
        'y2' => 'no wire',
        'y3' => 'no wire',
        'acc2' => 'no wire',
        'w1' => 'no wire',
        'w2' => 'no wire',
        'w3' => 'no wire',
        'o/b' => 'no wire',
        'acc1p' => 'no wire',
        'acc1n' => 'no wire',
    ];
    /** @var string */
    private $before_state = 'off';
    private $current_state = 'off';
    private $current_stage = 0;
    private $hum_wiring;
    private $ob_state;

    /**
     * Constructor - Get info about current relay state from DB
     * @throws Exception
     */
    public function __construct()
    {
        $this->conn = Z_PgSQL::connection();
        $this->hum_wiring = $this->conn->getItem("SELECT hum_wiring FROM device_config");
        $this->getRelayState();
        if ($this->relay['o/b'] !== 'no wire') {
            $this->ob_state = $this->conn->getItem("SELECT ob_state FROM device_config");
        } else {
            $this->ob_state = 'no wire';
        }

        $this->updateStates();
    }

    public function updateStates()
    {
        if ($this->relay['o/b'] === 'no wire') { // without OB
            if ($this->relay['y1'] === 'on') {
                $this->before_state = 'cooling';
                $this->current_stage = 1;
                if ($this->relay['y2'] === 'on') {
                    $this->current_stage = 2;
                }
                if ($this->relay['y3'] === 'on') {
                    $this->current_stage = 3;
                }
            } elseif ($this->relay['w1'] === 'on') {
                $this->before_state = 'heating';
                //echo 1;
                $this->current_stage = 1;
                if ($this->relay['w2'] === 'on') {
                    $this->current_stage = 2;
                }
                if ($this->relay['w3'] === 'on') {
                    $this->current_stage = 3;
                }
            }
        } else {
            //echo '-----------'.$this->current_state.'|||||||'. $this->before_state.'---------------';
            if($this->ob_state === 'cool' && $this->relay['o/b'] === 'on'){
                $this->before_state = 'cooling';
            }elseif($this->ob_state === 'cool' && $this->relay['o/b'] === 'off'){
                $this->before_state = 'heating';
            }elseif ($this->ob_state === 'heat' && $this->relay['o/b'] === 'on'){
                $this->before_state = 'heating';
            }elseif ($this->ob_state === 'heat' && $this->relay['o/b'] === 'off'){
                $this->before_state = 'cooling';
            }
            if($this->relay['y1'] === 'on'){
                $this->current_stage = 1;
                if($this->relay['y2'] === 'on'){
                    $this->current_stage = 2;
                    if ($this->relay['y3'] === 'on') {
                        $this->current_stage = 3;
                    }
                }
            }
        }
        $this->current_state = $this->before_state;
    }

    /**
     * Destructor
     */
    public function __destruct()
    {
    }

    /**
     * Set Relay on by alias name
     * @param string $alias
     * @return void
     */
    public function relayOn(string $alias): void
    {
        if ($this->relay[$alias] === 'no wire') {
            return;
        }
        $this->relay[$alias] = 'on';
    }

    /**
     * Set Relay off by alias name
     * @param string $alias
     * @return void
     */
    public function relayOff(string $alias): void
    {
        if ($this->relay[$alias] === 'no wire') {
            return;
        }
        $this->relay[$alias] = 'off';
    }

    /**
     * Get Relay and Wiring states from DB
     * @return string[]
     * @throws Exception
     */
    private function getRelayState(): array
    {
        // Get wiring
        $list = $this->conn->getTable("SELECT * FROM wirings");
        for ($i = 0; $i < count($list); $i++) {
            if ($list[$i]['type'] === 't' && $list[$i]['alias'] !== 'r' && $list[$i]['alias'] !== 'c') {
                $this->relay[$list[$i]['alias']] = 'off';
            }
        }

        // Get relay
        $list = $this->conn->getTable("SELECT * FROM relays");
        for ($i = 0; $i < count($list); $i++) {
            if ($this->relay[$list[$i]['alias']] !== 'no wire') {
                $this->relay[$list[$i]['alias']] = ($list[$i]['type'] === 't') ? 'on' : 'off';
            }
        }
        return $this->relay;
    }

    public function getRelay()
    {
        return $this->relay;
    }

    /**
     * Set relay to DB
     * @param bool $change_temp is need change cooling/heating
     * @return void
     * @throws Exception
     */
    private function setRelay()
    {
        $query = '';
        foreach ($this->relay as $alias => $value) {
            if ($value !== 'no wire') {
                if ($value === 'on') {
                    $query .= "UPDATE relays SET type = true WHERE alias = '{$alias}'; ";
                } else {
                    $query .= "UPDATE relays SET type = false WHERE alias = '{$alias}'; ";
                }
            } else {
                $query .= "UPDATE relays SET type = false WHERE alias = '{$alias}'; ";
            }
        }
        $this->conn->setQuery($query, true)['result'];
    }

    /**
     * Set Relay states to DB
     * @return void
     * @throws Exception
     */
    public function updateRelayState(): void
    {
        $system_delay = intval($this->conn->getItem("SELECT system_delay FROM settings"));
        $current = $this->conn->getRow("SELECT mode,stage,
                                                    (SELECT 
                                                         CASE 
                                                             WHEN (current_timestamp - timestamp) > interval '{$system_delay} minute' THEN 1
                                                             ELSE 0
                                                         END) AS dive 
                                              FROM current_stage");
        switch ($current['mode']) {
            case 0:
                $old_state = 'off';
                break;
            case 1:
                $old_state = 'cooling';
                break;
            case 2:
                $old_state = 'heating';
                break;
            default:
                $old_state = 'off';
        }
        switch ($this->current_state) {
            case  'off':
                $new_state = 0;
                break;
            case 'cooling':
                $new_state = 1;
                break;
            case 'emergency':
            case 'heating':
                $new_state = 2;
                break;
            default:
                $new_state = 0;
        }

//        $is_need_change = false;
//        echo 'dive-'.$current['dive'];
////        echo $current['dive'];
//////        $is_blink = 1;
//        if ($current['dive'] == 't') { // > 5 min
//            echo 1;
//            $is_need_change = true;
//            //$is_blink = 0;
//        } else {
//            echo 2;
//            if ($this->current_stage !== (int)$current['stage'] && $this->current_state === $old_state) {
//                echo 3;
//                if ($this->getTempStageTimer() > 1) {
//                    echo 4;
//                    $is_need_change = true;
//                    //$is_blink = 0;
//                }
//            }
//        }
////        if ($this->current_stage === (int)$current['stage'] && $this->current_state === $old_state) {
////            $is_blink = 0;
////        }
//        if ($is_need_change) {
//            echo 5;
//            //$this->conn->setQuery("UPDATE current_stage SET mode = {$new_state},stage = {$this->current_stage},timestamp = current_timestamp;");
//
//            echo '|||||||||||'.$this->current_state.'-----'.$old_state.'|||||||';
//            //if ($this->current_state !== $old_state) {
//                echo 6;
//                $this->conn->setQuery("UPDATE timing SET set_backlight_time = current_timestamp", true);
//            //}
//        }

        $this->setRelay();
        $old_mode = $this->current_state;
        $old_stage = $this->current_stage;
        $this->updateStates();
        if (($old_mode !== $this->current_state &&
                !($old_mode=='off' && $this->current_stage==0)) ||
            $old_stage != $this->current_stage) {

            $this->conn->setQuery("UPDATE timing SET set_backlight_time = current_timestamp");
        }
        $this->backlight();
    }

    /**
     * Turn set mode Off
     * @return void
     */
    public function setAllOff()
    {
        foreach ($this->relay as $alias => $value) {
            if ($this->relay[$alias] !== 'no wire' && $alias !== 'g' && $alias !== 'acc1n' && $alias !== 'acc1p' && $alias !== 'acc2') {
                $this->relay[$alias] = 'off';
            }
        }
        $this->current_state = 'off';
    }

    /**
     * Set relay Heating stage 0
     * @return bool
     * @throws Exception
     */
    public function heatingStage0(): bool
    {
        if ($this->ob_state === 'heat') {
            $this->relayOn('o/b');
        } else {
            $this->relayOff('o/b');
        }
        $this->relayOff('y1');
        $this->relayOff('y2');
        $this->relayOff('y3');

        $this->relayOff('w1');
        $this->relayOff('w2');
        $this->relayOff('w3');

        $this->startTempTimer('heating', 0);
        $this->current_state = 'heating';
        return true;
    }

    /**
     * Set relay Cooling stage 0
     * @return bool
     * @throws Exception
     */
    public function coolingStage0(): bool
    {
        $this->relayOff('o/b');
        if ($this->ob_state === 'cool') {
            $this->relayOn('o/b');
        } else {
            $this->relayOff('o/b');
        }
        $this->relayOff('y1');
        $this->relayOff('y2');
        $this->relayOff('y3');

        $this->relayOff('w1');
        $this->relayOff('w2');
        $this->relayOff('w3');

        $this->startTempTimer('cooling', 0);
        $this->current_state = 'cooling';
        return true;
    }

    /**
     * Set relay Heating stage 1
     * @return bool
     * @throws Exception
     */
    public function heatingStage1(): bool
    {
        if ($this->relay['o/b'] !== 'no wire') {
            if ($this->ob_state === 'heat') {
                $this->relayOn('o/b');
            } else {
                $this->relayOff('o/b');
            }
            $this->relayOn('y1');
            $this->relayOff('y2');
            $this->relayOff('y3');
        } else {
            $this->relayOff('y1');
            $this->relayOff('y2');
            $this->relayOff('y3');
            $this->relayOn('w1');
            $this->relayOff('w2');
            $this->relayOff('w3');
        }
        $this->startTempTimer('heating', 1);
        $this->current_state = 'heating';
        return true;
    }

    /**
     * Set relay Cooling stage 1
     * @return bool
     * @throws Exception
     */
    public function coolingStage1(): bool
    {
        if ($this->relay['o/b'] !== 'no wire') {
            if ($this->ob_state === 'cool') {
                $this->relayOn('o/b');
            } else {
                $this->relayOff('o/b');
            }
            $this->relayOn('y1');
            $this->relayOff('y2');
            $this->relayOff('y3');
        } else {

            $this->relayOn('y1');
            $this->relayOff('y2');
            $this->relayOff('y3');
            $this->relayOff('w1');
            $this->relayOff('w2');
            $this->relayOff('w3');
        }
        $this->startTempTimer('cooling', 1);
        $this->current_state = 'cooling';
        return true;
    }


    /**
     * Set relay Heating stage 2
     * @return bool
     * @throws Exception
     */
    public function heatingStage2(): bool
    {
        if ($this->relay['o/b'] !== 'no wire') {
            if ($this->ob_state === 'heat') {
                $this->relayOn('o/b');
            } else {
                $this->relayOff('o/b');
            }

            $this->relayOn('y1');
            $this->relayOn('y2');
            $this->relayOff('y3');
        } else {
            $this->relayOff('y1');
            $this->relayOff('y2');
            $this->relayOff('y3');
            $this->relayOn('w1');
            $this->relayOn('w2');
            $this->relayOff('w3');
        }
        $this->startTempTimer('heating', 2);
        $this->current_state = 'heating';
        return true;
    }

    /**
     * Set relay Cooling stage 2
     * @return bool
     * @throws Exception
     */
    public function coolingStage2(): bool
    {
        if ($this->relay['o/b'] !== 'no wire') {
            if ($this->ob_state === 'cool') {
                $this->relayOn('o/b');
            } else {
                $this->relayOff('o/b');
            }
            $this->relayOn('y1');
            $this->relayOn('y2');
            $this->relayOff('y3');
        } else {
            $this->relayOn('y1');
            $this->relayOn('y2');
            $this->relayOff('y3');
            $this->relayOff('w1');
            $this->relayOff('w2');
            $this->relayOff('w3');
        }
        $this->startTempTimer('cooling', 2);
        $this->current_state = 'cooling';
        return true;
    }

    /**
     * Set relay Heating stage 3
     * @return bool
     * @throws Exception
     */
    public function heatingStage3(): bool
    {
        if ($this->relay['o/b'] !== 'no wire') {
            if ($this->ob_state === 'heat') {
                $this->relayOn('o/b');
            } else {
                $this->relayOff('o/b');
            }

            $this->relayOn('y1');
            $this->relayOn('y2');
            $this->relayOn('y3');
        } else {
            $this->relayOn('w1');
            $this->relayOn('w2');
            $this->relayOn('w3');

            $this->relayOff('y1');
            $this->relayOff('y2');
            $this->relayOff('y3');
        }
        $this->startTempTimer('heating', 3);
        $this->current_state = 'heating';
        return true;
    }

    /**
     * Set relay Cooling stage 3
     * @return bool
     * @throws Exception
     */
    public function coolingStage3(): bool
    {
        if ($this->relay['o/b'] !== 'no wire') {
            if ($this->ob_state === 'cool') {
                $this->relayOn('o/b');
            } else {
                $this->relayOff('o/b');
            }

            $this->relayOn('y1');
            $this->relayOn('y2');
            $this->relayOn('y3');
        } else {
            $this->relayOn('y1');
            $this->relayOn('y2');
            $this->relayOn('y3');
            $this->relayOff('w1');
            $this->relayOff('w2');
            $this->relayOff('w3');
        }
        $this->startTempTimer('cooling', 3);
        $this->current_state = 'cooling';
        return true;
    }

    /**
     * Set relay On Humidifier
     * @param string $hum_wiring
     * @return bool
     */
    public function humidifierOn(): bool
    {
        $this->relayOn($this->hum_wiring);
        return true;
    }

    /**
     * Set relay Off Humidifier
     * @return bool
     */
    public function humidifierOff(): bool
    {
        $this->relayOff($this->hum_wiring);
        return true;
    }

    /**
     * Set relay On Dehumidifier
     * @param string $hum_wiring
     * @return bool
     */
    public function deHumidifierOn(): bool
    {
        $this->relayOn($this->hum_wiring);
        return true;
    }

    /**
     * Set relay Off Dehumidifier
     * @param string $hum_wiring
     * @return bool
     */
    public function deHumidifierOff(): bool
    {
        $this->relayOff($this->hum_wiring);
        return true;
    }

    /**
     * Set relay On Emergency heating stage 1
     * @return bool
     * @throws Exception
     */
    public function emergencyHeating1(): bool
    {
        if ($this->relay['o/b'] !== 'no wire') {
            $this->relayOff('y1');
            $this->relayOff('y2');
            $this->relayOff('y3');

            $this->relayOn('w1');
            $this->relayOff('w2');

            $this->current_state = 'emergency';
            return true;
        }
        return false;
    }

    /**
     * Set relay On Emergency heating stage 2
     * @return bool
     * @throws Exception
     */
    public function emergencyHeating2(): bool
    {
        if ($this->relay['o/b'] !== 'no wire') {
            $this->relayOff('y1');
            $this->relayOff('y2');
            $this->relayOff('y3');

            $this->relayOn('w1');
            $this->relayOn('w2');

            $this->current_state = 'emergency';
            return true;
        }
        return false;
    }

    /**
     * @throws Exception
     */

    // call from startWork in Shceme
    public function fanWorkTime()
    {
        //$fan_settings = $this->conn->getRow("SELECT fan_time AS user_set_time, start_fan_timing AS work_in_hour,(SELECT fan FROM current_state) AS user_set_interval, EXTRACT(MINUTE from (current_timestamp))-EXTRACT(MINUTE from (fan_time)) AS interval_minute, EXTRACT(MINUTE from (current_timestamp))-EXTRACT(MINUTE from (fan_time)) AS other FROM timing");
        $interval = $this->conn->getItem("SELECT fan FROM current_state");
        if((int)$interval > 0) {
            $fan_settings = $this->conn->getRow("SELECT EXTRACT(MINUTE from ((current_timestamp - fan_time)- interval '{$interval} minute'))-1 AS work FROM timing");
            $this->getRelayState(); // Update relay
            if ($this->relay['y1'] === 'on' || $this->relay['w1'] === 'on') {
                $this->fanOn();
            } else {
                if ((int)$interval > 0 && (int)$fan_settings['work'] < 0 && (int)$fan_settings['work'] >= (-1) * (int)$interval) {
                    $this->fanOn();
                } else { // fan auto mode
                    $this->fanOff();
                }
//            if ((int)$fan_settings["user_set_interval"] > 0) { // fan on mode
//                if ((int)$fan_settings['interval_minute'] > 60) { // next hour
//                    $this->conn->setQuery("UPDATE timing SET fan_time = current_timestamp, start_fan_timing = 0");
//                    $this->fanOn();
//                } else if ((int)$fan_settings['work_in_hour'] > (int)$fan_settings['other']) { // current hour need on
//                    $work_in_hour = (int)$fan_settings['interval_minute'];
//                    $this->conn->setQuery("UPDATE timing SET start_fan_timing = {$work_in_hour}");
//                    $this->fanOn();
//                } else { // current hour need off
//                    if ($this->current_state !== 'heating' && $this->current_state !== 'cooling') {
//                        $this->fanOff();
//                    }
//                }
//            } else { // fan auto mode
//                $this->fanOff();
//            }
            }
        } else {
            if ($this->relay['y1'] === 'on' || $this->relay['w1'] === 'on') {
                $this->fanOn();
            }else{
                $this->fanOff();
            }
        }

        if ($this->relay['g'] !== 'no wire') {
            if ($this->relay['g'] === 'on') {
                $this->conn->setQuery("UPDATE relays SET type = true WHERE alias = 'g';");
            } else {
                $this->conn->setQuery("UPDATE relays SET type = false WHERE alias = 'g';");
            }
            $this->getRelayState();
        }
    }

    /**
     * Set Fan relay On
     * @return void
     * @throws Exception
     */
    public function fanOn()
    {
        if ($this->relay['g'] !== 'no wire') {
            $this->relay['g'] = 'on';
        }
    }

    /**
     * Set Fan relay Off
     * @return void
     * @throws Exception
     */
    public function fanOff()
    {
        if ($this->relay['g'] !== 'no wire') {
            $this->relay['g'] = 'off';
        }
    }

    /**
     * Start or restart timer for main state and stage
     * @param $current_state
     * @param $stage
     * @return void
     * @throws Exception
     */
    public function startTempTimer($current_state, $stage)
    {
        if ($current_state !== $this->before_state) {
            $this->before_state = $current_state;
            $this->conn->setQuery("UPDATE timing SET uptime = current_timestamp");
        }
        $current_stage = intval($this->conn->getItem("SELECT stage FROM current_stage"));
        if ($current_stage !== $stage) {
            $this->conn->setQuery("UPDATE timing SET s1uptime = current_timestamp");
        }
    }

    /**
     * Get Heating/Cooling power on time
     * @return int
     */
    public function getTempTimer(): int
    {
        return intval($this->conn->getItem("SELECT EXTRACT(minute FROM (current_timestamp - uptime)) FROM timing"));
    }

    /**
     * Get Heating/Cooling time in current stage
     * @return int
     */
    public function getTempStageTimer(): int
    {
     // It calculates the difference in minutes between the current timestamp
     // and a column s1uptime in a table named timing.
        return intval($this->conn->getItem("SELECT EXTRACT(minute FROM (current_timestamp - s1uptime)) FROM timing"));
    }

    /**
     * Get Heating/Cooling time in current stage
     * @return int
     */
    public function getTempStageTimer2(): int
    {
        return intval($this->conn->getItem("SELECT EXTRACT(minute FROM (current_timestamp - s2uptime)) FROM timing"));
    }

    /**
     * Get fan power on time in current hour
     * @return int
     */
    public function getFanTimer(): int
    {
        return intval($this->conn->getItem("SELECT EXTRACT(minute FROM (current_timestamp - fan_time)) FROM timing"));
    }

    /**
     * echo function realisation
     * @return string
     */
    public function __toString()
    {
        $temp_stage_timer = $this->getTempStageTimer();
        $temp_main_timer = $this->getTempTimer();
        $fan_timer = $this->getFanTimer();

        $relay_str = '';
        foreach ($this->relay as $alias => $value) {
            $relay_str .= $alias . '->';
            $relay_str .= $value;
            $relay_str .= "\n";
        }

        $relay_str .= 'Work mode ->' . $temp_main_timer . " ";
        $relay_str .= 'in stage ->' . $temp_stage_timer . "\n";
        $relay_str .= 'Fan timer ->' . $fan_timer;

        return $relay_str;
    }

    // Called from updateRelayState function
    //
    private function backlight()
    {
        $backlight = $this->conn->getRow("SELECT ((SELECT set_backlight_time FROM timing) + interval '5 second') AS backlight_interval,
                                          current_timestamp FROM device_config");

        if ($backlight['backlight_interval'] <= $backlight['current_timestamp'] && $this->current_state !== 'emergency') { // >5s and not emergency user selected
            $this->conn->setQuery("UPDATE device_config SET scheme_backlight_rgb = ARRAY[0,0,0],backlight_type = 0;");
        } else {
          //  echo $this->current_state;
            //$color = self::OFF_RGBM;
            switch ($this->current_state) {
                case 'off':
                    //$color = self::OFF_RGBM;
                    $color = [0,0,0,0]; // [r,g,b, backlight_type]
                    break;
                case 'cooling':
                    $color = self::COOLING_RGBM;
                    break;
                case 'emergency':
                    $color = self::EMERGENCY_RGBM;
                    $color_st = self::EMERGENCY_ST_RGBM; // Why use two variables? the color is enoght
                    break;
                case 'heating':
                    $color = self::HEATING_RGBM;
                    break;
            }
            if ($backlight['backlight_interval'] > $backlight['current_timestamp']) {      //< 5s
                $st_array = 'ARRAY[' . $color[0] . ',' . $color[1] . ',' . $color[2] . ']';
                $this->conn->setQuery("UPDATE device_config SET scheme_backlight_rgb = {$st_array},
                                        backlight_type = '{$color[3]}'");
            } elseif ($this->current_state === 'emergency') {                                                                            // Emergency
                $st_array = 'ARRAY[' . $color_st[0] . ',' . $color_st[1] . ',' . $color_st[2] . ']';
                $this->conn->setQuery("UPDATE device_config SET scheme_backlight_rgb = {$st_array},
                                        backlight_type = '{$color_st[3]}'");
            }
        }

    }

    public function getModeName()
    {
        return $this->current_state;
    }

    public function getModeId()
    {
        switch ($this->current_state) {
            case 'cooling':
                return 1;
            case 'emergency':
            case 'heating':
                return 2;
            default: // off
                return 0;
        }
    }

    public function getCurrentStage(): int
    {
        return $this->current_stage;
    }

    public function hasEmergency(): int
    {
        if ($this->relay['o/b'] !== 'no wire' && $this->relay['w1'] !== 'no wire') {
            if ($this->relay['o/b'] !== 'no wire' && $this->relay['w1'] !== 'no wire' && $this->relay['w2'] !== 'no wire') {
                return 2;
            }
            return 1;
        }
        return 0;
    }

    /**
     * @return string
     */
    public function getObState(): string
    {
        return $this->ob_state;
    }

    /**
     * @param string $ob_state
     */
    public function setObState(string $ob_state): void
    {
        $this->ob_state = $ob_state;
    }

    public function getHeatingMaxStage()
    {
        $stage = 0;
        if ($this->relay['o/b'] !== 'no wire') {
            if ($this->relay['y1'] !== 'no wire') {
                $stage = 1;
                if ($this->relay['y2'] !== 'no wire') {
                    $stage = 2;
                }
            }
        } else {
            if ($this->relay['w1'] !== 'no wire') {
                $stage = 1;
                if ($this->relay['w2'] !== 'no wire') {
                    $stage = 2;
                    if ($this->relay['w3'] !== 'no wire') {
                        $stage = 3;
                    }
                }
            }
        }
        return $stage;
    }

    public function getCoolingMaxStage()
    {
        $stage = 0;
        if ($this->relay['o/b'] !== 'no wire') {
            if ($this->relay['y1'] !== 'no wire') {
                $stage = 1;
                if ($this->relay['y2'] !== 'no wire') {
                    $stage = 2;
                }
            }
        } else {
            if ($this->relay['y1'] !== 'no wire') {
                $stage = 1;
                if ($this->relay['y2'] !== 'no wire') {
                    $stage = 2;
                    if ($this->relay['y3'] !== 'no wire') {
                        $stage = 3;
                    }
                }
            }
        }
        return $stage;
    }

}
