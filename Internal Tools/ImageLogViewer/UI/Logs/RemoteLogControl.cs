using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;

using ImageLogViewer.UI.UserControls;
using ComponentOwl.BetterListView;
using System.Threading;
using System.IO;
using System.Xml;
using System.Xml.Linq;

namespace ImageLogViewer.UI.Logs
{
	public partial class RemoteLogControl : GradientBaseUserControl
	{
        DateTime lastRefresh;
        LogSeverityFilter filter = new LogSeverityFilter();
        DateTime epoch = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);
		List<LogEntry> m_LogEntries;
		string m_CurrentFilename;

		// The thread and TCP connection...
        private MyTCPLogServer m_LogServer = null;
		private Thread m_LogServerThread = null;


		BetterListViewSearchSettings m_theSearchSettings = new BetterListViewSearchSettings(
			BetterListViewSearchMode.Substring,
			BetterListViewSearchOptions.UpdateSearchHighlight);


		public RemoteLogControl()
		{

			InitializeComponent();

#if null
			// Enable live search...
			blvLogEntry.SearchSettings = new BetterListViewSearchSettings(
				blvLogEntry.SearchSettings.Mode,
				blvLogEntry.SearchSettings.Options | BetterListViewSearchOptions.UpdateSearchHighlight,
				blvLogEntry.SearchSettings.SubItemIndices);
#endif

			Dock = DockStyle.Fill;
			blvLogEntry.Anchor = AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right | AnchorStyles.Top;


#if null //DMO listchange
#region delegates            

            this.blvLogEntryTime.AspectToStringConverter = delegate(object x)
            {
                return epoch.AddMilliseconds((long)x).ToLocalTime().ToString();
            };
            this.blvLogEntryStatus.AspectToStringConverter = delegate(object x)
            {
                return string.Empty;
            };
            this.blvLogEntryStatus.ImageGetter = delegate(object x)
            {
                LogEntry l = x as LogEntry;
                return ((Protocol.LOGMSGSEVERITY)l.Severity).ToString();
            };
#endregion
#endif //DMO listchange
		}

        bool shouldRefrshLog()
        {
            if (lastRefresh == null) return true; // no refresh done so far
            double secSinceLastRefresh=(DateTime.Now - lastRefresh).TotalSeconds;
            return (secSinceLastRefresh > 59d);
        }


        internal void RefreshLogsIfRequired(bool bForce = false)
        {
            if (shouldRefrshLog() || bForce)
                btnRefreshLog_Click(null, null);
        }


		internal void LoadLogEntriesFromFile(string theFilename)
		{
            try
            {
				m_LogEntries = LoadAllLogEntries(theFilename);
				tslMessageCount.Text = m_LogEntries.Count.ToString();

				PopulateLogList(); // Now that all entries have been loaded, populate the list from scratch...

				tsbSaveLogFile.Enabled = false;
            }
            catch (Exception ex)
            {
                Console.WriteLine("Message: {0} Stack: {1}", ex.Message, ex.StackTrace);
                return; // can not proceed if we could not connect
            }
            finally
            {
                lastRefresh = DateTime.Now;
                System.Console.WriteLine("{0} logs refresh attempted", lastRefresh);
            }
		}


		private void LoadSettings()
		{
			TabControl theParent = Parent as TabControl;
			switch (Parent.TabIndex)
			{
				case 0: // NXTLog
				{
					tsedtTextLog.Text = Properties.Settings.Default.NXTLogPort;
					break;
				}

				case 1: // FaceLog
				{
					tsedtTextLog.Text = Properties.Settings.Default.FaceLogPort;
					break;
				}

				case 2: // MotorLog
				{
					tsedtTextLog.Text = Properties.Settings.Default.MotorLogPort;
					break;
				}

				case 3: // EventLog
				{
					tsedtTextLog.Text = Properties.Settings.Default.EventLogPort;
					break;
				}
			}
		}


