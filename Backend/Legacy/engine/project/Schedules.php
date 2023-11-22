<?php

namespace project;

use core\db\Z_PgSQL;
use Exception;

class Schedules
{
    /** @var Z_PgSQL */
    private Z_PgSQL $conn;
    /** @var int schedule ID in Database */
    private int $id;
    /**
     * @var string schedule name
     * @example 'noname'
     */
    private string $name;
    /**
     * @var object  schedule start and stop time
     * @example (object)['start' => '00:00', 'end' => '00:00']
     */
    private object $time;
    /**
     * @var bool[] witch weekdays schedule work, weekday selected by start time at list one must be true
     * @example  [true,false,false,false,false,false,false]
     */
    private array $weekdays;
    /**
     * @var string "off|auto|cooling|heating"
     * @example 'auto'
     */
    private string $mode; // Don't use in current revision
    /** @var float schedule temperature in Celsius with 2 decimal  from 18 to 30 */
    private float $temperature;
    /** @var int schedule humidity in percent from 20% to 100% */
    private int $humidity; // Don't use in current revision
    /** @var bool enabled or disabled state */
    private bool $state;
    /**
     * @var string "away|home|sleep|custom"
     * @example 'away'
     */
    private string $type;
    /** @var bool is current schedule need remove */
    private bool $removed;

    /**
     * Constructor Get data From DB
     * @param $id
     * @throws Exception
     */
    public function __construct($id)
    {
        $this->conn = Z_PgSQL::connection();
        $schedule = $this->conn->getRow("SELECT schedules.schedule_id AS id schedules.name AS 'name', start_time, end_time, is_enable, temp, d0, d1, d2, d3, d4, d5, d6, schedule_types.alias AS type, is_remove FROM schedules JOIN schedule_types on schedules.type_id = schedule_types.type_id WHERE schedule_id = $id AND is_remove = true");
        if ($schedule !== []) {
            $this->id = $schedule['id'];
            $this->name = $schedule['name'];
            $this->type = $schedule['type'];

            $this->time = (object)['start' => $schedule['start_time'], 'end' => $schedule['end_time']];

            $this->weekdays[0] = $schedule['d0'];
            $this->weekdays[1] = $schedule['d1'];
            $this->weekdays[2] = $schedule['d2'];
            $this->weekdays[3] = $schedule['d3'];
            $this->weekdays[4] = $schedule['d4'];
            $this->weekdays[5] = $schedule['d5'];
            $this->weekdays[6] = $schedule['d6'];

            $this->mode = 'auto'; // in schedule don't use other mode
            $this->temperature = $schedule['temp'];
            $this->humidity = 0; // in schedule don't use humidity
            $this->state = $schedule['is_enable'];
            $this->removed = $schedule['is_remove'];
        } else { // default values if new schedule
            $this->id = 0;
            $this->name = 'noname';
            $this->type = 'home';

            $this->time = (object)['start' => '00:00', 'end' => '00:00'];

            $this->weekdays[0] = 'f'; // Sunday
            $this->weekdays[1] = 'f'; // Monday
            $this->weekdays[2] = 'f';
            $this->weekdays[3] = 'f';
            $this->weekdays[4] = 'f';
            $this->weekdays[5] = 'f';
            $this->weekdays[6] = 'f';

            $this->mode = 'auto';
            $this->temperature = 24;
            $this->humidity = 0;
            $this->state = false;
            $this->removed = false;
        }

    }

    /**
     * Destructor last time update schedule in DB
     * @throws Exception
     */
    public function __destruct()
    {
        $this->updateSchedule();
    }

    public function __toString()
    {
        // name='{}', type_id = {$type_id}, start_time = '{$this->time->start}', end_time = '{$this->time->end}', is_remove = $this->removed, is_enable = $this->state, temp = $this->temperature, d0 = $this->weekdays[0], d1 = $this->weekdays[1], d2 = $this->weekdays[2], d3 = $this->weekdays[3], d4 = $this->weekdays[4], d5 = $this->weekdays[5], d6 = $this->weekdays[6]
        $schedule_str = 'ID -> ' . $this->id . "\n";
        $schedule_str .= 'Name -> ' . $this->name . "\n";
        $schedule_str .= 'Type -> ' . $this->type . "\n" . "\n";

        $schedule_str .= 'Time -> start -> ' . $this->time->start . "\n";
        $schedule_str .= 'Time -> end -> ' . $this->time->start . "\n";
        $schedule_str .= 'Weekdays -> ' . $this->weekdays[0] . $this->weekdays[1] . $this->weekdays[2] . $this->weekdays[3] . $this->weekdays[4] . $this->weekdays[5] . $this->weekdays[6] . "\n" . "\n";

        $schedule_str .= 'Mode -> ' . $this->mode . "\n";
        $schedule_str .= 'Temperature -> ' . $this->temperature . "\n";
        $schedule_str .= 'Humidity -> ' . $this->humidity . "\n" . "\n";

        $schedule_str .= 'State -> ' . $this->state . "\n";
        $schedule_str .= 'Is Removed -> ' . $this->removed;

        return $schedule_str;
    }

