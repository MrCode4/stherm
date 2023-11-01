<?php

namespace project;
ini_set('display_errors', '1');
ini_set('display_startup_errors', '1');
error_reporting(E_ALL);

use core\db\Z_PgSQL;
use Exception;

require_once "./core/db/PgSQL.php";

/**
 * All work with wi-fi
 * @package project
 */
class Wifi
{
    private const WRAP = "/usr/share/apache2/default-site/htdocs/engine/wifi.out";
    private const WRAP_CUR = "/usr/share/apache2/default-site/htdocs/engine/";
    private $conn;
    private const EXEC_TIMEOUT_INTERVAL = 30;

    /**
     * @throws Exception
     */
    public function __construct()
    {
        $this->conn = Z_PgSQL::connection();
    }

    /**
     * Get Wi-FI list
     * @return array
     * @throws Exception
     */
    public function getWifiList()
    {
        exec("timeout " . self::EXEC_TIMEOUT_INTERVAL . " " . self::WRAP . " search", $result, $retval);
        $result[0] = rtrim($result[0], "'");
        $result[0] = trim($result[0], "'");
        $result[0] = json_decode($result[0]);
        if (is_int($result[0])) {
            return [];
        } else {
            $connected = [];
            $connected_wifi = $this->conn->getTable("SELECT essid FROM connected_wifi");
            for ($i = 0; $i < count($connected_wifi); $i++) {
                $connected[] = $connected_wifi[$i]['essid'];
            }
            $list = $result[0]->data;
            usort($list, [new Wifi(), 'signalCompare']);
            for ($i = 0; $i < count($list); $i++) {
                if ($list[$i]->signal != 0) {
                    if($list[$i]->signal!=100){
                        $list[$i]->signal = floor(4 * (25 + $list[$i]->signal) / 100);
                    }else{
                        $list[$i]->signal = 4;
                    }
                }
//                if($list[$i]->signal == 4) {
//                    $list[$i]->signal = 3;
//                }
//                $list[$i]->signal += 1;
                if (in_array($list[$i]->essid, $connected)) {
                    $list[$i]->is_connected_old = true;
                } else {
                    $list[$i]->is_connected_old = false;
                }
            }
            $new_list = $list;

            for ($i = 0; $i < count($list); $i++) {
                for ($j = count($list) - 1; $j > $i; $j--) {
                    if ($list[$i]->essid === $list[$j]->essid) {
                        if ($list[$j]->connected != true) {
                            unset($new_list[$j]);
                        } else {
                            unset($new_list[$i]);
                        }
                    }
                }
            }
            $list = array_values($new_list);
        }
        return $list;
    }

    /**
     * Forgot connected WI-FI
     * @param string $essid
     * @return bool|void
     * @throws Exception
     */
    public function forgot(string $essid)
    {
        exec("timeout " . self::EXEC_TIMEOUT_INTERVAL . " " . self::WRAP . " forget \"$essid\"", $result, $retval);
        if ((int)$result[0] === 0) {
            $this->conn->setQuery("DELETE FROM connected_wifi WHERE essid = '{$essid}' AND is_connected = true");
            Log::setLog('Wifi forgot', 1, "true");
            return true;
        }
        Log::setLog('Wifi forgot', 1, "false");
        return false;
    }

