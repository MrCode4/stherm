<?php

namespace project;
use core\db\Z_PgSQL;
use Exception;

/**
 * Work with main and external sensors
 * @package project
 */
class Sensors
{
    private $conn;

    /**
     * @throws Exception
     */
    public function __construct() {
        $this->conn = Z_PgSQL::connection();
    }

    /**
     * Get main sensor's data
     * @return int[]             // [temp,hum,tof,amb,co2]
     * @throws Exception
     */
    public function getSensorsData() {
        $measure = (int)$this->conn->getItem("SELECT measure_id FROM settings");
        $sensor_data = $this->conn->getRow("SELECT sensor_logs.temp AS temp,
                                                         sensor_logs.humidity,
                                                         sensor_logs.tof,
                                                         sensor_logs.ambiend,
                                                         sensor_logs.co2
                                                  FROM sensors
                                                        LEFT JOIN sensor_logs ON sensors.sensor_id = sensor_logs.sensor_id
                                                  WHERE is_main = true AND is_remove = false
                                                  ORDER BY timestamp DESC
                                                  LIMIT 1");
        if(!empty($sensor_data)) {
            if($measure === 1) {                                                       // if measure is fahrenheit
                $sensor_data_result[] = round($sensor_data['temp'] * 9/5) + 32;
            } else {
                $sensor_data_result[] = (int)$sensor_data['temp'];
            }
            $sensor_data_result[] = (int)$sensor_data['humidity'];
            $sensor_data_result[] = (int)$sensor_data['tof'];
            $sensor_data_result[] = (int)$sensor_data['ambiend'];
            $sensor_data_result[] = (int)$sensor_data['co2'];
            return $sensor_data_result;
        } else {
            return [];
        }
    }

    /**
     * Start or end pairing list
     * @param int $state // 1 - start, end - stop
     * @return bool
     * @throws Exception
     */
    public function startEndPairing(int $state) {
        if($state === 1) {
            return (bool)$this->conn->setQuery("UPDATE device_config SET start_pairing = true")['result'];
        } else if($state === 0) {
            return (bool)$this->conn->setQuery("UPDATE device_config SET start_pairing = false")['result'];
        }
        return false;
    }

    /**
     * Get external sensor list
     * @return array[]
     * @throws Exception
     */
    public function getSensorPairList() {
        return $this->conn->getTable("SELECT sensor_id AS id,
                                                   sensor,
                                                   type_id   AS type
                                            FROM sensors
                                            WHERE is_main = false AND is_paired = false AND is_remove = false");

    }

    /**
     * Get all paired sensors
     * @return array
     * @throws Exception
     */
    public function getSensorList() {
        $avg_temp = $this->conn->getRow("SELECT (SELECT settings.measure_id FROM settings) AS measure_id, 
                                                      current_temp                               AS temp
                                               FROM current_state");
        $sensors = $this->conn->getTable("SELECT sensor_id AS id,
                                                       name,
                                                       type_id   AS type,
                                                       is_main
                                                FROM sensors
                                                    WHERE is_paired = true AND is_remove = false
                                                ORDER BY sensor_id");
        for($i = 0; $i < count($sensors); $i++) {
            $sensors[$i]['id'] = (int)$sensors[$i]['id'];
            $sensors[$i]['type'] = (int)$sensors[$i]['type'];
            if($sensors[$i]['is_main'] === 't') {
                $sensors[$i]['is_main'] = true;
            } else {
                $sensors[$i]['is_main'] = false;
            }
        }
        if((int)$avg_temp['measure_id'] === 1) {                                                       // if measure is fahrenheit
            $avg_temp['temp'] = round($avg_temp['temp'] * 9/5) + 32;
        } else {
            $avg_temp['temp'] = (int)$avg_temp['temp'];
        }
        return ['avg_temp' => $avg_temp['temp'],
                'sensors'  => $sensors];
    }

    /**
     * Get sensors locations
     * @return array
     * @throws Exception
     */
    public function getSensorLocations() {
        $locations = $this->conn->getTable("SELECT location_id AS id,
                                                         name,
                                                         alias
                                                  FROM sensor_locations
                                                  ORDER BY location_id");
        for($i = 0; $i < count($locations); $i++) {
            $locations[$i]['id'] = (int)$locations[$i]['id'];
        }
        return $locations;
    }

    /**
     * Add or edit sensor
     * @param int $sensor_id          // 0 - add sensor, else - edit sensor by id
     * @param string $sensor
     * @param string $name
     * @param int $location_id
     * @return bool
     * @throws Exception
     */
    public function setSensor(int $sensor_id, string $sensor, string $name, int $location_id) {
        if($sensor_id === 0) {                                            // add sensor
            return (bool)$this->conn->setQuery("UPDATE sensors SET location_id = '{$location_id}',
                                                                   name        = '{$name}',
                                                                   is_paired   = true,
                                                                   is_sent     = false
                                                 WHERE sensor = '{$sensor}';
                                                 UPDATE device_config SET start_pairing = false",true)['result'];
        } else {                                                          // edit sensor
            return (bool)$this->conn->setQuery("UPDATE sensors SET name = '{$name}',
                                                                   location_id = '{$location_id}',
                                                                   is_sent = false
                                                WHERE sensor_id = '{$sensor_id}'")['result'];
        }
    }

    /**
     * Get sensor's info by id
     * @param int $sensor_id
     * @return array
     * @throws Exception
     */
    public function getSensorInfo(int $sensor_id) {
        $sensor = $this->conn->getRow("SELECT sensors.name,
                                                    is_main,
                                                    sensors.location_id,
                                                    sensor_locations.name                                                                            AS location_name,
                                                    COALESCE((SELECT round(temp) FROM sensor_logs ORDER BY timestamp DESC LIMIT 1)::varchar,'-')     AS temp,
                                                    COALESCE((SELECT round(humidity) FROM sensor_logs ORDER BY timestamp DESC LIMIT 1)::varchar,'-') AS humidity,
                                                    (SELECT measure_id FROM settings)                                                                AS measure_id
                                             FROM sensors
                                                LEFT JOIN sensor_locations ON sensors.location_id = sensor_locations.location_id
                                                LEFT JOIN sensor_logs      ON sensors.sensor_id = sensor_logs.sensor_id
                                             WHERE sensors.sensor_id = '{$sensor_id}' AND is_remove = false
                                             ORDER BY sensor_logs.timestamp DESC
                                             LIMIT 1");
        $sensor['location_id'] = (int)$sensor['location_id'];

        if($sensor['temp'] !== '-') {
            if((int)$sensor['measure_id'] === 1) {                                      // if measure is fahrenheit
                $sensor['temp'] = round($sensor['temp'] * 9 / 5 + 32);
            }
        }
        unset($sensor['measure_id']);
        if($sensor['is_main'] === 't') {
            $sensor['is_main'] = true;
        } else {
            $sensor['is_main'] = false;
        }
        return $sensor;
    }

    /**
     * Check if exist name
     * @param string $name
     * @return bool
     */
    public function checkSensorName(string $name) {
        if($name !== '') {
            $count = (int)$this->conn->getItem("SELECT count(*) FROM sensors WHERE name = '{$name}'");
            if($count === 0) {
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    /**
     * Remove sensor
     * @param int $sensor_id
     * @return bool
     * @throws Exception
     */
    public function remove(int $sensor_id) {
        return (bool)$this->conn->setQuery("UPDATE sensors SET is_sent = false,is_remove = true,is_paired = false WHERE sensor_id = '{$sensor_id}';
                                                  UPDATE device_config SET forget_sensor = true")['result'];
    }
}