    /**
     * Set Data To DB
     * @throws Exception
     */
    public function updateSchedule()
    {
        $this->conn = Z_PgSQL::connection();
        $type_id = (int)$this->conn->getItem("SELECT type_id FROM schedule_types WHERE alias = '$this->type'");
        if ($this->id > 0) {
            $this->conn->setQuery("UPDATE schedules SET name='$this->name', type_id = $type_id, start_time = '$this->time->start', end_time = '{$this->time->end}', is_remove = $this->removed, is_enable = $this->state, temp = $this->temperature, d0 = $this->weekdays[0], d1 = $this->weekdays[1], d2 = $this->weekdays[2], d3 = $this->weekdays[3], d4 = $this->weekdays[4], d5 = $this->weekdays[5], d6 = $this->weekdays[6] WHERE schedule_id = $this->id");
        } else {
            $this->conn->setQuery("INSERT INTO schedules(name, type_id, start_time, end_time, is_enable, temp, d0, d1, d2, d3, d4, d5, d6) VALUES('$this->name', $type_id, '$this->time->start', '$this->time->end', $this->state, $this->temperature, $this->weekdays[0], $this->weekdays[1], $this->weekdays[2], $this->weekdays[3], $this->weekdays[4], $this->weekdays[5], $this->weekdays[6])");
        }
    }

    /**
     * ID Getter
     * @return int
     */
    public function getId(): int
    {
        return $this->id;
    }

    /**
     * Name Getter
     * @return string
     */
    public function getName(): string
    {
        return $this->name;
    }

    /**
     * Name Setter
     * @param string $name
     * @return bool
     * @throws Exception
     */
    public function setName(string $name): bool
    {
        $names = $this->conn->getTable("SELECT name FROM schedules");
        $is_repeat = false;
        for ($i = 0; $i < count($names); $i++) {
            if (strtolower($names[$i]['name']) === strtolower($name)) {
                $is_repeat = true;
                break;
            }
        }

        if ($is_repeat) {
            return false;
        }
        $this->name = $name;
        return true;
    }

    /**
     * Schedule work time Getter
     * @return object
     * @example [start->'00:00', end->'00:00']
     */
    public function getTime(): object
    {
        return $this->time;
    }

    /**
     * Schedule work time Getter
     * @param object $time
     * @param bool $is_enable
     * @throws Exception
     * @example [start->'00:00', end->'00:00']
     */
    public function setTime(object $time, bool $is_enable = true): void
    {
        $this->time = $time;
        if ($is_enable) {
            $overlapped = $this->checkOverlapping();
            $this->state = true;

            $overlapped_str = implode(',', $overlapped);
            $this->conn->setQuery("UPDATE schedules SET is_enable = false WHERE schedule_id IN($overlapped_str)");
        } else {
            $this->state = false;
        }
    }

    /**
     * Weekdays array Getter
     * @return array
     * @example [false,false,false,false,false,false,false]
     */
    public function getWeekdays(): array
    {
        return $this->weekdays;
    }

    /**
     * Weekdays setter
     * @param array $weekdays
     * @param bool $is_enable default value true
     * @return bool
     * @throws Exception
     * @example [false, true, false, false, false, false, false]
     */
    public function setWeekdays(array $weekdays, bool $is_enable = true): bool
    {
        if (count($weekdays) !== 7) {
            return false;
        }
        $has_selected = false;
        for ($i = 0; $i < count($weekdays); $i++) {
            if ($weekdays[$i] === true) {
                $has_selected = true;
                break;
            }
        }
        if (!$has_selected) {
            return false;
        }
        $this->weekdays = $weekdays;
        if ($is_enable) {
            $overlapped = $this->checkOverlapping();
            $overlapped_str = implode(',', $overlapped);
            $this->conn->setQuery("UPDATE schedules SET is_enable = false WHERE schedule_id IN($overlapped_str)");
            return true;
        }
        $this->state = false;
        return true;
    }

    /**
     * Mode alias Getter
     * @return string
     */
    public function getModeAlias(): string
    {
        return $this->mode;
    }

    /**
     * Mode ID Getter
     * @return int
     */
    public function getModeId(): int
    {
        return (int)$this->conn->getItem("SELECT mode_id FROM mode WHERE alias = '$this->mode'");
    }

    /**
     * Mode Setter by alias
     * @param string $mode
     */
    public function setMode(string $mode): void
    {
        $this->mode = $mode;
    }

    /**
     * Mode Setter by ID
     * @param int $mode_id
     * @return void
     */
    public function setModeById(int $mode_id): void
    {
        $this->mode = $this->conn->getItem("SELECT alias FROM mode WHERE mode_id = $mode_id");
    }