		private List<LogEntry> LoadAllLogEntries(string theFilename)
		{
			List<LogEntry> theEntries = new List<LogEntry>();

			using (FileStream fs = File.Open(theFilename, FileMode.Open, FileAccess.Read, FileShare.ReadWrite))
			using (BufferedStream bs = new BufferedStream(fs))
			{
				int nCount = 0;
				byte[] dataBuffer = new byte[1024 * 1024];
				string tempdata = "";
				string data = "";


				while (true)
				{
					try
					{
						// Read until we find an end marker, then extract the entire object... and return it...
						nCount = bs.Read(dataBuffer, 0, 1024 * 1024);

						if (nCount > 0)
						{
							tempdata = Encoding.UTF8.GetString(dataBuffer, 0, nCount);

							data += tempdata;

							while (true)
							{
								// Do we have a full message?
								int nIndex = data.IndexOf("</log4j:event>");

								// didn't find our substring...
								if (nIndex <= 0) // Must read some more...
									break;
								else // Found it, store the data...
								{
									nIndex += 14; // We want to include the trailing quote and bracket... we know it's there...

									int nStartIndex = data.IndexOf("<log4j:event");

									if (nStartIndex >= 0)
									{
										LogEntry theEntry;

										theEntry = ParseLogLine(data.Substring(nStartIndex, nIndex - nStartIndex), true);
										if (theEntry != null)
											theEntries.Add(theEntry);

										// Hold onto the remainder of string... we'll need it for next time...
										data = data.Substring(nIndex);
										continue;
									}
									else // were messed up on this one... drop it...
									{
										data = "";
										break;
									}
								}
							}
						}
						else
							break; // No more data...
					}
					catch (Exception)
					{
					}
				}
			}

			return theEntries;
		}


		public void AppendLogEntry(string theLine, bool bAsXML)
		{
			if (null == m_LogEntries)
				m_LogEntries = new List<LogEntry>();

			LogEntry theEntry;

			theEntry = ParseLogLine(theLine, bAsXML);

			if (theEntry != null)
			{
				BetterListViewItem theItem = null;
				int nSelectedIndex = -1;

				// Check index of selected item (if any
				BetterListViewSelectedIndexCollection theIndicies = blvLogEntry.SelectedIndices;
				bool bSelectedIsBottom = ((theIndicies.Count > 0) && (theIndicies[0] == (blvLogEntry.Items.Count - 1)));
				if (bSelectedIsBottom)
					nSelectedIndex = theIndicies[0];

				m_LogEntries.Add(theEntry);

				blvLogEntry.BeginUpdate();
				blvLogEntry.SuspendSort();

				// If this entry matches our current filter, then add it...
				if (!filter.IsFiltered((int)theEntry.Severity))
				{
					theItem = new BetterListViewItem();

					theItem.Text = "";
					theItem.ImageKey = ((LOGMSGSEVERITY)theEntry.Severity).ToString();

					theItem.SubItems.Add(epoch.AddMilliseconds((long)Convert.ToInt64(theEntry.TimeStamp)).ToLocalTime().ToString());
					theItem.SubItems.Add(theEntry.Category);
					theItem.SubItems.Add(theEntry.Message);

					// Now add the item to the list
					blvLogEntry.Items.Add(theItem);
				}

				blvLogEntry.ResumeSort();
				blvLogEntry.EndUpdate();

				tslMessageCount.Text = blvLogEntry.Items.Count.ToString();

				// Handle scrolling.  If no selection or last item is selected, we scroll to the bottom
				// If there is a selection somewhere other than the last item...  we don't scroll the list...
				if (theItem != null)
				{
					// Now scroll to the bottom of the list if necessary...
					BetterListViewSelectedItemCollection theItems = blvLogEntry.SelectedItems;

					if (theItems.Count == 0)
					{
						// If no item is selected, scroll to the bottom...
						blvLogEntry.EnsureVisible(blvLogEntry.Items.Count - 1);
					}
					else if (bSelectedIsBottom) // If there is a selection, and it's the bottom, reselect the new bottom item...
					{
						blvLogEntry.Items[nSelectedIndex].Selected = false;
						blvLogEntry.Items[blvLogEntry.Items.Count - 1].Selected = true;
						blvLogEntry.EnsureVisible(blvLogEntry.Items.Count - 1);
					}
				}
			}
		}


