<?php

namespace core;
require_once __DIR__ . "/../core/db/PgSQL.php";
use core\db\Z_PgSQL;
use Exception;

class User
{
    /**
     * check user login
     * @param string $username
     * @param string $password
     * @return object
     * @throws Exception
     */
    public static function login(string $username, string $password): object
    {
        $check_type = intval(self::checkType());

        if ($check_type === 2) { // login DC
            $output = '';
            $ldap = parse_ini_file('..' . DIRECTORY_SEPARATOR . 'configuration' . DIRECTORY_SEPARATOR . 'ldap.ini');
            if (!empty($username) && !empty($password)) {
                $ldap_connect = ldap_connect($ldap['LDAP_HOST'], $ldap['LDAP_PORT']) or die("Connection failed");
                $ldap_bind = @ldap_bind($ldap_connect, $username, $password);
                if ($ldap_bind) {
                    $output = 'True';
                } else {
                    $output = 'False';
                }

                ldap_close($ldap_connect);

                // TODO Понять почему не работает логин через LdapUserValidator.exe

                //$ldap_validator_path = "engine/library/asp/LdapUserValidator.exe";
                //$output = shell_exec("{$ldap_validator_path} {$username} {$password} {$ldap_server}");

                if ($output === "True") {
                    $db = Z_PgSQL::connection();
                    $user_id = (int)$db->getItem("SELECT user_id FROM users WHERE username = '{$username}'");
                    if($user_id === 0){
                        $user_id = self::addUser($username, $password, 'no-mail', '')->result->user_id;
                    }
                    $user = self::getInfo($user_id);
                    self::setSession($user->lang_id, $user->region_id, $user_id, $user->host_id, $user->type_id, '');
                    return (object)["type" => MESSAGE, "message" => "Login ok", "result" => true];
                } elseif ($output === "False") {
                    return (object)["type" => WARNING, "message" => "Wrong username and password", "result" => false];
                } else {
                    return (object)["type" => ERROR, "message" => "LDAP Error " . $output, "result" => false];
                }
            }
            return (object)["type" => ERROR, "message" => "Wrong user data", "result" => false];
        } else if ($check_type === 1) { // login local
            if (!empty($username) && !empty($password)) {
                $db = Z_PgSQL::connection();
                $user = $db->getRow("SELECT user_id, password FROM users WHERE username = '{$username}'");
                if(empty($user)) {
                    return (object)["type" => WARNING, "message" => "Wrong username and password", "result" => false];
                } else {
                    if(password_verify($password, $user['password'])) {
                        self::setSession($user['user_id'], '');
                        return (object)["type" => MESSAGE, "message" => "Login ok", "result" => true];
                    } else {
                        return (object)["type" => WARNING, "message" => "Wrong username and password", "result" => false];
                    }
                }
            }
        }
        return (object)["type" => ERROR, "message" => "Please login", "result" => false];
    }

    /**
     * password recovery call
     * @param string $email
     * @return object
     * @throws Exception
     */
    public static function forgot(string $email): object
    {
        $db = PgSQL::connection();
        $user_id = $db->getItem("SELECT user_id FROM users WHERE mail = '{$email}'");
        if($user_id>0){
            $token = System::randomGenerator();
            if($db->setQuery("INSERT INTO recovery(user_id, token) VALUES({$user_id}, '{$token}')")['result']){
                // TODO SEND MAIL
                return (object)['type' => MESSAGE, 'message' => 'Recovery mail sent' , 'result' => true];
            }else{
                return (object)['type' => ERROR, 'message' => 'DB Problem' , 'result' => false];
            }
        }
        return (object)['type' => ERROR, 'message' => 'Unknown mail address' , 'result' => false];
    }

    /**
     * create new password by token
     * @param string $token
     * @param string $password
     * @return object
     * @throws Exception
     */
    public static function newPassword(string $token, string $password):object
    {
        $db = PgSQL::connection();
        $user_id = $db->getItem("SELECT user_id FROM recovery WHERE token = '{$token}' AND expire>current_timestamp");
        if ($user_id > 0) {
            $pass = System::getHash($password);
            if($db->setQuery("UPDATE users SET password = {$pass}")['result']){
                return (object)['type' => MESSAGE, 'message' => 'Password changed' , 'result' => false];
            }else{
                return (object)['type' => ERROR, 'message' => 'DB Problem' , 'result' => false];
            }
        } else {
            return (object)['type' => ERROR, 'message' => 'Wrong key', 'result' => false];
        }
    }

