namespace ImageLogViewer
{
	partial class MainWindow
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainWindow));
            this.MainSplitContainer = new System.Windows.Forms.SplitContainer();
            this.TopSplitterContainer = new System.Windows.Forms.SplitContainer();
            this.statusStripTreeView = new System.Windows.Forms.StatusStrip();
            this.toolStripStatusLabel1 = new System.Windows.Forms.ToolStripStatusLabel();
            this.toolStripStatusFrameCount = new System.Windows.Forms.ToolStripStatusLabel();
            this.toolStripStatusLabel2 = new System.Windows.Forms.ToolStripStatusLabel();
            this.toolStripStatusCropCount = new System.Windows.Forms.ToolStripStatusLabel();
            this.ImageLogTree = new System.Windows.Forms.TreeView();
            this.ilTreeView = new System.Windows.Forms.ImageList(this.components);
            this.tsTreeView = new System.Windows.Forms.ToolStrip();
            this.toolStripLabel1 = new System.Windows.Forms.ToolStripLabel();
            this.tsSortCombo = new System.Windows.Forms.ToolStripComboBox();
            this.expandTextButton = new System.Windows.Forms.ToolStripButton();
            this.openToolStripButton = new System.Windows.Forms.ToolStripButton();
            this.saveToolStripButton = new System.Windows.Forms.ToolStripButton();
            this.printToolStripButton = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator = new System.Windows.Forms.ToolStripSeparator();
            this.cutToolStripButton = new System.Windows.Forms.ToolStripButton();
            this.copyToolStripButton = new System.Windows.Forms.ToolStripButton();
            this.pasteToolStripButton = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator3 = new System.Windows.Forms.ToolStripSeparator();
            this.helpToolStripButton = new System.Windows.Forms.ToolStripButton();
            this.tabImageData = new System.Windows.Forms.TabControl();
            this.tabPage3 = new System.Windows.Forms.TabPage();
            this.pbImage = new System.Windows.Forms.PictureBox();
            this.toolStrip1 = new System.Windows.Forms.ToolStrip();
            this.imageToolStripToggleData = new System.Windows.Forms.ToolStripButton();
            this.imageToolStripSaveAs = new System.Windows.Forms.ToolStripButton();
            this.tabPage4 = new System.Windows.Forms.TabPage();
            this.pgImageDetails = new System.Windows.Forms.PropertyGrid();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.remoteLogControlNXTLog = new ImageLogViewer.UI.Logs.RemoteLogControl();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.remoteLogFaceTrackingLog = new ImageLogViewer.UI.Logs.RemoteLogControl();
            this.tabPage5 = new System.Windows.Forms.TabPage();
            this.remoteLogMotorLog = new ImageLogViewer.UI.Logs.RemoteLogControl();
            this.tabPage6 = new System.Windows.Forms.TabPage();
            this.remoteLogEventLog = new ImageLogViewer.UI.Logs.RemoteLogControl();
            this.notifyIcon1 = new System.Windows.Forms.NotifyIcon(this.components);
            this.btnPanel = new System.Windows.Forms.Panel();
            this.versionLabel = new System.Windows.Forms.Label();
            this.btnQuit = new System.Windows.Forms.Button();
            this.tsMain = new System.Windows.Forms.ToolStrip();
            this.btnOpen = new System.Windows.Forms.ToolStripButton();
            this.btnSave = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripLabel3 = new System.Windows.Forms.ToolStripLabel();
            this.lblRemotePort = new System.Windows.Forms.ToolStripTextBox();
            this.tsbutStartServer = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
            this.tsDeviceSettings = new System.Windows.Forms.ToolStripButton();
            ((System.ComponentModel.ISupportInitialize)(this.MainSplitContainer)).BeginInit();
            this.MainSplitContainer.Panel1.SuspendLayout();
            this.MainSplitContainer.Panel2.SuspendLayout();
            this.MainSplitContainer.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.TopSplitterContainer)).BeginInit();
            this.TopSplitterContainer.Panel1.SuspendLayout();
            this.TopSplitterContainer.Panel2.SuspendLayout();
            this.TopSplitterContainer.SuspendLayout();
            this.statusStripTreeView.SuspendLayout();
            this.tsTreeView.SuspendLayout();
            this.tabImageData.SuspendLayout();
            this.tabPage3.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pbImage)).BeginInit();
            this.toolStrip1.SuspendLayout();
            this.tabPage4.SuspendLayout();
            this.tabControl1.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.tabPage2.SuspendLayout();
            this.tabPage5.SuspendLayout();
            this.tabPage6.SuspendLayout();
            this.btnPanel.SuspendLayout();
            this.tsMain.SuspendLayout();
            this.SuspendLayout();
            // 
            // MainSplitContainer
            // 
            this.MainSplitContainer.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.MainSplitContainer.Dock = System.Windows.Forms.DockStyle.Fill;
            this.MainSplitContainer.FixedPanel = System.Windows.Forms.FixedPanel.Panel2;
            this.MainSplitContainer.Location = new System.Drawing.Point(0, 31);
            this.MainSplitContainer.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.MainSplitContainer.MinimumSize = new System.Drawing.Size(600, 462);
            this.MainSplitContainer.Name = "MainSplitContainer";
            this.MainSplitContainer.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // MainSplitContainer.Panel1
            // 
            this.MainSplitContainer.Panel1.Controls.Add(this.TopSplitterContainer);
            // 
            // MainSplitContainer.Panel2
            // 
            this.MainSplitContainer.Panel2.Controls.Add(this.tabControl1);
            this.MainSplitContainer.Size = new System.Drawing.Size(763, 462);
            this.MainSplitContainer.SplitterDistance = 229;
            this.MainSplitContainer.SplitterWidth = 6;
            this.MainSplitContainer.TabIndex = 0;
            this.MainSplitContainer.Resize += new System.EventHandler(this.MainSplitContainer_Resize);
            // 
            // TopSplitterContainer
            // 
            this.TopSplitterContainer.Dock = System.Windows.Forms.DockStyle.Fill;
            this.TopSplitterContainer.FixedPanel = System.Windows.Forms.FixedPanel.Panel1;
            this.TopSplitterContainer.Location = new System.Drawing.Point(0, 0);
            this.TopSplitterContainer.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.TopSplitterContainer.Name = "TopSplitterContainer";
            // 
            // TopSplitterContainer.Panel1
            // 
            this.TopSplitterContainer.Panel1.Controls.Add(this.statusStripTreeView);
            this.TopSplitterContainer.Panel1.Controls.Add(this.ImageLogTree);
            this.TopSplitterContainer.Panel1.Controls.Add(this.tsTreeView);
            // 
            // TopSplitterContainer.Panel2
            // 
            this.TopSplitterContainer.Panel2.Controls.Add(this.tabImageData);
            this.TopSplitterContainer.Size = new System.Drawing.Size(759, 225);
            this.TopSplitterContainer.SplitterDistance = 338;
            this.TopSplitterContainer.SplitterWidth = 6;
            this.TopSplitterContainer.TabIndex = 0;
            // 
            // statusStripTreeView
            // 
            this.statusStripTreeView.ImageScalingSize = new System.Drawing.Size(20, 20);
            this.statusStripTreeView.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabel1,
            this.toolStripStatusFrameCount,
            this.toolStripStatusLabel2,
            this.toolStripStatusCropCount});
            this.statusStripTreeView.Location = new System.Drawing.Point(0, 193);
            this.statusStripTreeView.Name = "statusStripTreeView";
            this.statusStripTreeView.Padding = new System.Windows.Forms.Padding(2, 0, 21, 0);
            this.statusStripTreeView.Size = new System.Drawing.Size(338, 32);
            this.statusStripTreeView.SizingGrip = false;
            this.statusStripTreeView.TabIndex = 2;
            this.statusStripTreeView.Text = "statusStrip1";
            // 
            // toolStripStatusLabel1
            // 
            this.toolStripStatusLabel1.Name = "toolStripStatusLabel1";
            this.toolStripStatusLabel1.Size = new System.Drawing.Size(118, 25);
            this.toolStripStatusLabel1.Text = "Frame Count:";
            // 
            // toolStripStatusFrameCount
            // 
            this.toolStripStatusFrameCount.Name = "toolStripStatusFrameCount";
            this.toolStripStatusFrameCount.Size = new System.Drawing.Size(22, 25);
            this.toolStripStatusFrameCount.Text = "0";
            // 
            // toolStripStatusLabel2
            // 
            this.toolStripStatusLabel2.Name = "toolStripStatusLabel2";
            this.toolStripStatusLabel2.Size = new System.Drawing.Size(108, 25);
            this.toolStripStatusLabel2.Text = "Crop Count:";
            // 
            // toolStripStatusCropCount
            // 
            this.toolStripStatusCropCount.Name = "toolStripStatusCropCount";
            this.toolStripStatusCropCount.Size = new System.Drawing.Size(22, 25);
            this.toolStripStatusCropCount.Text = "0";
            // 
            // ImageLogTree
            // 
            this.ImageLogTree.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.ImageLogTree.Dock = System.Windows.Forms.DockStyle.Fill;
            this.ImageLogTree.ImageIndex = 0;
            this.ImageLogTree.ImageList = this.ilTreeView;
            this.ImageLogTree.Location = new System.Drawing.Point(0, 33);
            this.ImageLogTree.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.ImageLogTree.Name = "ImageLogTree";
            this.ImageLogTree.SelectedImageIndex = 0;
            this.ImageLogTree.Size = new System.Drawing.Size(338, 192);
            this.ImageLogTree.TabIndex = 0;
            this.ImageLogTree.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.ImageLogTree_AfterSelect);
            // 
            // ilTreeView
            // 
            this.ilTreeView.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("ilTreeView.ImageStream")));
            this.ilTreeView.TransparentColor = System.Drawing.Color.Transparent;
            this.ilTreeView.Images.SetKeyName(0, "CaptureFolder-24x24.png");
            this.ilTreeView.Images.SetKeyName(1, "CaptureFolderNoEyes-24x24.png");
            this.ilTreeView.Images.SetKeyName(2, "Crop-24.24.png");
            this.ilTreeView.Images.SetKeyName(3, "Crop_Discarded.png");
            this.ilTreeView.Images.SetKeyName(4, "Frame.png");
            // 
            // tsTreeView
            // 
            this.tsTreeView.ImageScalingSize = new System.Drawing.Size(20, 20);
            this.tsTreeView.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripLabel1,
            this.tsSortCombo,
            this.expandTextButton,
            this.openToolStripButton,
            this.saveToolStripButton,
            this.printToolStripButton,
            this.toolStripSeparator,
            this.cutToolStripButton,
            this.copyToolStripButton,
            this.pasteToolStripButton,
            this.toolStripSeparator3,
            this.helpToolStripButton});
            this.tsTreeView.Location = new System.Drawing.Point(0, 0);
            this.tsTreeView.Name = "tsTreeView";
            this.tsTreeView.Padding = new System.Windows.Forms.Padding(0, 0, 3, 0);
            this.tsTreeView.Size = new System.Drawing.Size(338, 33);
            this.tsTreeView.TabIndex = 1;
            this.tsTreeView.Text = "toolStrip2";
            // 
            // toolStripLabel1
            // 
            this.toolStripLabel1.Name = "toolStripLabel1";
            this.toolStripLabel1.Size = new System.Drawing.Size(73, 28);
            this.toolStripLabel1.Text = "Sort By:";
            // 
            // tsSortCombo
            // 
            this.tsSortCombo.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.tsSortCombo.Items.AddRange(new object[] {
            "Time",
            "Camera ID",
            "Image Type"});
            this.tsSortCombo.Name = "tsSortCombo";
            this.tsSortCombo.Size = new System.Drawing.Size(180, 33);
            this.tsSortCombo.SelectedIndexChanged += new System.EventHandler(this.tsSortCombo_SelectedIndexChanged);
            // 
            // expandTextButton
            // 
            this.expandTextButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.expandTextButton.Image = global::ImageLogViewer.Properties.Resources.Details;
            this.expandTextButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.expandTextButton.Name = "expandTextButton";
            this.expandTextButton.Size = new System.Drawing.Size(34, 28);
            this.expandTextButton.Text = "&Expand";
            this.expandTextButton.Click += new System.EventHandler(this.expandTextButton_Click);
            // 
            // openToolStripButton
            // 
            this.openToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.openToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("openToolStripButton.Image")));
            this.openToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.openToolStripButton.Name = "openToolStripButton";
            this.openToolStripButton.Size = new System.Drawing.Size(34, 24);
            this.openToolStripButton.Text = "&Open";
            // 
            // saveToolStripButton
            // 
            this.saveToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.saveToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("saveToolStripButton.Image")));
            this.saveToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.saveToolStripButton.Name = "saveToolStripButton";
            this.saveToolStripButton.Size = new System.Drawing.Size(34, 24);
            this.saveToolStripButton.Text = "&Save";
            // 
            // printToolStripButton
            // 
            this.printToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.printToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("printToolStripButton.Image")));
            this.printToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.printToolStripButton.Name = "printToolStripButton";
            this.printToolStripButton.Size = new System.Drawing.Size(34, 24);
            this.printToolStripButton.Text = "&Print";
            // 
            // toolStripSeparator
            // 
            this.toolStripSeparator.Name = "toolStripSeparator";
            this.toolStripSeparator.Size = new System.Drawing.Size(6, 6);
            // 
            // cutToolStripButton
            // 
            this.cutToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.cutToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("cutToolStripButton.Image")));
            this.cutToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.cutToolStripButton.Name = "cutToolStripButton";
            this.cutToolStripButton.Size = new System.Drawing.Size(34, 24);
            this.cutToolStripButton.Text = "C&ut";
            // 
            // copyToolStripButton
            // 
            this.copyToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.copyToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("copyToolStripButton.Image")));
            this.copyToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.copyToolStripButton.Name = "copyToolStripButton";
            this.copyToolStripButton.Size = new System.Drawing.Size(34, 24);
            this.copyToolStripButton.Text = "&Copy";
            // 
            // pasteToolStripButton
            // 
            this.pasteToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.pasteToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("pasteToolStripButton.Image")));
            this.pasteToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.pasteToolStripButton.Name = "pasteToolStripButton";
            this.pasteToolStripButton.Size = new System.Drawing.Size(34, 24);
            this.pasteToolStripButton.Text = "&Paste";
            // 
            // toolStripSeparator3
            // 
            this.toolStripSeparator3.Name = "toolStripSeparator3";
            this.toolStripSeparator3.Size = new System.Drawing.Size(6, 6);
            // 
            // helpToolStripButton
            // 
            this.helpToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.helpToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("helpToolStripButton.Image")));
            this.helpToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.helpToolStripButton.Name = "helpToolStripButton";
            this.helpToolStripButton.Size = new System.Drawing.Size(34, 24);
            this.helpToolStripButton.Text = "He&lp";
            // 
            // tabImageData
            // 
            this.tabImageData.Controls.Add(this.tabPage3);
            this.tabImageData.Controls.Add(this.tabPage4);
            this.tabImageData.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabImageData.Location = new System.Drawing.Point(0, 0);
            this.tabImageData.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.tabImageData.Name = "tabImageData";
            this.tabImageData.SelectedIndex = 0;
            this.tabImageData.Size = new System.Drawing.Size(415, 225);
            this.tabImageData.TabIndex = 0;
            // 
            // tabPage3
            // 
            this.tabPage3.Controls.Add(this.pbImage);
            this.tabPage3.Controls.Add(this.toolStrip1);
            this.tabPage3.Location = new System.Drawing.Point(4, 29);
            this.tabPage3.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.tabPage3.Name = "tabPage3";
            this.tabPage3.Padding = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.tabPage3.Size = new System.Drawing.Size(407, 192);
            this.tabPage3.TabIndex = 0;
            this.tabPage3.Text = "Image";
            this.tabPage3.UseVisualStyleBackColor = true;
            // 
            // pbImage
            // 
            this.pbImage.BackgroundImageLayout = System.Windows.Forms.ImageLayout.None;
            this.pbImage.Dock = System.Windows.Forms.DockStyle.Fill;
            this.pbImage.Location = new System.Drawing.Point(4, 34);
            this.pbImage.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.pbImage.Name = "pbImage";
            this.pbImage.Size = new System.Drawing.Size(399, 153);
            this.pbImage.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.pbImage.TabIndex = 0;
            this.pbImage.TabStop = false;
            this.pbImage.Paint += new System.Windows.Forms.PaintEventHandler(this.pbImage_Paint);
            // 
            // toolStrip1
            // 
            this.toolStrip1.ImageScalingSize = new System.Drawing.Size(20, 20);
            this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.imageToolStripToggleData,
            this.imageToolStripSaveAs});
            this.toolStrip1.Location = new System.Drawing.Point(4, 5);
            this.toolStrip1.Name = "toolStrip1";
            this.toolStrip1.Padding = new System.Windows.Forms.Padding(0, 0, 3, 0);
            this.toolStrip1.Size = new System.Drawing.Size(399, 29);
            this.toolStrip1.TabIndex = 0;
            this.toolStrip1.Text = "toolStrip1";
            // 
            // imageToolStripToggleData
            // 
            this.imageToolStripToggleData.CheckOnClick = true;
            this.imageToolStripToggleData.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.imageToolStripToggleData.Image = global::ImageLogViewer.Properties.Resources.Details;
            this.imageToolStripToggleData.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.imageToolStripToggleData.Name = "imageToolStripToggleData";
            this.imageToolStripToggleData.Size = new System.Drawing.Size(34, 24);
            this.imageToolStripToggleData.Text = "Toggle Details";
            this.imageToolStripToggleData.CheckedChanged += new System.EventHandler(this.imageToolStripToggleData_CheckedChanged);
            // 
            // imageToolStripSaveAs
            // 
            this.imageToolStripSaveAs.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.imageToolStripSaveAs.Image = global::ImageLogViewer.Properties.Resources.Disk;
            this.imageToolStripSaveAs.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.imageToolStripSaveAs.Name = "imageToolStripSaveAs";
            this.imageToolStripSaveAs.Size = new System.Drawing.Size(34, 24);
            this.imageToolStripSaveAs.Text = "toolStripButton1";
            this.imageToolStripSaveAs.ToolTipText = "Save As...";
            this.imageToolStripSaveAs.Click += new System.EventHandler(this.imageToolStripSaveAs_Click);
            // 
            // tabPage4
            // 
            this.tabPage4.Controls.Add(this.pgImageDetails);
            this.tabPage4.Location = new System.Drawing.Point(4, 29);
            this.tabPage4.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.tabPage4.Name = "tabPage4";
            this.tabPage4.Padding = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.tabPage4.Size = new System.Drawing.Size(967, 204);
            this.tabPage4.TabIndex = 1;
            this.tabPage4.Text = "Details";
            this.tabPage4.UseVisualStyleBackColor = true;
            // 
            // pgImageDetails
            // 
            this.pgImageDetails.Dock = System.Windows.Forms.DockStyle.Fill;
            this.pgImageDetails.Location = new System.Drawing.Point(4, 5);
            this.pgImageDetails.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.pgImageDetails.Name = "pgImageDetails";
            this.pgImageDetails.Size = new System.Drawing.Size(959, 194);
            this.pgImageDetails.TabIndex = 0;
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.tabPage1);
            this.tabControl1.Controls.Add(this.tabPage2);
            this.tabControl1.Controls.Add(this.tabPage5);
            this.tabControl1.Controls.Add(this.tabPage6);
            this.tabControl1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabControl1.Location = new System.Drawing.Point(0, 0);
            this.tabControl1.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(759, 223);
            this.tabControl1.TabIndex = 0;
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.remoteLogControlNXTLog);
            this.tabPage1.Location = new System.Drawing.Point(4, 29);
            this.tabPage1.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.tabPage1.Size = new System.Drawing.Size(751, 190);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "NXT Log";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // remoteLogControlNXTLog
            // 
            this.remoteLogControlNXTLog.BottomColor = System.Drawing.Color.FromArgb(((int)(((byte)(238)))), ((int)(((byte)(238)))), ((int)(((byte)(238)))));
            this.remoteLogControlNXTLog.Dock = System.Windows.Forms.DockStyle.Fill;
            this.remoteLogControlNXTLog.Location = new System.Drawing.Point(4, 5);
            this.remoteLogControlNXTLog.Margin = new System.Windows.Forms.Padding(6);
            this.remoteLogControlNXTLog.Name = "remoteLogControlNXTLog";
            this.remoteLogControlNXTLog.Size = new System.Drawing.Size(743, 180);
            this.remoteLogControlNXTLog.TabIndex = 0;
            this.remoteLogControlNXTLog.TopColor = System.Drawing.Color.White;
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.remoteLogFaceTrackingLog);
            this.tabPage2.Location = new System.Drawing.Point(4, 29);
            this.tabPage2.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.tabPage2.Size = new System.Drawing.Size(1311, 190);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "FaceTracking Log";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // remoteLogFaceTrackingLog
            // 
            this.remoteLogFaceTrackingLog.BottomColor = System.Drawing.Color.FromArgb(((int)(((byte)(238)))), ((int)(((byte)(238)))), ((int)(((byte)(238)))));
            this.remoteLogFaceTrackingLog.Dock = System.Windows.Forms.DockStyle.Fill;
            this.remoteLogFaceTrackingLog.Location = new System.Drawing.Point(4, 5);
            this.remoteLogFaceTrackingLog.Margin = new System.Windows.Forms.Padding(6);
            this.remoteLogFaceTrackingLog.Name = "remoteLogFaceTrackingLog";
            this.remoteLogFaceTrackingLog.Size = new System.Drawing.Size(1303, 180);
            this.remoteLogFaceTrackingLog.TabIndex = 0;
            this.remoteLogFaceTrackingLog.TopColor = System.Drawing.Color.White;
            // 
            // tabPage5
            // 
            this.tabPage5.Controls.Add(this.remoteLogMotorLog);
            this.tabPage5.Location = new System.Drawing.Point(4, 29);
            this.tabPage5.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.tabPage5.Name = "tabPage5";
            this.tabPage5.Padding = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.tabPage5.Size = new System.Drawing.Size(1311, 190);
            this.tabPage5.TabIndex = 2;
            this.tabPage5.Text = "Motor Log";
            this.tabPage5.UseVisualStyleBackColor = true;
            // 
            // remoteLogMotorLog
            // 
            this.remoteLogMotorLog.BottomColor = System.Drawing.Color.FromArgb(((int)(((byte)(238)))), ((int)(((byte)(238)))), ((int)(((byte)(238)))));
            this.remoteLogMotorLog.Dock = System.Windows.Forms.DockStyle.Fill;
            this.remoteLogMotorLog.Location = new System.Drawing.Point(4, 5);
            this.remoteLogMotorLog.Margin = new System.Windows.Forms.Padding(6);
            this.remoteLogMotorLog.Name = "remoteLogMotorLog";
            this.remoteLogMotorLog.Size = new System.Drawing.Size(1303, 180);
            this.remoteLogMotorLog.TabIndex = 0;
            this.remoteLogMotorLog.TopColor = System.Drawing.Color.White;
            // 
            // tabPage6
            // 
            this.tabPage6.Controls.Add(this.remoteLogEventLog);
            this.tabPage6.Location = new System.Drawing.Point(4, 29);
            this.tabPage6.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.tabPage6.Name = "tabPage6";
            this.tabPage6.Padding = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.tabPage6.Size = new System.Drawing.Size(1311, 190);
            this.tabPage6.TabIndex = 3;
            this.tabPage6.Text = "Event Log";
            this.tabPage6.UseVisualStyleBackColor = true;
            // 
            // remoteLogEventLog
            // 
            this.remoteLogEventLog.BottomColor = System.Drawing.Color.FromArgb(((int)(((byte)(238)))), ((int)(((byte)(238)))), ((int)(((byte)(238)))));
            this.remoteLogEventLog.Dock = System.Windows.Forms.DockStyle.Fill;
            this.remoteLogEventLog.Location = new System.Drawing.Point(4, 5);
            this.remoteLogEventLog.Margin = new System.Windows.Forms.Padding(6);
            this.remoteLogEventLog.Name = "remoteLogEventLog";
            this.remoteLogEventLog.Size = new System.Drawing.Size(1303, 180);
            this.remoteLogEventLog.TabIndex = 0;
            this.remoteLogEventLog.TopColor = System.Drawing.Color.White;
            // 
            // notifyIcon1
            // 
            this.notifyIcon1.Text = "notifyIcon1";
            this.notifyIcon1.Visible = true;
            // 
            // btnPanel
            // 
            this.btnPanel.Controls.Add(this.versionLabel);
            this.btnPanel.Controls.Add(this.btnQuit);
            this.btnPanel.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.btnPanel.Location = new System.Drawing.Point(0, 446);
            this.btnPanel.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.btnPanel.Name = "btnPanel";
            this.btnPanel.Size = new System.Drawing.Size(763, 65);
            this.btnPanel.TabIndex = 1;
            // 
            // versionLabel
            // 
            this.versionLabel.AutoSize = true;
            this.versionLabel.Location = new System.Drawing.Point(18, 29);
            this.versionLabel.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.versionLabel.Name = "versionLabel";
            this.versionLabel.Size = new System.Drawing.Size(63, 20);
            this.versionLabel.TabIndex = 1;
            this.versionLabel.Text = "Version";
            // 
            // btnQuit
            // 
            this.btnQuit.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnQuit.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnQuit.Location = new System.Drawing.Point(633, 12);
            this.btnQuit.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.btnQuit.Name = "btnQuit";
            this.btnQuit.Size = new System.Drawing.Size(112, 35);
            this.btnQuit.TabIndex = 0;
            this.btnQuit.Text = "Quit";
            this.btnQuit.UseVisualStyleBackColor = true;
            this.btnQuit.Click += new System.EventHandler(this.btnQuit_Click);
            // 
            // tsMain
            // 
            this.tsMain.ImageScalingSize = new System.Drawing.Size(20, 20);
            this.tsMain.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.btnOpen,
            this.btnSave,
            this.toolStripSeparator1,
            this.toolStripLabel3,
            this.lblRemotePort,
            this.tsbutStartServer,
            this.toolStripSeparator2,
            this.tsDeviceSettings});
            this.tsMain.Location = new System.Drawing.Point(0, 0);
            this.tsMain.Name = "tsMain";
            this.tsMain.Padding = new System.Windows.Forms.Padding(0, 0, 3, 0);
            this.tsMain.Size = new System.Drawing.Size(763, 31);
            this.tsMain.TabIndex = 2;
            this.tsMain.Text = "toolStrip2";
            // 
            // btnOpen
            // 
            this.btnOpen.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.btnOpen.Image = global::ImageLogViewer.Properties.Resources.CaptureFolder_24x24;
            this.btnOpen.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.btnOpen.Name = "btnOpen";
            this.btnOpen.Size = new System.Drawing.Size(34, 26);
            this.btnOpen.Text = "toolStripButton1";
            this.btnOpen.Click += new System.EventHandler(this.btnOpen_Click);
            // 
            // btnSave
            // 
            this.btnSave.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.btnSave.Enabled = false;
            this.btnSave.Image = global::ImageLogViewer.Properties.Resources.Disk;
            this.btnSave.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.btnSave.Name = "btnSave";
            this.btnSave.Size = new System.Drawing.Size(34, 26);
            this.btnSave.Text = "Save File...";
            this.btnSave.Click += new System.EventHandler(this.btnSave_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(6, 31);
            // 
            // toolStripLabel3
            // 
            this.toolStripLabel3.Name = "toolStripLabel3";
            this.toolStripLabel3.Size = new System.Drawing.Size(48, 26);
            this.toolStripLabel3.Text = "Port:";
            // 
            // lblRemotePort
            // 
            this.lblRemotePort.Name = "lblRemotePort";
            this.lblRemotePort.Size = new System.Drawing.Size(148, 31);
            // 
            // tsbutStartServer
            // 
            this.tsbutStartServer.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.tsbutStartServer.Image = global::ImageLogViewer.Properties.Resources.icons8_refresh_26;
            this.tsbutStartServer.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.tsbutStartServer.Name = "tsbutStartServer";
            this.tsbutStartServer.Size = new System.Drawing.Size(34, 26);
            this.tsbutStartServer.Text = "Refresh Listening";
            this.tsbutStartServer.Click += new System.EventHandler(this.tsbutStartServer_Click);
            // 
            // toolStripSeparator2
            // 
            this.toolStripSeparator2.Name = "toolStripSeparator2";
            this.toolStripSeparator2.Size = new System.Drawing.Size(6, 31);
            // 
            // tsDeviceSettings
            // 
            this.tsDeviceSettings.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.tsDeviceSettings.Image = global::ImageLogViewer.Properties.Resources.icons8_settings_24;
            this.tsDeviceSettings.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.tsDeviceSettings.Name = "tsDeviceSettings";
            this.tsDeviceSettings.Size = new System.Drawing.Size(34, 26);
            this.tsDeviceSettings.Text = "Enable/Configure Remote Logging";
            this.tsDeviceSettings.Click += new System.EventHandler(this.toolStripButton1_Click);
            // 
            // MainWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(9F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = global::ImageLogViewer.Properties.Settings.Default.Size;
            this.Controls.Add(this.MainSplitContainer);
            this.Controls.Add(this.btnPanel);
            this.Controls.Add(this.tsMain);
            this.DataBindings.Add(new System.Windows.Forms.Binding("Location", global::ImageLogViewer.Properties.Settings.Default, "Location", true, System.Windows.Forms.DataSourceUpdateMode.OnPropertyChanged));
            this.DataBindings.Add(new System.Windows.Forms.Binding("ClientSize", global::ImageLogViewer.Properties.Settings.Default, "Size", true, System.Windows.Forms.DataSourceUpdateMode.OnPropertyChanged));
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Location = global::ImageLogViewer.Properties.Settings.Default.Location;
            this.Margin = new System.Windows.Forms.Padding(6);
            this.Name = "MainWindow";
            this.Text = "EyeLock Image Log Viewer";
            this.MaximizedBoundsChanged += new System.EventHandler(this.MainWindow_MaximizedBoundsChanged);
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainWindow_FormClosing);
            this.Load += new System.EventHandler(this.MainWindow_Load);
            this.SizeChanged += new System.EventHandler(this.MainWindow_SizeChanged);
            this.MainSplitContainer.Panel1.ResumeLayout(false);
            this.MainSplitContainer.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.MainSplitContainer)).EndInit();
            this.MainSplitContainer.ResumeLayout(false);
            this.TopSplitterContainer.Panel1.ResumeLayout(false);
            this.TopSplitterContainer.Panel1.PerformLayout();
            this.TopSplitterContainer.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.TopSplitterContainer)).EndInit();
            this.TopSplitterContainer.ResumeLayout(false);
            this.statusStripTreeView.ResumeLayout(false);
            this.statusStripTreeView.PerformLayout();
            this.tsTreeView.ResumeLayout(false);
            this.tsTreeView.PerformLayout();
            this.tabImageData.ResumeLayout(false);
            this.tabPage3.ResumeLayout(false);
            this.tabPage3.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pbImage)).EndInit();
            this.toolStrip1.ResumeLayout(false);
            this.toolStrip1.PerformLayout();
            this.tabPage4.ResumeLayout(false);
            this.tabControl1.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.tabPage2.ResumeLayout(false);
            this.tabPage5.ResumeLayout(false);
            this.tabPage6.ResumeLayout(false);
            this.btnPanel.ResumeLayout(false);
            this.btnPanel.PerformLayout();
            this.tsMain.ResumeLayout(false);
            this.tsMain.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.SplitContainer MainSplitContainer;
		private System.Windows.Forms.SplitContainer TopSplitterContainer;
		private System.Windows.Forms.TreeView ImageLogTree;
		private System.Windows.Forms.NotifyIcon notifyIcon1;
		private System.Windows.Forms.Panel btnPanel;
		private System.Windows.Forms.Button btnQuit;
		private System.Windows.Forms.PictureBox pbImage;
		private System.Windows.Forms.ImageList ilTreeView;
		private System.Windows.Forms.TabControl tabImageData;
		private System.Windows.Forms.TabPage tabPage3;
		private System.Windows.Forms.ToolStrip toolStrip1;
		private System.Windows.Forms.TabPage tabPage4;
		private System.Windows.Forms.PropertyGrid pgImageDetails;
		private System.Windows.Forms.ToolStripButton imageToolStripToggleData;
		private System.Windows.Forms.TabControl tabControl1;
		private System.Windows.Forms.TabPage tabPage1;
		private System.Windows.Forms.TabPage tabPage6;
		private System.Windows.Forms.ToolStrip tsTreeView;
		private System.Windows.Forms.ToolStripLabel toolStripLabel1;
		private System.Windows.Forms.ToolStripComboBox tsSortCombo;
		private System.Windows.Forms.ToolStripButton imageToolStripSaveAs;
		private System.Windows.Forms.ToolStrip tsMain;
		private System.Windows.Forms.ToolStripButton btnOpen;
		private System.Windows.Forms.StatusStrip statusStripTreeView;
		private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel1;
		private System.Windows.Forms.ToolStripStatusLabel toolStripStatusFrameCount;
		private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel2;
		private System.Windows.Forms.ToolStripStatusLabel toolStripStatusCropCount;
		private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
		private System.Windows.Forms.ToolStripLabel toolStripLabel3;
		private System.Windows.Forms.ToolStripTextBox lblRemotePort;
		private System.Windows.Forms.ToolStripButton tsbutStartServer;
		private System.Windows.Forms.ToolStripButton btnSave;
		private UI.Logs.RemoteLogControl remoteLogControlNXTLog;
		private System.Windows.Forms.TabPage tabPage2;
		private UI.Logs.RemoteLogControl remoteLogFaceTrackingLog;
		private UI.Logs.RemoteLogControl remoteLogEventLog;
		private System.Windows.Forms.TabPage tabPage5;
		private UI.Logs.RemoteLogControl remoteLogMotorLog;
		private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
		private System.Windows.Forms.ToolStripButton tsDeviceSettings;
        private System.Windows.Forms.Label versionLabel;
        private System.Windows.Forms.ToolStripButton expandTextButton;
        private System.Windows.Forms.ToolStripButton openToolStripButton;
        private System.Windows.Forms.ToolStripButton saveToolStripButton;
        private System.Windows.Forms.ToolStripButton printToolStripButton;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator;
        private System.Windows.Forms.ToolStripButton cutToolStripButton;
        private System.Windows.Forms.ToolStripButton copyToolStripButton;
        private System.Windows.Forms.ToolStripButton pasteToolStripButton;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator3;
        private System.Windows.Forms.ToolStripButton helpToolStripButton;
    }
}

