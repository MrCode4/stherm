<?php
namespace project;

use Exception;
use core\db\Z_PgSQL;

//require_once "/usr/share/apache2/default-site/htdocs/engine/project/Sync.php";
require_once "/usr/share/apache2/default-site/htdocs/engine/project/System.php";
require_once "/usr/share/apache2/default-site/htdocs/engine/core/db/PgSQL.php";

/**
 * Check update on server and install full or partial
 * @package project
 */
class Update
{
    private const WEB_URL = "http://test.hvac.z-soft.am/engine/index.php";

    /**
     * @throws Exception
     */
    public function __construct() {
        $this->conn = Z_PgSQL::connection();
    }

    /**
     * Get software update info
     * @return array
     * @throws Exception
     */
    public function getSoftwareUpdateInfo() {
        $soft_v = parse_ini_file($_SERVER['DOCUMENT_ROOT'] . '/configuration/version.ini')['SOFTWARE_VERSION'];
        $configs = $this->conn->getRow("SELECT uid,
                                                     (SELECT date(soft_update_timestamp) FROM timing) AS date,
                                                     serial_number
                                              FROM device_config");
        $data = json_encode(
            [
                'request' =>
                    [
                        'class'  => 'sync',
                        'method' => 'getDeviceAvailableVersion',
                        'params' => [$configs['serial_number'],$soft_v]
                    ],
                'user' =>
                    [
                        'lang_id'   => 0,
                        'user_id'   => 0,
                        'type_id'   => 0,
                        'host_id'   => 0,
                        'region_id' => 0,
                        'token'     => ''
                    ]
            ]);
        $curl = curl_init();
        curl_setopt_array($curl, array(
            CURLOPT_URL => self::WEB_URL,
            CURLOPT_FAILONERROR => true,
            CURLOPT_RETURNTRANSFER => true,
            CURLOPT_ENCODING => '',
            CURLOPT_MAXREDIRS => 10,
            CURLOPT_TIMEOUT => 0,
            CURLOPT_FOLLOWLOCATION => true,
            CURLOPT_HTTP_VERSION => CURL_HTTP_VERSION_1_1,
            CURLOPT_CUSTOMREQUEST => 'POST',
            CURLOPT_POSTFIELDS => $data,
        ));
        $response = curl_exec($curl);
        if(curl_errno($curl)) {
            $error_msg = curl_error($curl);
        }
        curl_close($curl);
        $is_enable = false;
        if(!isset($error_msg)) {
            if($response !== []) {
                $available_sv = json_decode($response)->result->result;
                if($available_sv !== false) {
                    //if(substr($available_sv,2,1) === 'r') {
                    //    $this->install();
                   // } else if(substr($available_sv,2,1) === 'u'){
                    $available_sv = substr($available_sv,3,3);
                    $is_enable = true;
                   // }
                } else {
                    $available_sv = 'This version is up to date';
                }
            } else {
                $available_sv = 'This version is up to date';
            }
        } else {
            $available_sv = false;
        }
        return [
                'soft_v'    => $soft_v,
                'uid'       => $configs['uid'],
                'date'      => $configs['date'],
                'version'   => $available_sv,
                'info'      => '',
                'is_enable' => $is_enable
        ];
    }

    /**
     * Install new version
     * @return false|string|null
     * @throws Exception
     */
    public function install() {
        $serial_number = $this->conn->getItem("SELECT serial_number FROM device_config");
        return (new System())->getUpdate($serial_number);
    }

    /**
     * Cancel installing
     * @return bool
     */
    public function cancel() {
        return true;
    }
}