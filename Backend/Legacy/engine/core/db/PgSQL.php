<?php
/**
 * Copyright (c) 2020. Z-Soft All right reserved
 * @author Victor Ohanyan
 */

namespace core\db;
//var_dump(SYSTEM_SETTINGS);
//$system_settings = parse_ini_file($_SERVER['DOCUMENT_ROOT'] . DIRECTORY_SEPARATOR . 'configuration' . DIRECTORY_SEPARATOR . 'system.ini');
//var_dump($system_settings['APP_DEBUG_MODE']);
//define('DOCUMENT_ROOT',$_SERVER['DOCUMENT_ROOT']);
//require "../../init.php";
//define('SYSTEM_SETTINGS', parse_ini_file($_SERVER['DOCUMENT_ROOT'] . DIRECTORY_SEPARATOR . 'configuration' . DIRECTORY_SEPARATOR . 'system.ini'));
//var_dump(SYSTEM_SETTINGS);


use Exception;

/**
 * Class Z_PgSQL
 * @package core\db
 */

class Z_PgSQL
{
    /* @var  Z_PgSQL connection */
    private $connection;
    /* @var Z_PgSQL[] */
    private static $instance = [];

   // private const TIMEZONE = 'Asia/Yerevan';
    private const MAIN_ROOT = '/usr/share/apache2/default-site/htdocs/';

    private $system_settings;
    /**
     * Z_PgSQL constructor
     * @param string $configName
     * @throws Exception
     */
    private function __construct(string $configName)
    {
        //$this->system_settings = parse_ini_file('.' . DIRECTORY_SEPARATOR . 'configuration' . DIRECTORY_SEPARATOR . 'system.ini');
        $this->system_settings = parse_ini_file(self::MAIN_ROOT . '/configuration/system.ini');
       // $this->system_settings = parse_ini_file('/usr/share/apache2/default-site/htdocs/configuration/system.ini');
        $this->connection = NULL;
        //$filename = '.' . DIRECTORY_SEPARATOR . 'configuration' . DIRECTORY_SEPARATOR . 'database.ini';
        $filename = self::MAIN_ROOT . 'configuration/database.ini';
        $config = parse_ini_file($filename, true);
        if(empty($config)) throw new Exception("Config file not found");
        $configuration_data = $config[$configName];
        $this->connection = pg_connect("host='{$configuration_data['HOST']}' port={$configuration_data['PORT']} dbname='{$configuration_data['NAME']}' user='{$configuration_data['USERNAME']}' password='{$configuration_data['PASSWORD']}' options='--client_encoding=UTF8' connect_timeout=50");
        $timezone = $this->getItem("SELECT timezone FROM device_config");
        if($timezone !== '' && $timezone != null) {
            $query = "SET TIMEZONE = '" . $timezone . "'";
            $this->setQuery($query);
        }
        if(empty($this->connection)) throw new Exception("Connection failure ");
    }

    /**
     * Z_PgSql destructor.- disconnect from PgSql
     */
    public function __destruct()
    {
        if ($this->connection) {
            pg_close($this->connection);
        }
    }

    /**
     * PgSQL Select database with config and connect
     * @param string $configName
     * @return Z_PgSQL
     * @throws Exception
     */
    public static function connection(string $configName = 'default_connection'): Z_PgSQL
    {

        if (empty(static::$instance[$configName]) || static::$instance[$configName] === null) {
            static::$instance[$configName] = new static($configName);
        }
        return static::$instance[$configName];
    }

    /**
     * Return Database date and time
     * @return string
     */
    public final function getNow(): string
    {
        return $this->getItem("SELECT NOW() AT TIME ZONE current_setting('TimeZone') AS time;");
    }

    /**
     * Return One item
     * @param string $query
     * @return string
     */
    public final function getItem(string $query): string
    {
        $result = pg_fetch_assoc(pg_query($this->connection, $query));
        if($result){
            if (!empty($result[array_keys($result)[0]])) {
                return $result[array_keys($result)[0]];
            }
        }
        return '';
    }

    /**
     * Get one row from database in array if there are many rows returned only first row
     * @param string $query
     * @return array associative
     * @throws Exception
     */
    public final function getRow(string $query): array
    {
        try {
            $result = pg_fetch_assoc(pg_query($this->connection, $query));
            if(empty($result)){
                return [];
            }
            return $result;
        } catch (Exception $exception) {
            throw new Exception("Wrong query");
        }
    }

    /**
     * Get many rows from db in object or array
     * @param string $query
     * @return array
     * @throws Exception
     */
    public final function getTable(string $query): array
    {
        try {
            $data =  pg_fetch_all(pg_query($this->connection, $query));
            if(!empty($data)){
                return $data;
            }
            return [];
        } catch (Exception $exception) {
            throw new Exception("Wrong query");
        }
    }

    /**
     * DML query (for INSERT, UPDATE or DELETE)
     * @param string $query
     * @param bool $transaction ;
     * @return array [bool 'result', array 'return_data']
     * @throws Exception
     */
    public final function setQuery(string $query, bool $transaction = false): array
    {
        try{
            if($transaction){
                $query = " BEGIN; " . $query;
                $result = pg_query($this->connection, $query);
                if ($result) {
                    pg_query($this->connection,"COMMIT;");
                    if($this->system_settings['STATE'] === 'DEBUG'){
                        return ['result' => true, 'return_data' =>[],'query'=>$query];
                    }
                    return ['result' => true, 'return_data' =>[]];
                } else {
                    pg_query($this->connection,"ROLLBACK;");
                    if($this->system_settings['STATE'] === 'DEBUG'){
                        return ['result' => true, 'return_data' =>[],'query'=>$query];
                    }
                    return ['result' => false, 'return_data' =>[]];
                }
            }
            $result = pg_query($this->connection, $query);
            if($this->system_settings['STATE'] === 'DEBUG'){
                return ['result' => pg_affected_rows($result), 'return_data' =>pg_fetch_object($result),'query'=>$query];
            }
            return ['result' => pg_affected_rows($result), 'return_data' => pg_fetch_object($result)];
        }catch (Exception $exception){
            throw new Exception("Wrong query");
        }
    }
}