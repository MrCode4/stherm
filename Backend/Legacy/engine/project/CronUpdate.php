<?php

namespace project;
use Exception;
use project\Update;
use core\db\Z_PgSQL;
require_once "../core/db/PgSQL.php";
require_once "Update.php";

ini_set('display_errors', '1');
ini_set('display_startup_errors', '1');
error_reporting(E_ALL);

class CronUpdate
{
    public function __construct()
    {
        $this->conn = Z_PgSQL::connection();
        $this->update = new Update();
    }

    /**
     * Require update
     * @return void
     * @throws Exception
     */
    public function requireUpdate() {
        $this->update->install();
    }

    /**
     * Clear old logs and alerts
     * @return void
     * @throws Exception
     */
    public function clearDBOldInfo() {
        $is_passed = (int)$this->conn->getItem("SELECT
                                                          CASE
                                                              WHEN EXTRACT(epoch FROM current_timestamp - delete_info_timestamp)/86400 > delete_info_interval THEN 1
                                                              ELSE 0
                                                          END 
                                                      FROM timing");
        if($is_passed === 1) {
            $this->conn->setQuery("DELETE FROM alerts;
                                         DELETE FROM system_errors;
                                         DELETE FROM messages;
                                         DELETE FROM relay_logs;
                                         DELETE FROM sensor_logs;
                                         UPDATE timing SET delete_info_timestamp = current_timestamp");
        }
    }

}

$cron_update = new CronUpdate();
$cron_update->clearDBOldInfo();
$cron_update->requireUpdate();
