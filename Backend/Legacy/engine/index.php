<?php declare(strict_types=1);
/**
 * Copyright (c) 2020. Z-Soft All right reserved
 * @author  Suren Danielyan
 */
//ini_set('display_errors', '0');
//ini_set('display_startup_errors', '0');
error_reporting(0);

define("SYSTEM_SETTINGS", parse_ini_file('..' . DIRECTORY_SEPARATOR . 'configuration' . DIRECTORY_SEPARATOR . 'system.ini'));

const LOGOUT = 0;
const ERROR = 1;
const WARNING = 2;
const MESSAGE = 3;
const DEBUG = 4;
const ANSWER = 5;

//const REPEAT_LIST = [['class' => 'calendar', 'method' => 'getEvents'],];
const WITHOUT_LOGIN = ['login'];
const REPEAT_LIST = [];

require_once "core/Autoloader.php";

use core\API;
use core\Frontend;
use core\User;

if (session_status() == PHP_SESSION_ACTIVE) {
    session_abort();
}

session_start();
$api = new API();

/**
 * Execute front command
 * @param API $api
 * @return void
 * @throws Exception
 */
function execute(API $api): API
{
    $fn_result = Frontend::execute($api->class_name, $api->method_name, $api->params);

    if (gettype($fn_result) === 'object') {
        if (isset($fn_result->type) && isset($fn_result->message) && isset($fn_result->result)) {
            $api->type = $fn_result->type;
            $api->setMessage($fn_result->message);
            $api->result = $fn_result->result;
        } else {
            $api->type = ANSWER;
            $api->setMessage("ok");
            $api->result = $fn_result;
        }
    } else {
        $api->type = ANSWER;
        $api->message = "ok";
        $api->result = $fn_result;
    }
    if (isset($_SESSION['user_id'])) {
        $api->user_id = intval($_SESSION['user_id']);
    }
    if (isset($_SESSION['host_id'])) {
        $api->host_id = intval($_SESSION['host_id']);
    }
    if (isset($_SESSION['type_id'])) {
        $api->type_id = intval($_SESSION['type_id']);
    }
    if (isset($_SESSION['token'])) {
        $api->token = $_SESSION['token'];
    }
    if (isset($_SESSION['lang_id'])) {
        $api->lang_id = intval($_SESSION['lang_id']);
    }
    if (isset($_SESSION['region_id'])) {
        $api->region_id = intval($_SESSION['region_id']);
    }
    return $api;
}

/**
 * check is repeat command
 * @param API $api
 * @return bool
 */
function isRepeat(API $api): bool
{
    for ($i = 0; $i < count(REPEAT_LIST); $i++) {
        if ($api->class_name === REPEAT_LIST[$i]['class'] && $api->method_name === REPEAT_LIST[$i]['method']) {
            return true;
        }
    }
    return false;
}

try {
    $data = Frontend::getDataFromFrontend();
    $api = new API($data);
    if (SYSTEM_SETTINGS['HAS_USER']) {
        $is_login = false;
        for($i=0; $i<count(WITHOUT_LOGIN); $i++){
            if($api->class_name === WITHOUT_LOGIN[$i]){
                $is_login = true;
                break;
            }
        }
        if($is_login){
            if ($api->user_id !== 0 && $api->token !== "") {
                User::checkUser($api->user_id, $api->token, $api->host_id, $api->type_id, $api->region_id, $api->lang_id);
                if (!isRepeat($api)) {
                    User::setSession($api->lang_id, $api->region_id, $api->user_id, $api->host_id, $api->type_id, $api->token);
                }
                $api = execute($api);
            } else {
                if ($api->class_name === 'user' || $api->class_name === 'device') { // on device not need check user
                    $api = execute($api);
                } else {
                    $api->type = LOGOUT;
                    $api->setMessage("Please login");
                    $api->result = [];
                }
            }
        }else{
            $api = execute($api);
        }
    } else {
        User::setSession($api->lang_id, $api->region_id, $api->user_id, $api->host_id, $api->type_id, $api->token);
        $api = execute($api);
    }
} catch (Throwable $e) {
    $api->type = ERROR;
    $api->message = $e->getMessage() . '<br>' . $e->getFile() . '<br>' . $e->getLine();
} finally {
    echo $api;
    unset($api);
    gc_collect_cycles();
}