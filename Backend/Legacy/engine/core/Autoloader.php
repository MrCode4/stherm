<?php declare(strict_types=1);
/**
 * Copyright (c) 2020. Z-Soft All right reserved
 * @author Victor Ohanyan
 */

namespace core;

use Exception;

class Autoloader
{
    /**
     * @throws Exception
     */
    public static function register():void
    {
        spl_autoload_register(function ($class) {
            //var_dump($class);
            if($class === 'core\db\Z_PgSQL') {
                $class = 'core\db\PgSQL';
            }
            $file = $_SERVER['DOCUMENT_ROOT'].DIRECTORY_SEPARATOR.'engine'.DIRECTORY_SEPARATOR.str_replace('\\', DIRECTORY_SEPARATOR, $class) . '.php';
            if (file_exists($file)) {
                require_once "{$file}";
            }else{
                throw new Exception($file." File not found");
            }
        });
    }
}

try {
    Autoloader::register();
} catch (Exception $e) {
}