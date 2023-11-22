<?php

namespace project;

use core\db\Z_PgSQL;
use Exception;

require_once "defines.php";
require_once "/usr/share/apache2/default-site/htdocs/engine/core/db/PgSQL.php";

class Sensor
{
    /** @var Z_PgSQL */
    private $conn;

    /**
     * Constructor get data from DB
     * @throws Exception
     */
    public function __construct()
    {
        $this->conn = Z_PgSQL::connection();
    }
}