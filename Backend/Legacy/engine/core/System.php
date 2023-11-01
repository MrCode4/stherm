<?php
/**
 * Copyright (c) 2020. Z-Soft All right reserved
 * @author  Victor Ohanyan
 */

namespace core;

use core\db\PgSQL;
use Exception;

trait System
{
    /**
     * Random string generator for password, token or something key|token|password
     * @param int $length
     * @param bool $include_letter
     * @param bool $include_number
     * @param bool $include_symbol
     * @return string
     */
    public static function randomGenerator(int $length = 12, bool $include_letter = true, bool $include_number = true, bool $include_symbol = false): string
    {
        $number = ['1', '2', '3', '4', '5', '6', '7', '8', '9', '0'];
        $alphabet = ['a', 'b', 'c', 'd', ',e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'];
        $symbols = ['!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '~', ',', '.', ';', '[', ']', '{', '}', ':', '|', '<', '>', '?', '-'];
        $selected = [];
        if ($include_letter === true) {
            $selected = array_merge($selected, $alphabet);
        }
        if ($include_number === true) {
            $selected = array_merge($selected, $number);
        }
        if ($include_symbol === true) {
            $selected = array_merge($selected, $symbols);
        }
        $pass = '';
        for ($i = 0; $i < $length; $i++) {
            $pass .= $selected[rand(0, count($selected) - 1)];
        }
        return $pass;
    }

    /**
     * get Static text info
     * @param int $lang_id
     * @param string $alias
     * @return string
     * @throws Exception
     */
    public static function getStaticText(int $lang_id, string $alias): string
    {
        $db = PgSQL::Connection();
        return $db->getItem("SELECT text FROM static WHERE lang_id = {$lang_id} AND alias = '{$alias}'");
    }

    /**
     * Create Password hash
     * @param string $password
     * @return string
     */
    public static function getHash(string $password): string
    {
        return password_hash($password, PASSWORD_DEFAULT);
    }
}