    /**
     * Check has system user
     * @return mixed
     */
    public static function hasUser()
    {
        return SYSTEM_SETTINGS['HAS_USER'];
    }

    /**
     * Check is user logged
     * @param int $user_id
     * @param string $token
     * @param int $host_id
     * @param int $type_id
     * @param int $region_id
     * @param int $lang_id
     * @return bool
     * @throws Exception
     */
    public static function checkUser(int $user_id, string $token, int $host_id, int $type_id, int $region_id, int $lang_id): bool
    {
        $db = PgSQL::connection();
        $logout_time = SYSTEM_SETTINGS['LOGOUT_INTERVAL'];
        $user = $db->getItem("SELECT user_id FROM online WHERE user_id = {$user_id} AND token = '{$token}' AND last_action<current_timestamp + interval '{$logout_time}'");
        if ($user>0) {
            return true;
        }
        self::logout();
        return false;
    }

    /**
     * Set session data and update user online data
     * @param int $lang_id
     * @param int $region_id
     * @param int $user_id
     * @param int $host_id
     * @param int $type_id
     * @param string $token
     * @return void
     * @throws Exception
     */
    public static function setSession(int $user_id = 0, string $token = '')
    {
        if ($token === '') {
            $token = System::randomGenerator();
        }
        if(intval(SYSTEM_SETTINGS['HAS_USER'])===1){
            $db = Z_PgSQL::connection();
            if ($user_id === 0) {
                $db->setQuery("DELETE FROM online WHERE user_id = {$user_id}");
            }else{
                $db->setQuery("INSERT INTO online(user_id, token, last_action) VALUES({$user_id}, '{$token}', CURRENT_TIMESTAMP) ON CONFLICT (user_id) DO UPDATE SET last_action = CURRENT_TIMESTAMP");
            }
        }

        $_SESSION['user_id'] = $user_id;
        $_SESSION['host_id'] = 0;
        $_SESSION['type_id'] = 0;
        $_SESSION['lang_id'] = 1;
        $_SESSION['region_id'] = 0;
        $_SESSION['token'] = $token;
    }

    /**
     * get Logged User info
     * @param int $user_id
     * @return object
     * @throws Exception
     */
    public static function getInfo(int $user_id = 0): object
    {
        $db = Z_PgSQL::connection();
        if($user_id === 0){
            $user_id = $_SESSION['user_id'];
        }
        return (object)$db->getRow("SELECT user_id AS id, username, mail, host_id, type_id, region_id, name, photo, lang_id FROM users WHERE user_id = {$user_id}");
    }

    /**
     * Logout user
     * @param int $user_id
     * @return object
     * @throws Exception
     */
    public static function logout(int $user_id = 0): object
    {
        $db = Z_PgSQL::connection();
        if($user_id === 0){
            $user_id = $_SESSION['user_id'];
        }
//        echo $user_id;
        $db->setQuery("DELETE FROM online WHERE user_id = {$user_id}");
        self::setSession();
        return (object)['type' => MESSAGE, 'message' => 'Logout ok', 'result' => true];
    }

    /**
     * Check user checking system
     * @return mixed User check 1 IN APP 2 DC
     */
    public static function checkType()
    {
        return SYSTEM_SETTINGS['CHECKER'];
    }

    /**
     * Add User
     * @param string $username
     * @param string $password
     * @param string $mail
     * @param string $expiration
     * @param string $name
     * @param string $photo
     * @param int $type_id
     * @param int $host_id
     * @param int $region_id
     * @param int $lang_id
     * @return object
     * @throws Exception
     */
    public static function addUser(string $username, string $password, string $mail, string $expiration, string $name = '', string $photo = './upload/no-avatar.png', int $type_id = 1, int $host_id=1, int $region_id=1, int $lang_id = 1): object
    {
        $db = Z_PgSQL::connection();
        $pass = System::getHash($password);
        $user = $db->setQuery("INSERT INTO users(username, password, mail, host_id, type_id, region_id, name, photo, lang_id) VALUES('{$username}', '{$pass}', '{$mail}', {$host_id}, {$type_id}, {$region_id}, '{$name}', '{$photo}', {$lang_id}) RETURNING user_id");
        if($user['result']){
            return (object)['type' => MESSAGE, 'message' => 'User created' , 'result' => $user['return_data']];
        }
        return (object)['type' => ERROR, 'message' => 'DB Problem' , 'result' => false];
    }
}