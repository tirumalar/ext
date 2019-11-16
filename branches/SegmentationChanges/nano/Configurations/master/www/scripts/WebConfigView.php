<?php
    

class View
{
    private $model;
    private $controller;
 
    public function __construct($controller, $model)
    {
        $this->controller = $controller;
        $this->model = $model;
    }


    // Display config page
    public function Login()
    {
        // Load all settings before login... so we can display correct text...
     //   $this->controller->Load();
        require_once($_SERVER['DOCUMENT_ROOT']."/login.html");
    }
 

    // Display config page
    public function Configure()
    {
 //       $this->controller->Load();
        require_once($_SERVER['DOCUMENT_ROOT']."/configdlg.html");
    }
}
?>