		private LogEntry ParseLogLine(string theLine, bool bAsXML)
		{
			if (theLine.Length > 0)
			{
				// Example line... 2020-01-30, 15:06:40.678, ERROR, [BobListener] - BoB => internal_write_reg - write Bad file descriptor, ::
				LogEntry theLogEntry = new LogEntry();

				if (bAsXML)
				{
					theLine = theLine.Replace("log4j:", "");

					XmlDocument doc = new XmlDocument();
					doc.LoadXml(theLine); // Use .Load() if loading from a file

					XmlNodeList elemList = doc.GetElementsByTagName("event");
					for (int i = 0; i < elemList.Count; i++)
					{
						XmlNode theNode = elemList[i];

						// First the attributes... then the message...
						XmlAttributeCollection attrList = theNode.Attributes;

						foreach (XmlAttribute attr in attrList)
						{
							if (attr.Name == "timestamp")
								theLogEntry.TimeStamp = attr.Value;
							if (attr.Name == "level")
								theLogEntry.Severity = theLogEntry.StringToSeverity(attr.Value);
						}

						// Now dig into the message
						XmlNodeList childList = theNode.ChildNodes;
						for (int j = 0; j < childList.Count; j++)
						{
							if (childList[j].Name == "message")
							{
								XmlNodeList cchildList = childList[j].ChildNodes;
								for (int k = 0; k < cchildList.Count; k++)
								{
									string theToken = (String)cchildList[k].InnerText;

									// Extract the category...
									string[] theTokens = theToken.Split('-');

									if (theTokens.Count() > 0)
									{
										if (theTokens.Count() > 1)
										{
											theLogEntry.Category = theTokens[0].Trim();
											theLogEntry.Message = theTokens[1].Trim();
										}
										else
											theLogEntry.Message = theTokens[0].Trim();
									}
									else
										theLogEntry.Message = theToken.Trim();
								}
							}
						}
					}

					return theLogEntry;
				}
				else
				{
					// Example line... 2020-01-30, 15:06:40.678, ERROR, [BobListener] - BoB => internal_write_reg - write Bad file descriptor, ::
					string []theStrings = theLine.Split(',');

					// Specific ordering here...
					int nIndex = 0;

					foreach (string theToken in theStrings)
					{
						switch (nIndex++)
						{
							case 0: // Date
							{
								theLogEntry.TimeStamp = theToken.Trim();
								continue;			
							}

							case 1: // Time
							{
								theLogEntry.TimeStamp += " - " + theToken.Trim();
								continue;			
							}

							case 2: // Level
							{
								theLogEntry.Severity = theLogEntry.StringToSeverity(theToken.Trim());
								continue;
							}

							default:
							case 3: // Message
							{
								// Extract the category...
								string []theTokens = theToken.Split('-');

								if (theTokens.Count() > 0)
								{
									if (theTokens.Count() > 1)
									{
										theLogEntry.Category = theTokens[0].Trim();
										theLogEntry.Message = theTokens[1].Trim();
									}
									else
										theLogEntry.Message = theTokens[0].Trim();
								}
								else
									theLogEntry.Message = theToken.Trim();
								
								break;
							}
						}

						if (nIndex >= 3)
							break;
					}

					return theLogEntry;
				}
			}
			else
				return null;
		}

        private void btnRefreshLog_Click(object sender, EventArgs e)
        {
			ClearUI();
		}


		private void ClearUI()
		{
			// Clear the log...
			blvLogEntry.BeginUpdate();
			blvLogEntry.SuspendSort();
			blvLogEntry.Items.Clear();
			blvLogEntry.ResumeSort();
			blvLogEntry.EndUpdate();

			if (null != m_LogEntries)
			{
				m_LogEntries.Clear();
				tslMessageCount.Text = m_LogEntries.Count.ToString();
			}
		}


		private void btnpsFilterFatal_Click(object sender, EventArgs e)
		{
			filter.Flip(LOGMSGSEVERITY.FATAL);
			PopulateLogList();
		}

		private void btnpsFilterErrors_Click(object sender, EventArgs e)
        {
            filter.Flip(LOGMSGSEVERITY.ERROR);
			PopulateLogList();
        }
        private void btnspWarnings_Click(object sender, EventArgs e)
        {
            filter.Flip(LOGMSGSEVERITY.WARN);
			PopulateLogList();
        }

        private void btnspInformation_Click(object sender, EventArgs e)
        {
            filter.Flip(LOGMSGSEVERITY.INFO);
			PopulateLogList();
        }

		private void toolStripButton2_Click(object sender, EventArgs e)
		{
			filter.Flip(LOGMSGSEVERITY.DEBUG);
			PopulateLogList();
		}

		private void toolStripButton3_Click(object sender, EventArgs e)
		{
			filter.Flip(LOGMSGSEVERITY.TRACE);
			PopulateLogList();
		}


		private void cbxspFilter_TextChanged(object sender, EventArgs e)
        {
		//	blvLogEntry.FindItemsWithText(cbxspFilter.Text, m_theSearchSettings);
		// DMOTODO
#if null
            string txt= cbxspFilter.Text;
            if (String.IsNullOrEmpty(txt))
            {
                blvLogEntry.DefaultRenderer = null;
                blvLogEntry.AdditionalFilter = null;
            }
            else
            {
                TextMatchFilter filter = filter = TextMatchFilter.Contains(blvLogEntry, txt);
                blvLogEntry.DefaultRenderer = new HighlightTextRenderer(filter);
                HighlightTextRenderer highlightingRenderer = blvLogEntry.GetColumn(0).Renderer as HighlightTextRenderer;
                if (highlightingRenderer != null)
                    highlightingRenderer.Filter = filter;

                blvLogEntry.AdditionalFilter = filter;
            }
#endif
        }


