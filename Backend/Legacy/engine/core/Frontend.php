<?php

namespace core;

use API;
use Exception;
use http\Env\Request;

final class Frontend
{
    /**
     * Get Sent from Frontend
     * @param string $data
     * @return object
     * @throws Exception
     */
    public static function getDataFromFrontend(string $data = "000"):object
    {
        if($data==="000"){
            $all_data = file_get_contents('php://input');
        }else{
            $all_data = $data;
        }
        if (empty($all_data)) { // Empty Data
            throw new Exception("Empty JSON Data");
        }
        $income_data = json_decode($all_data);
        if (json_last_error() !== JSON_ERROR_NONE) { // WRONG DATA
            throw new Exception(json_last_error_msg());
        }
        return $income_data;
    }

    /**
     * Send message or data to Frontend
     * @param object $answer
     */
    public static function sendToFrontend(object $answer)
    {
        $user = self::getUserData();
        $request = self::getFrontRequest();
        $result = (object)["user" => $user, "result" => $answer, "request"=>$request];

        echo json_encode($result);
    }

    /**
     * Select class, method and run it
     * @param string $class
     * @param string $method
     * @param array $params
     * @return false|mixed
     * @throws Exception
     */
    public static function execute(string $class, string $method, array $params = [])
    {
        $structure = (object)parse_ini_file('..' . DIRECTORY_SEPARATOR . 'configuration' . DIRECTORY_SEPARATOR . 'structure.ini');
        if (!property_exists($structure, $class)) {
            throw new Exception("Class not found in structure", ERROR);
        }
        $class = $structure->{$class};
        if (!class_exists($class)) {
            throw new Exception("Class $class not found", ERROR);
        }
        $execute = new $class();
        if (!method_exists($execute, $method) || !is_callable(array($execute, $method))) {
            throw new Exception("Method not found", ERROR);
        }
        return call_user_func_array([$execute, $method], $params);
    }

    /**
     * Get sent user data
     * @return array
     */
    public static function getUserData(): array
    {
        //$user = ["lang_id"=>0, "region_id"=>0, "user_id"=>0, "type_id"=>0, "host_id"=>0, "token"=>"1"];
        return ["lang_id" => 0, "region_id" => 0, "user_id" => 0, "type_id" => 0, "host_id" => 0, "token" => "1"];
    }

    /**
     * Get sent request
     * @return object
     */
    public static function getFrontRequest(): object
    {
        $request = ["class" => '', "method" => '', "params" => []];
        if(isset($_SESSION['request'])){
            $request = json_decode($_SESSION['request']);
        }
        return (object)$request;
    }
}
