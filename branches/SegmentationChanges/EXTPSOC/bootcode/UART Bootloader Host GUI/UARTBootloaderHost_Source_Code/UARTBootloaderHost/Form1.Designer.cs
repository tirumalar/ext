namespace UARTBootloaderHost
{
    partial class UIForm
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
            this.components = new System.ComponentModel.Container();
            this.label1 = new System.Windows.Forms.Label();
            this.comPortComboBox = new System.Windows.Forms.ComboBox();
            this.serialPort = new System.IO.Ports.SerialPort(this.components);
            this.baudComboBox = new System.Windows.Forms.ComboBox();
            this.label3 = new System.Windows.Forms.Label();
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.FileNameTB = new System.Windows.Forms.TextBox();
            this.browseButton = new System.Windows.Forms.Button();
            this.bootloadButton = new System.Windows.Forms.Button();
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.progressBar1 = new System.Windows.Forms.ToolStripProgressBar();
            this.label2 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.textBox_StatusLog = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.backgroundWorker = new System.ComponentModel.BackgroundWorker();
            this.statusStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(10, 39);
            this.label1.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(86, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Select COM Port";
            // 
            // comPortComboBox
            // 
            this.comPortComboBox.DisplayMember = "1";
            this.comPortComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comPortComboBox.FormattingEnabled = true;
            this.comPortComboBox.Location = new System.Drawing.Point(100, 36);
            this.comPortComboBox.Margin = new System.Windows.Forms.Padding(2);
            this.comPortComboBox.Name = "comPortComboBox";
            this.comPortComboBox.Size = new System.Drawing.Size(80, 21);
            this.comPortComboBox.TabIndex = 2;
            this.comPortComboBox.MouseDown += new System.Windows.Forms.MouseEventHandler(this.comPortComboBox_MouseDown);
            // 
            // baudComboBox
            // 
            this.baudComboBox.DisplayMember = "4";
            this.baudComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.baudComboBox.FormattingEnabled = true;
            this.baudComboBox.Items.AddRange(new object[] {
            "9600",
            "19200",
            "38400",
            "57600",
            "115200"});
            this.baudComboBox.Location = new System.Drawing.Point(247, 36);
            this.baudComboBox.Margin = new System.Windows.Forms.Padding(2);
            this.baudComboBox.Name = "baudComboBox";
            this.baudComboBox.Size = new System.Drawing.Size(83, 21);
            this.baudComboBox.TabIndex = 4;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(187, 39);
            this.label3.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(58, 13);
            this.label3.TabIndex = 5;
            this.label3.Text = "Baud Rate";
            // 
            // openFileDialog1
            // 
            this.openFileDialog1.FileName = "*.cyacd";
            this.openFileDialog1.FileOk += new System.ComponentModel.CancelEventHandler(this.openFileDialog1_FileOk_1);
            // 
            // FileNameTB
            // 
            this.FileNameTB.AcceptsTab = true;
            this.FileNameTB.Location = new System.Drawing.Point(13, 98);
            this.FileNameTB.Margin = new System.Windows.Forms.Padding(2);
            this.FileNameTB.Name = "FileNameTB";
            this.FileNameTB.ReadOnly = true;
            this.FileNameTB.Size = new System.Drawing.Size(317, 20);
            this.FileNameTB.TabIndex = 9;
            // 
            // browseButton
            // 
            this.browseButton.AutoSize = true;
            this.browseButton.Location = new System.Drawing.Point(345, 95);
            this.browseButton.Margin = new System.Windows.Forms.Padding(2);
            this.browseButton.Name = "browseButton";
            this.browseButton.Size = new System.Drawing.Size(95, 25);
            this.browseButton.TabIndex = 10;
            this.browseButton.Text = "Browse";
            this.browseButton.UseVisualStyleBackColor = true;
            this.browseButton.Click += new System.EventHandler(this.browseButton_Click);
            // 
            // bootloadButton
            // 
            this.bootloadButton.AutoSize = true;
            this.bootloadButton.Location = new System.Drawing.Point(151, 128);
            this.bootloadButton.Margin = new System.Windows.Forms.Padding(2);
            this.bootloadButton.Name = "bootloadButton";
            this.bootloadButton.Size = new System.Drawing.Size(95, 25);
            this.bootloadButton.TabIndex = 11;
            this.bootloadButton.Text = "Bootload";
            this.bootloadButton.UseVisualStyleBackColor = true;
            this.bootloadButton.Click += new System.EventHandler(this.Bootload_Click);
            // 
            // statusStrip1
            // 
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.progressBar1});
            this.statusStrip1.Location = new System.Drawing.Point(0, 339);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Padding = new System.Windows.Forms.Padding(1, 0, 10, 0);
            this.statusStrip1.Size = new System.Drawing.Size(454, 23);
            this.statusStrip1.SizingGrip = false;
            this.statusStrip1.TabIndex = 21;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // progressBar1
            // 
            this.progressBar1.Margin = new System.Windows.Forms.Padding(10, 3, 1, 3);
            this.progressBar1.Name = "progressBar1";
            this.progressBar1.Size = new System.Drawing.Size(428, 17);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label2.Location = new System.Drawing.Point(10, 9);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(235, 13);
            this.label2.TabIndex = 25;
            this.label2.Text = "Step 1: Select COM Port and Baud Rate";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label5.Location = new System.Drawing.Point(10, 75);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(312, 13);
            this.label5.TabIndex = 26;
            this.label5.Text = "Step 2: Select a Bootloadable File and Click Bootload";
            // 
            // textBox_StatusLog
            // 
            this.textBox_StatusLog.BackColor = System.Drawing.Color.White;
            this.textBox_StatusLog.Location = new System.Drawing.Point(12, 165);
            this.textBox_StatusLog.Multiline = true;
            this.textBox_StatusLog.Name = "textBox_StatusLog";
            this.textBox_StatusLog.ReadOnly = true;
            this.textBox_StatusLog.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.textBox_StatusLog.Size = new System.Drawing.Size(428, 160);
            this.textBox_StatusLog.TabIndex = 17;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(10, 147);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(58, 13);
            this.label6.TabIndex = 27;
            this.label6.Text = "Status Log";
            // 
            // backgroundWorker
            // 
            this.backgroundWorker.WorkerReportsProgress = true;
            this.backgroundWorker.DoWork += new System.ComponentModel.DoWorkEventHandler(this.backgroundWorker_DoWork);
            this.backgroundWorker.ProgressChanged += new System.ComponentModel.ProgressChangedEventHandler(this.backgroundWorker_ProgressChanged);
            this.backgroundWorker.RunWorkerCompleted += new System.ComponentModel.RunWorkerCompletedEventHandler(this.backgroundWorker_RunWorkerCompleted);
            // 
            // UIForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoSize = true;
            this.ClientSize = new System.Drawing.Size(454, 362);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.textBox_StatusLog);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.FileNameTB);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.browseButton);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.comPortComboBox);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.baudComboBox);
            this.Controls.Add(this.statusStrip1);
            this.Controls.Add(this.bootloadButton);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Location = new System.Drawing.Point(50, 0);
            this.Margin = new System.Windows.Forms.Padding(2);
            this.MaximizeBox = false;
            this.Name = "UIForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "UART Bootloader Host Application";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.UIForm_FormClosing);
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox comPortComboBox;
        private System.IO.Ports.SerialPort serialPort;
        private System.Windows.Forms.ComboBox baudComboBox;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        private System.Windows.Forms.TextBox FileNameTB;
        private System.Windows.Forms.Button browseButton;
        private System.Windows.Forms.Button bootloadButton;
        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox textBox_StatusLog;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.ToolStripProgressBar progressBar1;
        private System.ComponentModel.BackgroundWorker backgroundWorker;
    }
}