        private void PopulateLogList()
		{
			if (null == m_LogEntries)
				return;

			Cursor.Current = Cursors.WaitCursor;

			blvLogEntry.BeginUpdate();
			blvLogEntry.SuspendSort();
			blvLogEntry.Items.Clear();

			foreach (LogEntry theEntry in m_LogEntries)
			{
				// If this entry matches our current filter, then add it...
				if (!filter.IsFiltered((int)theEntry.Severity))
				{
					BetterListViewItem theItem = new BetterListViewItem();

					theItem.Text = "";
					theItem.ImageKey = ((LOGMSGSEVERITY)theEntry.Severity).ToString();

					theItem.SubItems.Add(epoch.AddMilliseconds((long)Convert.ToInt64(theEntry.TimeStamp)).ToLocalTime().ToString());
					theItem.SubItems.Add(theEntry.Category);
					theItem.SubItems.Add(theEntry.Message);

					// Now add the item to the list
					blvLogEntry.Items.Add(theItem);
				}
			}

			blvLogEntry.ResumeSort();
			blvLogEntry.EndUpdate();

			Cursor.Current = Cursors.Default;
		}

        internal void OnClose()
        {
           // logfetcherBGW.CancelAsync(); //dont hold up on appclose
        }

        private void logRefreshTimer_Tick(object sender, EventArgs e)
        {
           // RefreshLogsIfRequired();
        }

		private void RemoteLogControl_Load(object sender, EventArgs e)
		{
	        this.Dock = DockStyle.Fill;

			LoadSettings();
		}


		public void SetStreamFilename(string theFileName)
		{
			m_CurrentFilename = theFileName;
			// Enable the Save As button...
			tsbSaveLogFile.Enabled = true;
		}

#if null
		private void tsbTextLogListen_CheckedChanged(object sender, EventArgs e)
		{
			if (tsbTextLogListen.Checked)
			{
				int nPort;

				if (tsedtTextLog.Text == "" || !Int32.TryParse(tsedtTextLog.Text, out nPort))
					MessageBox.Show("Invalid Port specified!  Please enter a valid port number to listen on...", "Socket Error",  MessageBoxButtons.OK, MessageBoxIcon.Error);
				else
				{
					m_LogServer = new MyTCPLogServer(nPort, this, LogMode.MODE_TEXT);
					SetStreamFilename(m_LogServer.tempFilename);
					m_LogServerThread = new Thread(new ThreadStart(m_LogServer.listen));
					m_LogServerThread.Start();

					// Depending on the parent tab... store our port #
					TabControl theParent = Parent as TabControl;

					switch (Parent.TabIndex)
					{
						case 0: // NXTLog
						{
							Properties.Settings.Default.NXTLogPort = tsedtTextLog.Text;
							break;
						}

						case 1: // FaceLog
						{
							Properties.Settings.Default.FaceLogPort = tsedtTextLog.Text;
							break;
						}

						case 2: // MotorLog
						{
							Properties.Settings.Default.MotorLogPort = tsedtTextLog.Text;
							break;
						}

						case 3: // EventLog
						{
							Properties.Settings.Default.EventLogPort = tsedtTextLog.Text;
							break;
						}
					}
				}
			}
			else if (null != m_LogServer)
			{
				// Cleaning up, clear everything out...
				ClearUI();

				m_LogServer.Kill(1000);
				m_LogServerThread.Interrupt();

				m_LogServer = null;
				m_LogServerThread = null;
			}
		}
#endif

		private void tsbTextLogOpenFile_Click(object sender, EventArgs e)
		{
			OpenFileDialog ofDialog = new OpenFileDialog();
			if (DialogResult.OK == ofDialog.ShowDialog())
			{
				try
				{
					tsbTextLogListen.Checked = false;

					m_CurrentFilename = ofDialog.FileName;

					LoadLogEntriesFromFile(m_CurrentFilename);
				}
				catch (IOException ioEx)
				{
					MessageBox.Show("Unable to open the file. " + ioEx.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Warning);
				}
			}
		}

		private void btnpsSearch_Click(object sender, EventArgs e)
		{
			Cursor.Current = Cursors.WaitCursor;

			blvLogEntry.FindItemsWithText(cbxspFilter.Text, m_theSearchSettings);

			Cursor.Current = Cursors.Default;
		}