    /**
     * Connect WI-FI with essid and password
     * @param string $essid
     * @param string $unique_name
     * @param string $password
     * @return string
     * @throws Exception
     */
    public function connect(string $essid, string $unique_name, string $password): string
    {
        if ($password === '') {
            $password = $this->conn->getItem("SELECT password FROM connected_wifi WHERE essid = '{$essid}'");
        }
        $wrap = self::WRAP;
        $output = shell_exec("timeout " . self::EXEC_TIMEOUT_INTERVAL . " " . "{$wrap} connect \"{$essid}\" \"{$password}\"");

        $output = (int)trim($output);
        $error = $output;

        Log::setLog('Wifi connect', 1, $error);

        switch ($error) {
            case 0:
                $this->conn->setQuery("INSERT INTO connected_wifi(essid,unique_name,password,is_connected)
                                             VALUES('{$essid}','{$unique_name}','{$password}',true)
                                              ON CONFLICT (unique_name) DO UPDATE SET is_connected = true");
                return 'Connected';
            case 1:
                return 'Unknown or unspecified error';
            case 2:
                return 'Invalid user input, wrong nmcli invocation';
            case 3:
                return 'Timeout expired';
            case 4:
                return 'Connection activation failed';
            case 5:
                return 'Connection deactivation failed';
            case 6:
                return 'Disconnecting device failed';
            case 7:
                return 'Connection deletion failed';
            case 8:
                return 'NetworkManager is not running';
            case 10:
                return 'Connection, device, or access point does not exist';
            case 11:
                return 'Wrong BSSID';
            case 12:
                return 'Wrong name';
            case 13:
                return 'Wrong password';
            case 14:
                return 'No active Wi-Fi found';
            case 15:
                return 'Wrong arguments';
            case 16:
                return 'Internal error: critical delete';
            case 17:
                return 'Internal error';
            case 18:
                return 'Connection already exists';
            case 19:
                return 'Internal error: parsing';
            case 20:
                return 'Wrong subnet';
            case 21:
                return 'Wrong IP';
            case 22:
                return 'Wrong ESSID';
            default:
                return 'Unknown error';
        }
    }

    /**
     * Disconnect connected WI-FI
     * @param string $essid
     * @param string $unique_name
     * @return bool
     * @throws Exception
     */
    public function disconnect(string $essid, string $unique_name): bool
    {
        exec("timeout " . self::EXEC_TIMEOUT_INTERVAL . " " . self::WRAP . " disconnect \"{$essid}\"", $result, $retval);
        if ($result[0] != 0) {
            Log::setLog('Wifi disconnect', 1, "false");
            return false;
        }
        (bool)$this->conn->setQuery("UPDATE connected_wifi SET is_connected = false 
                                           WHERE essid = '{$essid}' AND unique_name = '{$unique_name}'")['result'];
        Log::setLog('Wifi disconnect', 1, "true");
        return true;
    }

    /**
     * Manual connect to WI-FI
     * @param string $unique_name
     * @param string $essid
     * @param string $ip
     * @param string $subnet
     * @param string $gtw
     * @param string $dns1
     * @param string $dns2
     * @param string $password
     * @return bool
     * @throws Exception
     */
    public function manualConnect(string $unique_name, string $essid, string $ip, string $subnet, string $gtw, string $dns1, string $dns2, string $password)
    {
        $pattern = "/^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/";
        if (empty($unique_name) || empty($essid) || empty($ip) || empty($gtw) || empty($password)) {
            Log::setLog('Wifi manualConnect', 2, "Empty data");
            throw new Exception("Empty data");
        }
        if (preg_match_all($pattern, $ip) !== 1) {
            Log::setLog('Wifi manualConnect', 2, "IP error");
            throw new Exception("IP error");
        }
        if (!empty($subnet) && preg_match_all($pattern, $subnet) !== 1) {
            Log::setLog('Wifi manualConnect', 2, "Subnet error");
            throw new Exception("Subnet error");
        }
        if (preg_match_all($pattern, $gtw) !== 1) {
            Log::setLog('Wifi manualConnect', 2, "Gateway error");
            throw new Exception("Gateway error");
        }
        if (!empty($dns1) && preg_match_all($pattern, $dns1) !== 1) {
            Log::setLog('Wifi manualConnect', 2, "DNS 1 error");
            throw new Exception("DNS 1 error");
        }
        if (!empty($dns2) && preg_match_all($pattern, $dns2) !== 1) {
            Log::setLog('Wifi manualConnect', 2, "DNS 2 error");
            throw new Exception("DNS 2 error");
        }
        $output = shell_exec("timeout " . self::EXEC_TIMEOUT_INTERVAL . " " . self::WRAP . ' manual "' . 'essid:' . $essid . ' ip:' . $ip . ' subnet:' . $subnet . ' gtw:' . $gtw . ' dns:' . $dns1 . ' dns2:' . $dns2 . ' pass:' . $password . '"');
        $output = (int)trim($output);
        $error = $output;
        Log::setLog('Wifi manualConnect', 1, $error);
        switch ($error) {
            case 0:
                $this->conn->setQuery("INSERT INTO connected_wifi(essid,unique_name,password,is_connected) 
                                        VALUES('{$essid}','{$unique_name}','{$password}',true)");
                return 'Connected';
            case 1:
                return 'Unknown or unspecified error';
            case 2:
                return 'Invalid user input, wrong nmcli invocation';
            case 3:
                return 'Timeout expired';
            case 4:
                return 'Connection activation failed';
            case 5:
                return 'Connection deactivation failed';
            case 6:
                return 'Disconnecting device failed';
            case 7:
                return 'Connection deletion failed';
            case 8:
                return 'NetworkManager is not running';
            case 10:
                return 'Connection, device, or access point does not exist';
            case 11:
                return 'Wrong BSSID';
            case 12:
                return 'Wrong name';
            case 13:
                return 'Wrong password';
            case 14:
                return 'No active Wi-Fi found';
            case 15:
                return 'Wrong arguments';
            case 16:
                return 'Internal error: critical delete';
            case 17:
                return 'Internal error';
            case 18:
                return 'Connection already exists';
            case 19:
                return 'Internal error: parsing';
            case 20:
                return 'Wrong subnet';
            case 21:
                return 'Wrong IP';
            case 22:
                return 'Wrong ESSID';
            default:
                return 'Unknown error';
        }
    }

    private function signalCompare($a, $b)
    {
        return ($a->signal < $b->signal);
    }
}
