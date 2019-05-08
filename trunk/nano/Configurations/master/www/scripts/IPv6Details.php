<?php

require("checklogin.php"); // Make sure user is logged on to get to this page...
include_once($_SERVER['DOCUMENT_ROOT']."/scripts/debug.php");
// Class handles reading/updating interface specifics of the nano...
class IPv6Details
{
   ////////////////////////////////////////////////////
    // IPv6 values of interest
    ////////////////////////////////////////////////////
    public $EnableIPv6;
    public $DHCPMode;
    public $HostAddress;
    public $SubnetPrefixLength;
    public $DefaultGateway;
    public $Dns1;
    public $Dns2;

    public $ConfigFilename = "/home/www-internal/interfaces6";
    public $ConfigFileContents = "";

    public $RawConfigParams = [
        "EnableIPv6" => "true",
        "DhcpMode" => "information-only",
        //"AcceptRouterAdvertisements" => "true",
        "Address" => "",
        "SubnetPrefixLength" => "64",
        "Gateway" => "",
        "Dns1" => "",
        "Dns2" => ""
    ];

    function LoadConfig()
    {
        $this->ConfigFileContents = "";

        if (file_exists($this->ConfigFilename))
        {
            $this->ConfigFileContents = file_get_contents($this->ConfigFilename);
        }
        
        $this->ProcessConfig(FALSE);
        $this->GetConfigParams();
    }

    function SaveConfig()
    {
        $reboot = $this->SetConfigParams();
        $newConfigFileContents = $this->ProcessConfig(TRUE);

        file_put_contents($this->ConfigFilename, $newConfigFileContents);

        return $reboot;
    }

    function ParseRequest($post)
    {
        $this->EnableIPv6 = "false";
        foreach ($post as $key => $value)
        {
            $value = trim($value); // So far we don't have parameters with space characters on sides
            switch ($key)
            {
                case "ipv6_enable":
                    if ($value === "true")
                    {
                        $this->EnableIPv6 = "true";
                    }
                    break;
                case "ipv6_dhcpmode":
                    if ($value === "information-only" || $value === "normal" || $value === "none")
                    {
                        $this->DHCPMode = $value;
                    }
                    break;
                case "ipv6_address":
                    if ($this->IsValidIPv6Address($value))
                    {
                        $this->HostAddress = $value;
                    }
                    break;
                case "ipv6_prefix_len":
                    if (ctype_digit($value))
                    {
                        $this->SubnetPrefixLength = $value;
                    }
                    break;
                case "ipv6_gateway":
                    if ($this->IsValidIPv6Address($value))
                    {
                        $this->DefaultGateway = $value;
                    }
                    break;
                case "ipv6_dns1":
                    if ($this->IsValidIPv6Address($value))
                    {
                        $this->Dns1 = $value;
                    }
                    break;
                case "ipv6_dns2":
                    if ($this->IsValidIPv6Address($value))
                    {
                        $this->Dns2 = $value;
                    }
                    break;
            }
        }
    }

    function IsValidIPv6Address($address)
    {
        return filter_var($address, FILTER_VALIDATE_IP, FILTER_FLAG_IPV6) !== FALSE;
    }

    function ProcessConfig($isSave)
    {
        $processedParams = [];

        $configLines = explode("\n", $this->ConfigFileContents);
        $outConfigFileContents = "";

        foreach ($configLines as $line)
        {
            if (isset($line))
            {
                $keyValue = explode("=", $line);

                if (isset($keyValue[0]) && isset($keyValue[1]))
                {
                    $key = trim($keyValue[0]);
                    $value = trim($keyValue[1]);
                    if (array_key_exists($key, $this->RawConfigParams))
                    {
                        $processedParams[$key] = TRUE;
                        if ($isSave)
                        {
                            $outConfigFileContents .= sprintf("%s=%s\n", $key, $this->RawConfigParams[$key]);
                        }
                        else
                        {
                            $this->RawConfigParams[$key] = $value;
                        }

                        continue;
                    }
                }
            }

            // Pass line as is to the output if this is not a valid param
            $outConfigFileContents .= ($line."\n");
        }

        // Add parameters missing in the original content
        foreach ($this->RawConfigParams as $key => $value)
        {
            if (!array_key_exists($key, $processedParams))
            {
                $outConfigFileContents .= sprintf("%s=%s\n", $key, $this->RawConfigParams[$key]);
            }
        }

        $outConfigFileContents = substr($outConfigFileContents, 0, -1); // Remove the last carriage return
        return $outConfigFileContents;
    }

    function GetConfigParams()
    {
        $this->GetConfigParam("EnableIPv6", $this->EnableIPv6);
        $this->GetConfigParam("DhcpMode", $this->DHCPMode);
        $this->GetConfigParam("Address", $this->HostAddress);
        $this->GetConfigParam("SubnetPrefixLength", $this->SubnetPrefixLength);
        $this->GetConfigParam("Gateway", $this->DefaultGateway);
        $this->GetConfigParam("Dns1", $this->Dns1);
        $this->GetConfigParam("Dns2", $this->Dns2);
    }

    function SetConfigParams()
    {
        $reboot = FALSE;
        $reboot = $this->SetConfigParam("EnableIPv6", $this->EnableIPv6) || $reboot;
        $isEnabled = $this->EnableIPv6 === "true";
        $reboot = ($this->SetConfigParam("DhcpMode", $this->DHCPMode) && $isEnabled) || $reboot;
        $reboot = ($this->SetConfigParam("Address", $this->HostAddress) && $isEnabled) || $reboot;
        $reboot = ($this->SetConfigParam("SubnetPrefixLength", $this->SubnetPrefixLength) && $isEnabled) || $reboot;
        $reboot = ($this->SetConfigParam("Gateway", $this->DefaultGateway) && $isEnabled) || $reboot;
        $reboot = ($this->SetConfigParam("Dns1", $this->Dns1) && $isEnabled) || $reboot;
        $reboot = ($this->SetConfigParam("Dns2", $this->Dns2) && $isEnabled) || $reboot;
        return $reboot;
    }

    function GetConfigParam($key, &$outValue)
    {
        if (array_key_exists($key, $this->RawConfigParams))
        {
            $outValue = $this->RawConfigParams[$key];
        }
    }

    function SetConfigParam($key, $value)
    {
        // echo "Setting ".$key."\n";
        $isChanged = FALSE;
        if (array_key_exists($key, $this->RawConfigParams))
        {
            if ($this->RawConfigParams[$key] !== $value)
            {
                // echo $key." changed.\n";
                // echo "Old value: ".$this->RawConfigParams[$key]."\n";
                // echo "New value: ".$value."\n";
                $isChanged = TRUE;
            }
            $this->RawConfigParams[$key] = $value;
        }

        return $isChanged;
    }
} // Class
?>