		// Save log file...
		private void tsbSaveLogFile_Click(object sender, EventArgs e)
		{
			// If clicked, give user chance to save the temporary file somewhere..
			SaveFileDialog sfDialog = new SaveFileDialog();

			sfDialog.FileName = "Untitled.txt";
			sfDialog.Filter = "Text Log Files (*.txt)|*.txt";
			sfDialog.FilterIndex = 0;

			if (DialogResult.OK == sfDialog.ShowDialog())
				File.Copy(m_CurrentFilename, sfDialog.FileName);
		}

		public void FormClosing()
		{
			if (m_LogServer != null)
			{
				m_LogServer.Kill(1000);
				m_LogServerThread.Interrupt();

				m_LogServer = null;
				m_LogServerThread = null;
			}
		}

		private void tsbTextLogListen_Click(object sender, EventArgs e)
		{
			RefreshListening();
		}

		public void RefreshListening()
		{
			int nPort;

			// Kill current socket... then reopen on new port
			if (null != m_LogServer)
			{
				// Cleaning up, clear everything out...
				ClearUI();

				m_LogServer.Kill(1000);
				m_LogServerThread.Interrupt();

				m_LogServer = null;
				m_LogServerThread = null;
			}


			if (tsedtTextLog.Text == "" || !Int32.TryParse(tsedtTextLog.Text, out nPort))
				MessageBox.Show("Invalid Port specified!  Please enter a valid port number to listen on...", "Socket Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
			else
			{
				m_LogServer = new MyTCPLogServer(nPort, this, LogMode.MODE_TEXT);
				SetStreamFilename(m_LogServer.tempFilename);
				m_LogServerThread = new Thread(new ThreadStart(m_LogServer.listen));
				m_LogServerThread.Start();

				// Depending on the parent tab... store our port #
				TabControl theParent = Parent as TabControl;

				switch (Parent.TabIndex)
				{
					case 0: // NXTLog
						{
							Properties.Settings.Default.NXTLogPort = tsedtTextLog.Text;
							break;
						}

					case 1: // FaceLog
						{
							Properties.Settings.Default.FaceLogPort = tsedtTextLog.Text;
							break;
						}

					case 2: // MotorLog
						{
							Properties.Settings.Default.MotorLogPort = tsedtTextLog.Text;
							break;
						}

					case 3: // EventLog
						{
							Properties.Settings.Default.EventLogPort = tsedtTextLog.Text;
							break;
						}
				}
			}
		}
	}


	public class LogSeverityFilter //: IModelFilter
    {
        // show all
        int selSeverity = (ushort)(LOGMSGSEVERITY.FATAL | LOGMSGSEVERITY.ERROR | LOGMSGSEVERITY.WARN | LOGMSGSEVERITY.INFO | LOGMSGSEVERITY.DEBUG | LOGMSGSEVERITY.TRACE);
        public bool Filter(object x)
        {
            int s=(ushort)((LogEntry)x).Severity;
            return (s & selSeverity) == s;
        }

        public void Flip(LOGMSGSEVERITY sev)
        {
            int s = (int)sev;
            if ((s & selSeverity) == s)
            {
                selSeverity = selSeverity & (~(int)s);
            }
            else
            {
                selSeverity = selSeverity | (int)s;
            }
        }

        public LogSeverityFilter copy()
        {
            LogSeverityFilter r= new LogSeverityFilter();
            r.selSeverity = this.selSeverity;
            return r;
        }

		public bool IsFiltered(int theSeverity)
		{
			return ((selSeverity & theSeverity) != 0) ? false : true;
		}
    }

	public enum LOGMSGSEVERITY
	{
		TRACE = 1,
		DEBUG = 2,
		INFO = 4,
		WARN = 8,
		ERROR = 16,
		FATAL = 32
	}

	public class LogEntry
	{
		public LogEntry() {}

		public string TimeStamp { get; set; }
		public LOGMSGSEVERITY Severity { get; set; }
		public string Category { get; set; }
		public string Message { get; set; }

		public LOGMSGSEVERITY StringToSeverity(string theString)
		{
			if (theString.ToUpper() == "TRACE")
				return LOGMSGSEVERITY.TRACE;
			else if (theString.ToUpper() == "DEBUG")
				return LOGMSGSEVERITY.DEBUG;
			else if (theString.ToUpper() == "INFO")
				return LOGMSGSEVERITY.INFO;
			else if (theString.ToUpper() == "WARN")
				return LOGMSGSEVERITY.WARN;
			else if (theString.ToUpper() == "ERROR")
				return LOGMSGSEVERITY.ERROR;
			else if (theString.ToUpper() == "FATAL")
				return LOGMSGSEVERITY.FATAL;
			else
				return LOGMSGSEVERITY.TRACE;
		}
	}
}