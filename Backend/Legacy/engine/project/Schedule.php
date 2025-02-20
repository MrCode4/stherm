<?php

namespace project;
use core\db\Z_PgSQL;
use Exception;

/**
 * Work with schedules
 * @package project
 */
class Schedule
{
    private $conn;

    /**
     * @throws Exception
     */
    public function __construct() {
        $this->conn = Z_PgSQL::connection();
    }

    /**
     * Get schedule list
     * @return array[]
     * @throws Exception
     */
    public function getScheduleList() {
        $schedules = $this->conn->getTable("SELECT schedule_id                            AS id,
                                                         name,
                                                         type_id                                AS type,
                                                         is_enable                              AS state,
                                                         json_build_array(d0,d1,d2,d3,d4,d5,d6) AS week_days
                                                  FROM schedules
                                                  WHERE is_remove = false
                                                    GROUP BY is_enable, type_id, name, d0, d1, d2, d3, d4, d5, d6, schedule_id
                                                    ORDER BY schedule_id");
        for($i = 0; $i < count($schedules); $i++) {
            if($schedules[$i]['state'] === 't') {
                $schedules[$i]['state'] = true;
            } else {
                $schedules[$i]['state'] = false;
            }
            $schedules[$i]['id'] = (int)$schedules[$i]['id'];
            $schedules[$i]['type'] = (int)$schedules[$i]['type'];
            $schedules[$i]['week_days'] = json_decode($schedules[$i]['week_days']);
        }
        return $schedules;
    }

    /**
     * Get schedule's info
     * @param int $id
     * @return array
     * @throws Exception
     */
    public function getSchedule(int $id) {
        $schedule = $this->conn->getRow("SELECT schedule_id                            AS id,
                                                      name,
                                                      temp                                   AS temperature,
                                                      start_time,
                                                      end_time,
                                                      json_build_array(d0,d1,d2,d3,d4,d5,d6) AS repeat,
                                                      type_id                                AS type,
                                                      is_enable                              AS state,
                                                      (SELECT measure_id FROM settings)      AS measure
                                               FROM schedules
                                               WHERE schedule_id = '{$id}' AND is_remove = false");
        $schedule['id'] = (int)$schedule['id'];
        $schedule['repeat'] = json_decode($schedule['repeat']);
        $schedule['type'] = (int)$schedule['type'];
        if($schedule['state'] === 't') {
            $schedule['state'] = true;
        } else {
            $schedule['state'] = false;
        }
        if((int)$schedule['measure'] === 1) {                                                  // if measure is fahrenheit
            $schedule['temperature'] = round($schedule['temperature'] * 9/5 + 32);
        } else {
            $schedule['temperature'] = (int)$schedule['temperature'];
        }
        unset($schedule['measure']);
        return $schedule;
    }

//    /**
//     * Get all matching schedules
//     * @param int $schedule_id
//     * @param string $start_time
//     * @param string $end_time
//     * @param array $week_days
//     * @return array
//     * @throws Exception
//     */
//    private function getMatchingSchedules(int $schedule_id,string $start_time,string $end_time,array $week_days) {
//        $query = "SELECT schedule_id
//                  FROM schedules
//                  WHERE schedule_id != {$schedule_id} AND (('{$start_time}' > start_time AND '{$start_time}' < end_time) OR
//                                                           ('{$end_time}' > start_time AND '{$end_time}' < end_time) OR
//                                                           ('{$start_time}' < start_time AND '{$end_time}' > end_time)) AND (";
//        for($i = 0; $i < count($week_days); $i++) {
//            if($week_days[$i] === true) {
//                $query .= "d".$i."=true OR ";
//            }
//        }
//        $query = rtrim($query,' OR');
//        $query .= ')';
//        return $this->conn->getTable($query);
//    }



    /**
     * Get all matching schedules
     * @param int $schedule_id
     * @param string $start_time
     * @param string $end_time
     * @param array $week_days
     * @return array
     * @throws Exception
     */
    private function getMatchingSchedules(int $schedule_id,string $start_time,string $end_time,array $week_days) {
        $query = "SELECT schedule_id
                  FROM schedules
                  WHERE schedules.is_remove = false AND schedule_id != {$schedule_id} AND is_enable = true AND
                                                          (((('{$start_time}' > start_time AND '{$start_time}' < end_time) OR
                                                           ('{$end_time}' > start_time AND '{$end_time}' < end_time) OR
                                                           ('{$start_time}' < start_time AND '{$end_time}' > end_time AND end_time > start_time) OR  
                                                           ('{$start_time}' = start_time AND '{$end_time}' = end_time) OR
                                                           ('{$end_time}' = end_time AND '{$end_time}' > start_time) OR
                                                           ('{$start_time}' > '{$end_time}' AND '{$start_time}' < start_time AND '{$end_time}' < start_time) OR
                                                           ('{$start_time}' > '{$end_time}' AND '{$start_time}' > end_time AND '{$end_time}' > end_time) OR
                                                           ('{$start_time}' >= start_time AND '{$start_time}' < end_time) OR  
                                                           (end_time < start_time AND '{$start_time}' > start_time)) AND (";

        // ('{$start_time}' <= start_time AND '{$end_time}' >= end_time) OR
        for($i = 0; $i < count($week_days); $i++) {
            if($week_days[$i] === true) {
                $query .= "d".$i."=true OR ";
            }
        }
        $query = rtrim($query,' OR');
        $query .= ")) OR (end_time < start_time AND '{$start_time}' < end_time AND (";
        for($i = 0; $i < count($week_days); $i++) {
            if($week_days[$i] === true) {
                if($i === 0) {
                    $week_day = 6;
                } else {
                    $week_day = $i - 1;
                }
                $query .= "d".$week_day."=true OR ";
            }
        }
        $query = rtrim($query,' OR');
        $query .= ")))";
        return $this->conn->getTable($query);
    }

    /**
     * Add or edit schedule
     * @param int $id // id: 0 - add, other - edit
     * @param string $name
     * @param int $temp
     * @param string $start_time
     * @param string $end_time
     * @param array $repeat // [true,false,...x7]
     * @param int $type
     * @param bool $state
     * @param bool $other_schedule // other_schedule: first time-true, other-false
     * @return bool
     * @throws Exception
     */
    public function setSchedule(int $id, string $name, int $temp, string $start_time, string $end_time, array $repeat, int $type, bool $state, bool $other_schedule) {
        if(count($repeat) != 7) {
            throw new Exception("Incorrect data",1);
        }
        $time = strtotime($end_time) - strtotime($start_time);
        //if(($time/60 >= 120 && $time != 0) || strtotime($end_time) < strtotime($start_time)) {
       if(($time/60 >= 10 && $time != 0) || strtotime($end_time) < strtotime($start_time)) {
            $measure = (int)$this->conn->getItem("SELECT measure_id FROM settings");
            if($measure === 1) {                                                                // if measure is fahrenheit
                $temp = number_format(((int)$temp - 32) * 5/9, 2, '.', '');
            }
            $state = (int)$state;
            if($id === 0) {                                                                     // add new schedule
                if($state === 1) {
                    $busySchedules = self::getMatchingSchedules($id, $start_time, $end_time, $repeat);
                    for ($i = 0; $i < count($repeat); $i++) {
                        $repeat[$i] = (int)$repeat[$i];
                    }
                    if ($other_schedule === true && !empty($busySchedules)) {                   // first time
                        return false;
                    } else if (empty($busySchedules)) {
                        //$set = (bool)$this->conn->setQuery("UPDATE current_state SET state_id = 2,is_sent = false;
                        $set = (bool)$this->conn->setQuery("INSERT INTO schedules(name,type_id,start_time,end_time,is_enable,temp,d0,d1,d2,d3,d4,d5,d6,is_sent,web_id)
                                                                  VALUES('{$name}','{$type}','{$start_time}','{$end_time}','{$state}','{$temp}','{$repeat[0]}',
                                                                          '{$repeat[1]}','{$repeat[2]}','{$repeat[3]}','{$repeat[4]}','{$repeat[5]}','{$repeat[6]}',false,0)")['result'];
                    } else if ($other_schedule === false && !empty($busySchedules)) {
                        //$query = "UPDATE current_state SET state_id = 2,is_sent = false;
                        $query = "UPDATE schedules SET is_enable = false,is_sent = false WHERE schedule_id IN (";
                        for ($i = 0; $i < count($busySchedules); $i++) {
                            $query .= $busySchedules[$i]['schedule_id'] . ",";
                        }
                        $query = rtrim($query, ',');
                        $query .= "); INSERT INTO schedules(name,type_id,start_time,end_time,is_enable,temp,d0,d1,d2,d3,d4,d5,d6,is_sent,web_id)
                                      VALUES('{$name}','{$type}','{$start_time}','{$end_time}','{$state}','{$temp}',
                                             '{$repeat[0]}','{$repeat[1]}','{$repeat[2]}','{$repeat[3]}','{$repeat[4]}','{$repeat[5]}','{$repeat[6]}',false,0)";
                        $set = (bool)$this->conn->setQuery($query, true)['result'];
                    }
                } else {
                    for ($i = 0; $i < count($repeat); $i++) {
                        $repeat[$i] = (int)$repeat[$i];
                    }
                    $set = (bool)$this->conn->setQuery("INSERT INTO schedules(name,type_id,start_time,end_time,is_enable,temp,d0,d1,d2,d3,d4,d5,d6,is_sent,web_id)
                                                              VALUES('{$name}','{$type}','{$start_time}','{$end_time}','{$state}','{$temp}','{$repeat[0]}',
                                                               '{$repeat[1]}','{$repeat[2]}','{$repeat[3]}','{$repeat[4]}','{$repeat[5]}','{$repeat[6]}',false,0)")['result'];
                }
                //if(empty((new System)->checkCurrentSchedule()) && empty((new System)->checkCurrentSchedule($start_time))) {
                //var_dump((new System)->checkCurrentSchedule());
                if(empty((new System)->checkCurrentSchedule()) || $this->conn->getItem("SELECT state_next_check FROM current_state") === '') {
                    (new System)->setCurrentSchedule();
                }
            } else {                                                                            // edit schedule by id
                if($state === 1) {
                    $busySchedules = self::getMatchingSchedules($id,$start_time,$end_time,$repeat);
                    for($i = 0; $i < count($repeat); $i++) {
                        $repeat[$i] = (int)$repeat[$i];
                    }
                    if($other_schedule === true && !empty($busySchedules)) {
                        return false;
                    } else if(empty($busySchedules)) {
                        //$set = (bool)$this->conn->setQuery("UPDATE current_state SET state_id = 2,is_sent = false;
                        $set = (bool)$this->conn->setQuery("UPDATE schedules SET name = '{$name}',
                                                                                       temp = '{$temp}',
                                                                                       start_time = '{$start_time}',
                                                                                       end_time = '{$end_time}',
                                                                                       type_id = '{$type}',
                                                                                       is_enable = '{$state}',
                                                                                       d0 = '{$repeat[0]}',
                                                                                       d1 = '{$repeat[1]}',
                                                                                       d2 = '{$repeat[2]}',
                                                                                       d3 = '{$repeat[3]}',
                                                                                       d4 = '{$repeat[4]}',
                                                                                       d5 = '{$repeat[5]}',
                                                                                       d6 = '{$repeat[6]}',
                                                                                       is_sent = false
                                                                  WHERE schedule_id = '{$id}'")['result'];
                        $current = $this->conn->getItem("SELECT current_time FROM current_state");
                        if(empty((new System)->checkCurrentSchedule()) || $this->conn->getItem("SELECT state_next_check FROM current_state") === '') {
                            (new System)->setCurrentSchedule();
                        }
                    } else if($other_schedule === false && !empty($busySchedules)) {
                        //$query = "UPDATE current_state SET state_id = 2,is_sent = false;
                         $query = "UPDATE schedules SET is_enable = false,is_sent = false WHERE schedule_id IN (";
                        for($i = 0; $i < count($busySchedules); $i++) {
                            $query .= $busySchedules[$i]['schedule_id'].",";
                        }
                        $query = rtrim($query,',');
                        $query .= "); UPDATE schedules SET name = '{$name}',
                                                           temp = '{$temp}',
                                                           start_time = '{$start_time}',
                                                           end_time = '{$end_time}',
                                                           type_id = '{$type}',
                                                           is_enable = '{$state}',
                                                           d0 = '{$repeat[0]}',
                                                           d1 = '{$repeat[1]}',
                                                           d2 = '{$repeat[2]}',
                                                           d3 = '{$repeat[3]}',
                                                           d4 = '{$repeat[4]}',
                                                           d5 = '{$repeat[5]}',
                                                           d6 = '{$repeat[6]}',
                                                           is_sent = false
                                      WHERE schedule_id = '{$id}'";
                        $set = (bool)$this->conn->setQuery($query,true)['result'];
                    }
                } else {
                    for($i = 0; $i < count($repeat); $i++) {
                        $repeat[$i] = (int)$repeat[$i];
                    }
                    //$set = (bool)$this->conn->setQuery("UPDATE current_state SET state_id = 2,is_sent = false;
                    $set = (bool)$this->conn->setQuery("UPDATE schedules SET name = '{$name}',
                                                                                   temp = '{$temp}',
                                                                                   start_time = '{$start_time}',
                                                                                   end_time = '{$end_time}',
                                                                                   type_id = '{$type}',
                                                                                   is_enable = '{$state}',
                                                                                   d0 = '{$repeat[0]}',
                                                                                   d1 = '{$repeat[1]}',
                                                                                   d2 = '{$repeat[2]}',
                                                                                   d3 = '{$repeat[3]}',
                                                                                   d4 = '{$repeat[4]}',
                                                                                   d5 = '{$repeat[5]}',
                                                                                   d6 = '{$repeat[6]}',
                                                                                   is_sent = false
                                                              WHERE schedule_id = '{$id}'")['result'];
                }
            }
            return $set;
        } else {
            return (object)["type" => ERROR, "message" => "Schedule minimum delta is 10 minutes", "result" => false];
          //  throw new Exception('Schedule minimum delta is 2 hours', 1);
        }
    }

    /**
     * Check schedule's name
     * @param string $name
     * @return bool
     */
    public function checkScheduleName(string $name) {
        if($name !== '') {
            $is_exist = (int)$this->conn->getItem("SELECT count(schedule_id)
                                                         FROM schedules
                                                         WHERE name = '{$name}' AND is_remove = false");
            if($is_exist === 0) {
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    /**
     * Remove schedule
     * @param int $id
     * @return bool
     * @throws Exception
     */
    public function removeSchedule(int $id) {
        $set = (bool)$this->conn->setQuery("UPDATE schedules SET is_remove = true,is_sent = false WHERE schedule_id = '{$id}'")['result'];
        (new System)->setCurrentSchedule();
        return $set;
    }

    /**
     * Enable or disable schedule
     * @param int $id
     * @param bool $state
     * @param bool $other_schedule                             // other_schedule: first time-true, other-false
     * @return bool
     * @throws Exception
     */
    public function enableSchedule(int $id, bool $state, bool $other_schedule) {
        $state = (int)$state;
        if($state === 1) {
            $schedule = $this->conn->getRow("SELECT start_time,
                                                          end_time,
                                                          json_build_array(d0,d1,d2,d3,d4,d5,d6) AS week_days
                                                   FROM schedules
                                                   WHERE is_remove = false AND schedule_id = '{$id}'");
            $schedule['week_days'] = json_decode($schedule['week_days']);
            $busy_schedules = self::getMatchingSchedules($id,$schedule['start_time'],$schedule['end_time'],$schedule['week_days']);
            if(empty($busy_schedules)) {
                $current = $this->conn->getRow("SELECT current_time,start_time FROM schedules WHERE schedule_id = '{$id}'");
                //if($current['current_time'] < $current['start_time']) {
                (new System)->setCurrentSchedule();
                //}
                $set = (bool)$this->conn->setQuery("UPDATE schedules SET is_enable = true,is_sent = false WHERE schedule_id = '{$id}'")["result"];
                //                                          UPDATE current_state SET state_id = 2,is_sent = false")['result'];

            } else if($other_schedule === true) {
                return false;
            } else {
                $current = $this->conn->getRow("SELECT current_time,start_time FROM schedules WHERE schedule_id = '{$id}'");
               // if($current['current_time'] < $current['start_time']) {
                //    (new System)->setCurrentSchedule();
              //  }
               // $query = "UPDATE current_state SET state_id = 2,is_sent = false;
                $query = "UPDATE schedules SET is_enable = false,is_sent = false WHERE schedule_id IN(";
                for($i = 0; $i < count($busy_schedules); $i++) {
                    $query .= $busy_schedules[$i]['schedule_id'].",";
                }
                $query = rtrim($query,',');
                $query .= "); UPDATE schedules SET is_enable = true,is_sent = false WHERE schedule_id = '{$id}'";
                $set = (bool)$this->conn->setQuery($query,true)["result"];
            }
            if(empty((new System)->checkCurrentSchedule()) || $this->conn->getItem("SELECT state_next_check FROM current_state") === '') {
                (new System)->setCurrentSchedule();
            }
            //(new System)->setCurrentSchedule();
            return $set;
        } else {
            return (bool)$this->conn->setQuery("UPDATE schedules SET is_enable = false,is_sent = false WHERE schedule_id = '{$id}'")['result'];
        }
    }
}