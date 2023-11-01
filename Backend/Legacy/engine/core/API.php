<?php

namespace core;

use Exception;

class API
{
    public $lang_id;
    public $region_id;
    public $user_id;
    public $type_id;
    public $host_id;
    public $token;
    public $class_name;
    public $method_name;
    public $params;
    public $type;
    public $message;
    public $result;

    public function __construct($income = [])
    {
        if ($income === []) {
            $income = (object)["user" => (object)["lang_id" => 0, "region_id" => 0, "user_id" => 0, "type_id" => 0, "host_id" => 0, "token" => ""], "request" => (object)["class" => "", "method" => "", "params" => []]];
        }

//        $this->lang_id = intval($income->user->lang_id);
//        $this->region_id = intval($income->user->region_id);
//        $this->user_id = intval($income->user->user_id);
//        $this->type_id = intval($income->user->type_id);
//        $this->host_id = intval($income->user->host_id);
//        $this->token = $income->user->token;
        $this->class_name = $income->request->class;
        $this->method_name = $income->request->method;
        $this->params = $income->request->params;
        $this->type = 5;
        $this->message = "";
        $this->result = false;
    }

    /**
     * @return false|string
     */
    public function __toString()
    {
        return json_encode([
            //"user" => ["lang_id" => $this->lang_id, "region_id" => $this->region_id, "user_id" => $this->user_id, "type_id" => $this->type_id, "host_id" => $this->host_id, "token" => $this->token],
            "request" => ["class" => $this->class_name, "method" => $this->method_name, "params" => $this->params],
            "files" => [],
            "result" => ["type" => $this->type, "message" => $this->message, "result" => $this->result]
        ]);
    }

    /**
     * @return object
     */
    public function getUser(): object
    {
        return (object)['user_id' => $this->user_id, 'token' => $this->token, 'type_id' => $this->type_id, 'host' => $this->host_id, 'lang_id' => $this->lang_id, 'region' => $this->region_id];
    }

    /**
     * @return object
     */
    public function getRequest(): object
    {
        return (object)['class' => $this->class_name, 'method' => $this->method_name, 'params' => $this->params];
    }

    /**
     * @return object
     */
    public function getAnswer(): object
    {
        return (object)$this->result;
    }

    /**
     * @param string $message
     * @throws Exception
     */
    public function setMessage(string $message): void
    {
        // TODO STATIC TABLE
        //$text = System::getStaticText($this->lang_id, $message);
        $text = $message;
        if($text){
            $this->message = $text;
        }else{
            $this->message = $message;
        }
    }
}