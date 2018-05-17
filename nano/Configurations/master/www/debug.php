<?php

ob_start();
include_once('/home/www/FirePHP.class.php');
$firephp = FirePHP::getInstance(true);

$firephp->log('Log message', 'test');
$firephp->info('Info message', 'test2');
$firephp->warn('Warn message', 'test3');
$firephp->error('Error message', 'test4');
?>