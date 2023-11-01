<?php

namespace core;

trait Validator
{
    /**
     * check mail validation
     * @param string $email
     * @return bool|object
     */
    private function mailValidation(string $email) {
        $regex = '/^(([^<>()\[\]\\.,;:\s@"]+(\.[^<>()\[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/';
        if (!(bool)preg_match($regex, $email) && $email === '') {
            return (object)["type" => WARNING, "message" => "Not valid mail", "result" => false];
        }
        return true;
    }
}