<?php
    

class Controller
{
    private $model;
 
    public function __construct($model)
    {
        $this->model = $model;
    }
 

    public function Load()
    {
        $this->model->LoadAllSettings();
    }

    // User clicked on 'save'
    public function Save($thePost)
    {
        // Now, update and save all of our internals
        $this->model->UpdateAllSettings($thePost);
    }
}
?>