    /**
     * Temperature Getter with 2 number of digits after the decimal point
     * @param string $measure_type - default value 'c'
     * @return float
     * @example $measure_type = 'c' or 'C' // 'f' or 'F' - default value 'c'
     */
    public function getTemperature(string $measure_type = 'c'): float
    {
        if (strtolower($measure_type) === 'f') {
            return round($this->temperature * 9 / 5 + 32, 2);
        }
        return $this->temperature;
    }

    /**
     * Temperature Setter
     * @param float $temperature
     * @param string $measure_type - default value 'c'
     * @example $measure_type = 'c' or 'C' // 'f' or 'F' - default value 'c'
     */
    public function setTemperature(float $temperature, string $measure_type = 'c'): void
    {
        if (strtolower($measure_type) === 'f') {
            $this->temperature = round(($temperature - 32) * 5 / 9, 2);
            return;
        }
        $this->temperature = $temperature;
    }

    /**
     * Humidity Getter
     * @return int
     */
    public function getHumidity(): int
    {
        return $this->humidity;
    }

    /**
     * Humidity Setter
     * @param int $humidity
     * @return bool
     */
    public function setHumidity(int $humidity): bool
    {
        if ($humidity < 20 || $humidity > 70) {
            return false;
        }
        $this->humidity = $humidity;
        return true;
    }

    /**
     * Get is schedule enabled
     * @return bool
     */
    public function isEnabled(): bool
    {
        return $this->state;
    }

    /**
     * Enable or disable schedule
     * @param bool $state
     * @throws Exception
     */
    public function setState(bool $state): void
    {
        if ($state) {
            $overlapped = $this->checkOverlapping();
            $overlapped_str = implode(',', $overlapped);
            $this->conn->setQuery("UPDATE schedules SET is_enable = false WHERE schedule_id IN($overlapped_str)");
        }
        $this->state = $state;
    }

    /**
     * Get schedule type Alias
     * @return string
     */
    public function getTypeAlias(): string
    {
        return $this->type;
    }

    /**
     * Get schedule type Name
     * @return string
     */
    public function getTypeName(): string
    {
        return $this->conn->getItem("SELECT name FROM schedule_types WHERE alias = '$this->type'");
    }

    /**
     * Get schedule type ID
     * @return string
     */
    public function getTypeId(): string
    {
        return $this->conn->getItem("SELECT type_id FROM schedule_types WHERE alias = '$this->type'");
    }

    /**
     * Set schedule Type
     * @param string $type
     */
    public function setType(string $type): void
    {
        $this->type = $type;
    }

    /**
     * Set schedule Type by ID
     * @param int $type_id
     */
    public function setTypeById(int $type_id): void
    {
        $this->type = $this->conn->getItem("SELECT alias FROM schedule_types WHERE type_id = $type_id");
    }

    /**
     * Set schedule removed
     * @return void
     */
    public function remove(): void
    {
        $this->removed = true;
    }

    /**
     * Get List if schedules id with overlapping with current schedule
     * @throws Exception
     */
    public function checkOverlapping(): array
    {
        $query = "SELECT schedule_id
              FROM schedules
              WHERE schedules.is_remove = false AND schedule_id != $this->id AND is_enable = true AND
                                                      (((('{$this->time->start}' > start_time AND '{$this->time->start}' < end_time) OR
                                                       ('{$this->time->end}' > start_time AND '{$this->time->end}' < end_time) OR
                                                       ('{$this->time->start}' < start_time AND '{$this->time->end}' > end_time) OR  
                                                       ('{$this->time->start}' = start_time AND '{$this->time->end}' = end_time) OR
                                                       ('{$this->time->end}' = end_time AND '{$this->time->end}' > start_time) OR
                                                       ('{$this->time->start}' > '{$this->time->end}' AND '{$this->time->start}' < start_time AND '{$this->time->end}' < start_time) OR
                                                       ('{$this->time->start}' > '{$this->time->end}' AND '{$this->time->start}' > end_time AND '{$this->time->end}' > end_time) OR
                                                       (end_time < start_time AND '{$this->time->start}' > start_time)) AND (";
        for ($i = 0; $i < count($this->weekdays); $i++) {
            if ($this->weekdays[$i] === true) {
                $query .= "d" . $i . "=true OR ";
            }
        }
        $query = rtrim($query, ' OR');
        $query .= ")) OR (end_time < start_time AND '{$this->time->start}' < end_time AND (";
        for ($i = 0; $i < count($this->weekdays); $i++) {
            if ($this->weekdays[$i] === true) {
                if ($i === 0) {
                    $week_day = 6;
                } else {
                    $week_day = $i - 1;
                }
                $query .= "d" . $week_day . "=true OR ";
            }
        }
        $query = rtrim($query, ' OR');
        $query .= ")))";
        $list = $this->conn->getTable($query);
        $clear = [];
        for ($i = 0; $i < count($list); $i++) {
            $clear[$i] = $list[$i]['schedule_id'];
        }
        return $clear;
    }
}