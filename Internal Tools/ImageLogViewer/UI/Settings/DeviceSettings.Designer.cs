namespace ImageLogViewer.UI.Settings
{
    partial class DeviceSettings
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(DeviceSettings));
            this.gbDeviceAddress = new System.Windows.Forms.GroupBox();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.btnDefaultLogging = new System.Windows.Forms.Button();
            this.btnSetLoggingOptions = new System.Windows.Forms.Button();
            this.edtWorkstationAddress = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.btnRebootDevice = new System.Windows.Forms.Button();
            this.edtPort = new System.Windows.Forms.TextBox();
            this.lblIPAddress = new System.Windows.Forms.Label();
            this.edtIPAddress = new System.Windows.Forms.TextBox();
            this.lblPort = new System.Windows.Forms.Label();
            this.btnCancel = new System.Windows.Forms.Button();
            this.gbLogOptions = new System.Windows.Forms.GroupBox();
            this.radioDontLogImages = new System.Windows.Forms.RadioButton();
            this.radioLogFramesAndCrops = new System.Windows.Forms.RadioButton();
            this.radioLogCropsOnly = new System.Windows.Forms.RadioButton();
            this.cbxSaveImages = new System.Windows.Forms.CheckBox();
            this.gbTextLoggingOptions = new System.Windows.Forms.GroupBox();
            this.cbEventLogLevel = new System.Windows.Forms.ComboBox();
            this.cbMotorLogLevel = new System.Windows.Forms.ComboBox();
            this.cbFaceTrackingLevel = new System.Windows.Forms.ComboBox();
            this.label4 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.cbNXTLogLevel = new System.Windows.Forms.ComboBox();
            this.gbDeviceAddress.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.gbLogOptions.SuspendLayout();
            this.gbTextLoggingOptions.SuspendLayout();
            this.SuspendLayout();
            // 
            // gbDeviceAddress
            // 
            this.gbDeviceAddress.Controls.Add(this.groupBox2);
            this.gbDeviceAddress.Controls.Add(this.groupBox1);
            this.gbDeviceAddress.Location = new System.Drawing.Point(9, 153);
            this.gbDeviceAddress.Margin = new System.Windows.Forms.Padding(2);
            this.gbDeviceAddress.Name = "gbDeviceAddress";
            this.gbDeviceAddress.Padding = new System.Windows.Forms.Padding(2);
            this.gbDeviceAddress.Size = new System.Drawing.Size(489, 172);
            this.gbDeviceAddress.TabIndex = 0;
            this.gbDeviceAddress.TabStop = false;
            this.gbDeviceAddress.Text = "Endpoints";
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.btnDefaultLogging);
            this.groupBox2.Controls.Add(this.btnSetLoggingOptions);
            this.groupBox2.Controls.Add(this.edtWorkstationAddress);
            this.groupBox2.Controls.Add(this.label5);
            this.groupBox2.Location = new System.Drawing.Point(283, 29);
            this.groupBox2.Margin = new System.Windows.Forms.Padding(2);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Padding = new System.Windows.Forms.Padding(2);
            this.groupBox2.Size = new System.Drawing.Size(202, 139);
            this.groupBox2.TabIndex = 4;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Local Workstation Address";
            // 
            // btnDefaultLogging
            // 
            this.btnDefaultLogging.Location = new System.Drawing.Point(10, 102);
            this.btnDefaultLogging.Margin = new System.Windows.Forms.Padding(2);
            this.btnDefaultLogging.Name = "btnDefaultLogging";
            this.btnDefaultLogging.Size = new System.Drawing.Size(181, 30);
            this.btnDefaultLogging.TabIndex = 4;
            this.btnDefaultLogging.Text = "Revert To Default Logging";
            this.btnDefaultLogging.UseVisualStyleBackColor = true;
            this.btnDefaultLogging.Click += new System.EventHandler(this.btnDefaultLogging_Click);
            // 
            // btnSetLoggingOptions
            // 
            this.btnSetLoggingOptions.Location = new System.Drawing.Point(10, 63);
            this.btnSetLoggingOptions.Margin = new System.Windows.Forms.Padding(2);
            this.btnSetLoggingOptions.Name = "btnSetLoggingOptions";
            this.btnSetLoggingOptions.Size = new System.Drawing.Size(181, 35);
            this.btnSetLoggingOptions.TabIndex = 4;
            this.btnSetLoggingOptions.Text = "Start/Update Remote Logging";
            this.btnSetLoggingOptions.UseVisualStyleBackColor = true;
            this.btnSetLoggingOptions.Click += new System.EventHandler(this.btnSetLoggingOptions_Click);
            // 
            // edtWorkstationAddress
            // 
            this.edtWorkstationAddress.Location = new System.Drawing.Point(87, 29);
            this.edtWorkstationAddress.Margin = new System.Windows.Forms.Padding(2);
            this.edtWorkstationAddress.Name = "edtWorkstationAddress";
            this.edtWorkstationAddress.Size = new System.Drawing.Size(104, 20);
            this.edtWorkstationAddress.TabIndex = 2;
            // 
            // label5
            // 
            this.label5.Location = new System.Drawing.Point(7, 29);
            this.label5.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(75, 19);
            this.label5.TabIndex = 0;
            this.label5.Text = "IP Address:";
            this.label5.TextAlign = System.Drawing.ContentAlignment.TopRight;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.btnRebootDevice);
            this.groupBox1.Controls.Add(this.edtPort);
            this.groupBox1.Controls.Add(this.lblIPAddress);
            this.groupBox1.Controls.Add(this.edtIPAddress);
            this.groupBox1.Controls.Add(this.lblPort);
            this.groupBox1.Location = new System.Drawing.Point(14, 29);
            this.groupBox1.Margin = new System.Windows.Forms.Padding(2);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Padding = new System.Windows.Forms.Padding(2);
            this.groupBox1.Size = new System.Drawing.Size(254, 139);
            this.groupBox1.TabIndex = 3;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Device Address";
            // 
            // btnRebootDevice
            // 
            this.btnRebootDevice.Location = new System.Drawing.Point(60, 97);
            this.btnRebootDevice.Name = "btnRebootDevice";
            this.btnRebootDevice.Size = new System.Drawing.Size(124, 28);
            this.btnRebootDevice.TabIndex = 3;
            this.btnRebootDevice.Text = "Reboot Device";
            this.btnRebootDevice.UseVisualStyleBackColor = true;
            this.btnRebootDevice.Click += new System.EventHandler(this.btnRebootDevice_Click);
            // 
            // edtPort
            // 
            this.edtPort.Location = new System.Drawing.Point(88, 58);
            this.edtPort.Margin = new System.Windows.Forms.Padding(2);
            this.edtPort.Name = "edtPort";
            this.edtPort.Size = new System.Drawing.Size(106, 20);
            this.edtPort.TabIndex = 2;
            // 
            // lblIPAddress
            // 
            this.lblIPAddress.Location = new System.Drawing.Point(8, 30);
            this.lblIPAddress.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.lblIPAddress.Name = "lblIPAddress";
            this.lblIPAddress.Size = new System.Drawing.Size(75, 19);
            this.lblIPAddress.TabIndex = 0;
            this.lblIPAddress.Text = "IP Address:";
            this.lblIPAddress.TextAlign = System.Drawing.ContentAlignment.TopRight;
            // 
            // edtIPAddress
            // 
            this.edtIPAddress.Location = new System.Drawing.Point(88, 30);
            this.edtIPAddress.Margin = new System.Windows.Forms.Padding(2);
            this.edtIPAddress.Name = "edtIPAddress";
            this.edtIPAddress.Size = new System.Drawing.Size(106, 20);
            this.edtIPAddress.TabIndex = 2;
            // 
            // lblPort
            // 
            this.lblPort.Location = new System.Drawing.Point(8, 61);
            this.lblPort.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.lblPort.Name = "lblPort";
            this.lblPort.Size = new System.Drawing.Size(75, 19);
            this.lblPort.TabIndex = 1;
            this.lblPort.Text = "Port:";
            this.lblPort.TextAlign = System.Drawing.ContentAlignment.TopRight;
            // 
            // btnCancel
            // 
            this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnCancel.Location = new System.Drawing.Point(433, 329);
            this.btnCancel.Margin = new System.Windows.Forms.Padding(2);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(70, 28);
            this.btnCancel.TabIndex = 1;
            this.btnCancel.Text = "Close";
            this.btnCancel.UseVisualStyleBackColor = true;
            // 
            // gbLogOptions
            // 
            this.gbLogOptions.Controls.Add(this.radioDontLogImages);
            this.gbLogOptions.Controls.Add(this.radioLogFramesAndCrops);
            this.gbLogOptions.Controls.Add(this.radioLogCropsOnly);
            this.gbLogOptions.Controls.Add(this.cbxSaveImages);
            this.gbLogOptions.Location = new System.Drawing.Point(9, 10);
            this.gbLogOptions.Margin = new System.Windows.Forms.Padding(2);
            this.gbLogOptions.Name = "gbLogOptions";
            this.gbLogOptions.Padding = new System.Windows.Forms.Padding(2);
            this.gbLogOptions.Size = new System.Drawing.Size(254, 138);
            this.gbLogOptions.TabIndex = 2;
            this.gbLogOptions.TabStop = false;
            this.gbLogOptions.Text = "Image Logging Options";
            // 
            // radioDontLogImages
            // 
            this.radioDontLogImages.AutoSize = true;
            this.radioDontLogImages.Location = new System.Drawing.Point(21, 26);
            this.radioDontLogImages.Margin = new System.Windows.Forms.Padding(2);
            this.radioDontLogImages.Name = "radioDontLogImages";
            this.radioDontLogImages.Size = new System.Drawing.Size(133, 17);
            this.radioDontLogImages.TabIndex = 1;
            this.radioDontLogImages.TabStop = true;
            this.radioDontLogImages.Text = "Disable Image Logging";
            this.radioDontLogImages.UseVisualStyleBackColor = true;
            // 
            // radioLogFramesAndCrops
            // 
            this.radioLogFramesAndCrops.AutoSize = true;
            this.radioLogFramesAndCrops.Location = new System.Drawing.Point(21, 70);
            this.radioLogFramesAndCrops.Margin = new System.Windows.Forms.Padding(2);
            this.radioLogFramesAndCrops.Name = "radioLogFramesAndCrops";
            this.radioLogFramesAndCrops.Size = new System.Drawing.Size(131, 17);
            this.radioLogFramesAndCrops.TabIndex = 1;
            this.radioLogFramesAndCrops.TabStop = true;
            this.radioLogFramesAndCrops.Text = "Log Frames and Crops";
            this.radioLogFramesAndCrops.UseVisualStyleBackColor = true;
            // 
            // radioLogCropsOnly
            // 
            this.radioLogCropsOnly.AutoSize = true;
            this.radioLogCropsOnly.Location = new System.Drawing.Point(21, 48);
            this.radioLogCropsOnly.Margin = new System.Windows.Forms.Padding(2);
            this.radioLogCropsOnly.Name = "radioLogCropsOnly";
            this.radioLogCropsOnly.Size = new System.Drawing.Size(97, 17);
            this.radioLogCropsOnly.TabIndex = 1;
            this.radioLogCropsOnly.TabStop = true;
            this.radioLogCropsOnly.Text = "Log Crops Only";
            this.radioLogCropsOnly.UseVisualStyleBackColor = true;
            // 
            // cbxSaveImages
            // 
            this.cbxSaveImages.AutoSize = true;
            this.cbxSaveImages.Location = new System.Drawing.Point(21, 106);
            this.cbxSaveImages.Margin = new System.Windows.Forms.Padding(2);
            this.cbxSaveImages.Name = "cbxSaveImages";
            this.cbxSaveImages.Size = new System.Drawing.Size(177, 17);
            this.cbxSaveImages.TabIndex = 0;
            this.cbxSaveImages.Text = "Save Images To File On Device";
            this.cbxSaveImages.UseVisualStyleBackColor = true;
            // 
            // gbTextLoggingOptions
            // 
            this.gbTextLoggingOptions.Controls.Add(this.cbEventLogLevel);
            this.gbTextLoggingOptions.Controls.Add(this.cbMotorLogLevel);
            this.gbTextLoggingOptions.Controls.Add(this.cbFaceTrackingLevel);
            this.gbTextLoggingOptions.Controls.Add(this.label4);
            this.gbTextLoggingOptions.Controls.Add(this.label3);
            this.gbTextLoggingOptions.Controls.Add(this.label2);
            this.gbTextLoggingOptions.Controls.Add(this.label1);
            this.gbTextLoggingOptions.Controls.Add(this.cbNXTLogLevel);
            this.gbTextLoggingOptions.Location = new System.Drawing.Point(268, 10);
            this.gbTextLoggingOptions.Margin = new System.Windows.Forms.Padding(2);
            this.gbTextLoggingOptions.Name = "gbTextLoggingOptions";
            this.gbTextLoggingOptions.Padding = new System.Windows.Forms.Padding(2);
            this.gbTextLoggingOptions.Size = new System.Drawing.Size(230, 138);
            this.gbTextLoggingOptions.TabIndex = 3;
            this.gbTextLoggingOptions.TabStop = false;
            this.gbTextLoggingOptions.Text = "Text File Logging Options";
            // 
            // cbEventLogLevel
            // 
            this.cbEventLogLevel.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbEventLogLevel.FormattingEnabled = true;
            this.cbEventLogLevel.Items.AddRange(new object[] {
            "TRACE",
            "DEBUG",
            "INFO",
            "WARN",
            "ERROR",
            "FATAL"});
            this.cbEventLogLevel.Location = new System.Drawing.Point(117, 102);
            this.cbEventLogLevel.Margin = new System.Windows.Forms.Padding(2);
            this.cbEventLogLevel.Name = "cbEventLogLevel";
            this.cbEventLogLevel.Size = new System.Drawing.Size(92, 21);
            this.cbEventLogLevel.TabIndex = 1;
            // 
            // cbMotorLogLevel
            // 
            this.cbMotorLogLevel.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbMotorLogLevel.FormattingEnabled = true;
            this.cbMotorLogLevel.Items.AddRange(new object[] {
            "TRACE",
            "DEBUG",
            "INFO",
            "WARN",
            "ERROR",
            "FATAL"});
            this.cbMotorLogLevel.Location = new System.Drawing.Point(117, 77);
            this.cbMotorLogLevel.Margin = new System.Windows.Forms.Padding(2);
            this.cbMotorLogLevel.Name = "cbMotorLogLevel";
            this.cbMotorLogLevel.Size = new System.Drawing.Size(92, 21);
            this.cbMotorLogLevel.TabIndex = 1;
            // 
            // cbFaceTrackingLevel
            // 
            this.cbFaceTrackingLevel.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbFaceTrackingLevel.FormattingEnabled = true;
            this.cbFaceTrackingLevel.Items.AddRange(new object[] {
            "TRACE",
            "DEBUG",
            "INFO",
            "WARN",
            "ERROR",
            "FATAL"});
            this.cbFaceTrackingLevel.Location = new System.Drawing.Point(117, 53);
            this.cbFaceTrackingLevel.Margin = new System.Windows.Forms.Padding(2);
            this.cbFaceTrackingLevel.Name = "cbFaceTrackingLevel";
            this.cbFaceTrackingLevel.Size = new System.Drawing.Size(92, 21);
            this.cbFaceTrackingLevel.TabIndex = 1;
            // 
            // label4
            // 
            this.label4.Location = new System.Drawing.Point(27, 104);
            this.label4.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(75, 19);
            this.label4.TabIndex = 0;
            this.label4.Text = "Event Log:";
            this.label4.TextAlign = System.Drawing.ContentAlignment.TopRight;
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(27, 80);
            this.label3.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(75, 19);
            this.label3.TabIndex = 0;
            this.label3.Text = "Motor Log:";
            this.label3.TextAlign = System.Drawing.ContentAlignment.TopRight;
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(7, 55);
            this.label2.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(94, 24);
            this.label2.TabIndex = 0;
            this.label2.Text = "Face Tracking Log:";
            this.label2.TextAlign = System.Drawing.ContentAlignment.TopRight;
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(27, 31);
            this.label1.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(75, 19);
            this.label1.TabIndex = 0;
            this.label1.Text = "NXT Log:";
            this.label1.TextAlign = System.Drawing.ContentAlignment.TopRight;
            // 
            // cbNXTLogLevel
            // 
            this.cbNXTLogLevel.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbNXTLogLevel.FormattingEnabled = true;
            this.cbNXTLogLevel.Items.AddRange(new object[] {
            "TRACE",
            "DEBUG",
            "INFO",
            "WARN",
            "ERROR",
            "FATAL"});
            this.cbNXTLogLevel.Location = new System.Drawing.Point(117, 28);
            this.cbNXTLogLevel.Margin = new System.Windows.Forms.Padding(2);
            this.cbNXTLogLevel.Name = "cbNXTLogLevel";
            this.cbNXTLogLevel.Size = new System.Drawing.Size(92, 21);
            this.cbNXTLogLevel.TabIndex = 1;
            // 
            // DeviceSettings
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BottomColor = System.Drawing.Color.FromArgb(((int)(((byte)(224)))), ((int)(((byte)(224)))), ((int)(((byte)(224)))));
            this.ClientSize = new System.Drawing.Size(512, 364);
            this.Controls.Add(this.gbTextLoggingOptions);
            this.Controls.Add(this.gbLogOptions);
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.gbDeviceAddress);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Margin = new System.Windows.Forms.Padding(2);
            this.Name = "DeviceSettings";
            this.Text = "Remote Device Log Settings";
            this.TopColor = System.Drawing.Color.WhiteSmoke;
            this.Load += new System.EventHandler(this.DeviceSettings_Load);
            this.gbDeviceAddress.ResumeLayout(false);
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.gbLogOptions.ResumeLayout(false);
            this.gbLogOptions.PerformLayout();
            this.gbTextLoggingOptions.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox gbDeviceAddress;
        private System.Windows.Forms.TextBox edtPort;
        private System.Windows.Forms.TextBox edtIPAddress;
        private System.Windows.Forms.Label lblPort;
        private System.Windows.Forms.Label lblIPAddress;
        private System.Windows.Forms.Button btnCancel;
        private System.Windows.Forms.GroupBox gbLogOptions;
        private System.Windows.Forms.CheckBox cbxSaveImages;
        private System.Windows.Forms.GroupBox gbTextLoggingOptions;
        private System.Windows.Forms.ComboBox cbFaceTrackingLevel;
        private System.Windows.Forms.ComboBox cbNXTLogLevel;
        private System.Windows.Forms.ComboBox cbEventLogLevel;
        private System.Windows.Forms.ComboBox cbMotorLogLevel;
        private System.Windows.Forms.Button btnSetLoggingOptions;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.RadioButton radioDontLogImages;
        private System.Windows.Forms.RadioButton radioLogFramesAndCrops;
        private System.Windows.Forms.RadioButton radioLogCropsOnly;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.TextBox edtWorkstationAddress;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button btnRebootDevice;
        private System.Windows.Forms.Button btnDefaultLogging;
    }
}