namespace ImageLogViewer.UI.Logs
{
	partial class RemoteLogControl
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

		#region Component Designer generated code

		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(RemoteLogControl));
            this.toolStrip1 = new System.Windows.Forms.ToolStrip();
            this.tsbTextLogOpenFile = new System.Windows.Forms.ToolStripButton();
            this.tsbSaveLogFile = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripLabel2 = new System.Windows.Forms.ToolStripLabel();
            this.tsedtTextLog = new System.Windows.Forms.ToolStripTextBox();
            this.tsbTextLogListen = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripLabel1 = new System.Windows.Forms.ToolStripLabel();
            this.cbxspFilter = new System.Windows.Forms.ToolStripComboBox();
            this.btnpsSearch = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator3 = new System.Windows.Forms.ToolStripSeparator();
            this.btnpsFilterFatal = new System.Windows.Forms.ToolStripButton();
            this.btnpsFilterErrors = new System.Windows.Forms.ToolStripButton();
            this.btnspWarnings = new System.Windows.Forms.ToolStripButton();
            this.btnspInformation = new System.Windows.Forms.ToolStripButton();
            this.btnspDebug = new System.Windows.Forms.ToolStripButton();
            this.btnspTrace = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator5 = new System.Windows.Forms.ToolStripSeparator();
            this.btnRefreshLog = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator4 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripLabel3 = new System.Windows.Forms.ToolStripLabel();
            this.tslMessageCount = new System.Windows.Forms.ToolStripLabel();
            this.logIconsImageList = new System.Windows.Forms.ImageList(this.components);
            this.logfetcherBGW = new System.ComponentModel.BackgroundWorker();
            this.logRefreshTimer = new System.Windows.Forms.Timer(this.components);
            this.blvLogEntry = new ComponentOwl.BetterListView.BetterListView();
            this.blvLogEntryStatus = new ComponentOwl.BetterListView.BetterListViewColumnHeader();
            this.blvLogEntryTime = new ComponentOwl.BetterListView.BetterListViewColumnHeader();
            this.blvLogEntryCategory = new ComponentOwl.BetterListView.BetterListViewColumnHeader();
            this.blvLogEntryDescription = new ComponentOwl.BetterListView.BetterListViewColumnHeader();
            this.toolStrip1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.blvLogEntry)).BeginInit();
            this.SuspendLayout();
            // 
            // toolStrip1
            // 
            this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.tsbTextLogOpenFile,
            this.tsbSaveLogFile,
            this.toolStripSeparator1,
            this.toolStripLabel2,
            this.tsedtTextLog,
            this.tsbTextLogListen,
            this.toolStripSeparator2,
            this.toolStripLabel1,
            this.cbxspFilter,
            this.btnpsSearch,
            this.toolStripSeparator3,
            this.btnpsFilterFatal,
            this.btnpsFilterErrors,
            this.btnspWarnings,
            this.btnspInformation,
            this.btnspDebug,
            this.btnspTrace,
            this.toolStripSeparator5,
            this.btnRefreshLog,
            this.toolStripSeparator4,
            this.toolStripLabel3,
            this.tslMessageCount});
            this.toolStrip1.Location = new System.Drawing.Point(0, 0);
            this.toolStrip1.Name = "toolStrip1";
            this.toolStrip1.Size = new System.Drawing.Size(751, 25);
            this.toolStrip1.TabIndex = 3;
            this.toolStrip1.Text = "toolStrip1";
            // 
            // tsbTextLogOpenFile
            // 
            this.tsbTextLogOpenFile.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.tsbTextLogOpenFile.Image = global::ImageLogViewer.Properties.Resources.CaptureFolder_24x24;
            this.tsbTextLogOpenFile.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.tsbTextLogOpenFile.Name = "tsbTextLogOpenFile";
            this.tsbTextLogOpenFile.Size = new System.Drawing.Size(23, 22);
            this.tsbTextLogOpenFile.Text = "Open Log File";
            this.tsbTextLogOpenFile.Click += new System.EventHandler(this.tsbTextLogOpenFile_Click);
            // 
            // tsbSaveLogFile
            // 
            this.tsbSaveLogFile.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.tsbSaveLogFile.Image = global::ImageLogViewer.Properties.Resources.Disk;
            this.tsbSaveLogFile.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.tsbSaveLogFile.Name = "tsbSaveLogFile";
            this.tsbSaveLogFile.Size = new System.Drawing.Size(23, 22);
            this.tsbSaveLogFile.Text = "Save Log To File...";
            this.tsbSaveLogFile.Click += new System.EventHandler(this.tsbSaveLogFile_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(6, 25);
            // 
            // toolStripLabel2
            // 
            this.toolStripLabel2.Name = "toolStripLabel2";
            this.toolStripLabel2.Size = new System.Drawing.Size(32, 22);
            this.toolStripLabel2.Text = "Port:";
            // 
            // tsedtTextLog
            // 
            this.tsedtTextLog.Font = new System.Drawing.Font("Segoe UI", 9F);
            this.tsedtTextLog.Name = "tsedtTextLog";
            this.tsedtTextLog.Size = new System.Drawing.Size(100, 25);
            // 
            // tsbTextLogListen
            // 
            this.tsbTextLogListen.CheckOnClick = true;
            this.tsbTextLogListen.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.tsbTextLogListen.Image = ((System.Drawing.Image)(resources.GetObject("tsbTextLogListen.Image")));
            this.tsbTextLogListen.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.tsbTextLogListen.Name = "tsbTextLogListen";
            this.tsbTextLogListen.Size = new System.Drawing.Size(23, 22);
            this.tsbTextLogListen.Text = "Start/Stop Listening";
            this.tsbTextLogListen.CheckedChanged += new System.EventHandler(this.tsbTextLogListen_CheckedChanged);
            // 
            // toolStripSeparator2
            // 
            this.toolStripSeparator2.Name = "toolStripSeparator2";
            this.toolStripSeparator2.Size = new System.Drawing.Size(6, 25);
            // 
            // toolStripLabel1
            // 
            this.toolStripLabel1.Name = "toolStripLabel1";
            this.toolStripLabel1.Size = new System.Drawing.Size(45, 22);
            this.toolStripLabel1.Text = "Search:";
            // 
            // cbxspFilter
            // 
            this.cbxspFilter.AutoSize = false;
            this.cbxspFilter.Name = "cbxspFilter";
            this.cbxspFilter.Size = new System.Drawing.Size(121, 23);
            this.cbxspFilter.TextChanged += new System.EventHandler(this.cbxspFilter_TextChanged);
            // 
            // btnpsSearch
            // 
            this.btnpsSearch.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.btnpsSearch.Image = ((System.Drawing.Image)(resources.GetObject("btnpsSearch.Image")));
            this.btnpsSearch.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.btnpsSearch.Name = "btnpsSearch";
            this.btnpsSearch.Size = new System.Drawing.Size(23, 22);
            this.btnpsSearch.Text = "Search";
            this.btnpsSearch.Click += new System.EventHandler(this.btnpsSearch_Click);
            // 
            // toolStripSeparator3
            // 
            this.toolStripSeparator3.Name = "toolStripSeparator3";
            this.toolStripSeparator3.Size = new System.Drawing.Size(6, 25);
            // 
            // btnpsFilterFatal
            // 
            this.btnpsFilterFatal.Checked = true;
            this.btnpsFilterFatal.CheckOnClick = true;
            this.btnpsFilterFatal.CheckState = System.Windows.Forms.CheckState.Checked;
            this.btnpsFilterFatal.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.btnpsFilterFatal.Image = global::ImageLogViewer.Properties.Resources.TB_Fatal;
            this.btnpsFilterFatal.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.btnpsFilterFatal.Name = "btnpsFilterFatal";
            this.btnpsFilterFatal.Size = new System.Drawing.Size(23, 22);
            this.btnpsFilterFatal.Text = "Show Fatals";
            this.btnpsFilterFatal.Click += new System.EventHandler(this.btnpsFilterFatal_Click);
            // 
            // btnpsFilterErrors
            // 
            this.btnpsFilterErrors.Checked = true;
            this.btnpsFilterErrors.CheckOnClick = true;
            this.btnpsFilterErrors.CheckState = System.Windows.Forms.CheckState.Checked;
            this.btnpsFilterErrors.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.btnpsFilterErrors.Image = global::ImageLogViewer.Properties.Resources.TB_Error;
            this.btnpsFilterErrors.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.btnpsFilterErrors.Name = "btnpsFilterErrors";
            this.btnpsFilterErrors.Size = new System.Drawing.Size(23, 22);
            this.btnpsFilterErrors.Text = "Show Errors";
            this.btnpsFilterErrors.Click += new System.EventHandler(this.btnpsFilterErrors_Click);
            // 
            // btnspWarnings
            // 
            this.btnspWarnings.Checked = true;
            this.btnspWarnings.CheckOnClick = true;
            this.btnspWarnings.CheckState = System.Windows.Forms.CheckState.Checked;
            this.btnspWarnings.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.btnspWarnings.Image = global::ImageLogViewer.Properties.Resources.TB_Warning;
            this.btnspWarnings.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.btnspWarnings.Name = "btnspWarnings";
            this.btnspWarnings.Size = new System.Drawing.Size(23, 22);
            this.btnspWarnings.Text = "Show Warnings";
            this.btnspWarnings.Click += new System.EventHandler(this.btnspWarnings_Click);
            // 
            // btnspInformation
            // 
            this.btnspInformation.Checked = true;
            this.btnspInformation.CheckOnClick = true;
            this.btnspInformation.CheckState = System.Windows.Forms.CheckState.Checked;
            this.btnspInformation.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.btnspInformation.Image = global::ImageLogViewer.Properties.Resources.TB_Information;
            this.btnspInformation.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.btnspInformation.Name = "btnspInformation";
            this.btnspInformation.Size = new System.Drawing.Size(23, 22);
            this.btnspInformation.Text = "Show Information";
            this.btnspInformation.Click += new System.EventHandler(this.btnspInformation_Click);
            // 
            // btnspDebug
            // 
            this.btnspDebug.Checked = true;
            this.btnspDebug.CheckOnClick = true;
            this.btnspDebug.CheckState = System.Windows.Forms.CheckState.Checked;
            this.btnspDebug.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.btnspDebug.Image = global::ImageLogViewer.Properties.Resources.TB_Debug;
            this.btnspDebug.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.btnspDebug.Name = "btnspDebug";
            this.btnspDebug.Size = new System.Drawing.Size(23, 22);
            this.btnspDebug.Text = "Show Debug";
            this.btnspDebug.Click += new System.EventHandler(this.toolStripButton2_Click);
            // 
            // btnspTrace
            // 
            this.btnspTrace.Checked = true;
            this.btnspTrace.CheckOnClick = true;
            this.btnspTrace.CheckState = System.Windows.Forms.CheckState.Checked;
            this.btnspTrace.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.btnspTrace.Image = ((System.Drawing.Image)(resources.GetObject("btnspTrace.Image")));
            this.btnspTrace.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.btnspTrace.Name = "btnspTrace";
            this.btnspTrace.Size = new System.Drawing.Size(23, 22);
            this.btnspTrace.Text = "Show Trace";
            this.btnspTrace.Click += new System.EventHandler(this.toolStripButton3_Click);
            // 
            // toolStripSeparator5
            // 
            this.toolStripSeparator5.Name = "toolStripSeparator5";
            this.toolStripSeparator5.Size = new System.Drawing.Size(6, 25);
            // 
            // btnRefreshLog
            // 
            this.btnRefreshLog.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.btnRefreshLog.Image = global::ImageLogViewer.Properties.Resources.refresh;
            this.btnRefreshLog.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.btnRefreshLog.Name = "btnRefreshLog";
            this.btnRefreshLog.Size = new System.Drawing.Size(23, 22);
            this.btnRefreshLog.Text = "Clear Log";
            this.btnRefreshLog.Click += new System.EventHandler(this.btnRefreshLog_Click);
            // 
            // toolStripSeparator4
            // 
            this.toolStripSeparator4.Name = "toolStripSeparator4";
            this.toolStripSeparator4.Size = new System.Drawing.Size(6, 25);
            // 
            // toolStripLabel3
            // 
            this.toolStripLabel3.Name = "toolStripLabel3";
            this.toolStripLabel3.Size = new System.Drawing.Size(92, 22);
            this.toolStripLabel3.Text = "Message Count:";
            // 
            // tslMessageCount
            // 
            this.tslMessageCount.Name = "tslMessageCount";
            this.tslMessageCount.Size = new System.Drawing.Size(85, 15);
            this.tslMessageCount.Text = "<placeholder>";
            // 
            // logIconsImageList
            // 
            this.logIconsImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("logIconsImageList.ImageStream")));
            this.logIconsImageList.TransparentColor = System.Drawing.Color.Transparent;
            this.logIconsImageList.Images.SetKeyName(0, "TRACE");
            this.logIconsImageList.Images.SetKeyName(1, "DEBUG");
            this.logIconsImageList.Images.SetKeyName(2, "INFO");
            this.logIconsImageList.Images.SetKeyName(3, "WARN");
            this.logIconsImageList.Images.SetKeyName(4, "ERROR");
            this.logIconsImageList.Images.SetKeyName(5, "FATAL");
            // 
            // logfetcherBGW
            // 
            this.logfetcherBGW.WorkerSupportsCancellation = true;
            // 
            // logRefreshTimer
            // 
            this.logRefreshTimer.Enabled = true;
            this.logRefreshTimer.Interval = 10000;
            this.logRefreshTimer.Tick += new System.EventHandler(this.logRefreshTimer_Tick);
            // 
            // blvLogEntry
            // 
            this.blvLogEntry.Columns.Add(this.blvLogEntryStatus);
            this.blvLogEntry.Columns.Add(this.blvLogEntryTime);
            this.blvLogEntry.Columns.Add(this.blvLogEntryCategory);
            this.blvLogEntry.Columns.Add(this.blvLogEntryDescription);
            this.blvLogEntry.Dock = System.Windows.Forms.DockStyle.Fill;
            this.blvLogEntry.ImageList = this.logIconsImageList;
            this.blvLogEntry.Location = new System.Drawing.Point(0, 25);
            this.blvLogEntry.MultiSelect = false;
            this.blvLogEntry.Name = "blvLogEntry";
            this.blvLogEntry.Size = new System.Drawing.Size(751, 191);
            this.blvLogEntry.TabIndex = 5;
            // 
            // blvLogEntryStatus
            // 
            this.blvLogEntryStatus.Name = "blvLogEntryStatus";
            this.blvLogEntryStatus.PreferredSortOrderAscending = false;
            this.blvLogEntryStatus.Style = ComponentOwl.BetterListView.BetterListViewColumnHeaderStyle.Nonclickable;
            this.blvLogEntryStatus.Text = "Status";
            this.blvLogEntryStatus.Width = 72;
            // 
            // blvLogEntryTime
            // 
            this.blvLogEntryTime.MinimumWidth = 128;
            this.blvLogEntryTime.Name = "blvLogEntryTime";
            this.blvLogEntryTime.SortMethod = ComponentOwl.BetterListView.BetterListViewSortMethod.Text;
            this.blvLogEntryTime.Style = ComponentOwl.BetterListView.BetterListViewColumnHeaderStyle.Sortable;
            this.blvLogEntryTime.Text = "Time Stamp";
            // 
            // blvLogEntryCategory
            // 
            this.blvLogEntryCategory.Name = "blvLogEntryCategory";
            this.blvLogEntryCategory.PreferredSortOrderAscending = false;
            this.blvLogEntryCategory.SortMethod = ComponentOwl.BetterListView.BetterListViewSortMethod.Text;
            this.blvLogEntryCategory.Style = ComponentOwl.BetterListView.BetterListViewColumnHeaderStyle.Sortable;
            this.blvLogEntryCategory.Text = "Category";
            // 
            // blvLogEntryDescription
            // 
            this.blvLogEntryDescription.MinimumWidth = 512;
            this.blvLogEntryDescription.Name = "blvLogEntryDescription";
            this.blvLogEntryDescription.PreferredSortOrderAscending = false;
            this.blvLogEntryDescription.SortMethod = ComponentOwl.BetterListView.BetterListViewSortMethod.Text;
            this.blvLogEntryDescription.Style = ComponentOwl.BetterListView.BetterListViewColumnHeaderStyle.Sortable;
            this.blvLogEntryDescription.Text = "Description";
            this.blvLogEntryDescription.Width = 512;
            // 
            // RemoteLogControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.blvLogEntry);
            this.Controls.Add(this.toolStrip1);
            this.Name = "RemoteLogControl";
            this.Size = new System.Drawing.Size(751, 216);
            this.Load += new System.EventHandler(this.RemoteLogControl_Load);
            this.toolStrip1.ResumeLayout(false);
            this.toolStrip1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.blvLogEntry)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.ToolStrip toolStrip1;
		private System.Windows.Forms.ToolStripLabel toolStripLabel1;
		private System.Windows.Forms.ToolStripComboBox cbxspFilter;
		private System.Windows.Forms.ToolStripButton btnpsFilterErrors;
		private System.Windows.Forms.ToolStripButton btnspWarnings;
		private System.Windows.Forms.ToolStripButton btnspInformation;
        private System.Windows.Forms.ToolStripButton btnRefreshLog;
        private System.Windows.Forms.ImageList logIconsImageList;
        private System.ComponentModel.BackgroundWorker logfetcherBGW;
        private System.Windows.Forms.Timer logRefreshTimer;
		private ComponentOwl.BetterListView.BetterListView blvLogEntry;
		private ComponentOwl.BetterListView.BetterListViewColumnHeader blvLogEntryStatus;
		private ComponentOwl.BetterListView.BetterListViewColumnHeader blvLogEntryTime;
		private ComponentOwl.BetterListView.BetterListViewColumnHeader blvLogEntryCategory;
		private ComponentOwl.BetterListView.BetterListViewColumnHeader blvLogEntryDescription;
		private System.Windows.Forms.ToolStripButton tsbTextLogOpenFile;
		private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
		private System.Windows.Forms.ToolStripLabel toolStripLabel2;
		private System.Windows.Forms.ToolStripTextBox tsedtTextLog;
		private System.Windows.Forms.ToolStripButton tsbTextLogListen;
		private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
		private System.Windows.Forms.ToolStripButton btnpsSearch;
		private System.Windows.Forms.ToolStripSeparator toolStripSeparator3;
		private System.Windows.Forms.ToolStripSeparator toolStripSeparator4;
		private System.Windows.Forms.ToolStripLabel toolStripLabel3;
		private System.Windows.Forms.ToolStripLabel tslMessageCount;
        private System.Windows.Forms.ToolStripButton btnpsFilterFatal;
        private System.Windows.Forms.ToolStripButton btnspDebug;
        private System.Windows.Forms.ToolStripButton btnspTrace;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator5;
        private System.Windows.Forms.ToolStripButton tsbSaveLogFile;
    }
}
