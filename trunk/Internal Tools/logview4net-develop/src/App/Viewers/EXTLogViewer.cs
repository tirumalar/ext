using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;
using System.Windows.Forms;
using logview4net.Controls;
using logview4net.Listeners;
using System.Runtime.Serialization;
using System.Xml;

namespace logview4net.Viewers
{
	[Serializable]
	public partial class EXTLogViewer : ViewerBase, IViewer, ISerializable
	{
        private readonly ILog _log = Logger.GetLogger("logview4net.Viewers.EXTLogViewer");

        /// <summary>
        ///     This is where all events are stored while the view is paused.
        /// </summary>
        protected Queue<LogEvent> PauseCache = new Queue<LogEvent>();

        /// <summary>
        ///     The textbox containing the log events to be viewed.
        /// </summary>
       // public AdvRichTextBox Txt;

        /// <summary>
        ///     The list of <see cref="Action" /> that are registered for the current <see cref="Session" />.
        /// </summary>
        protected List<Action> _actions = new List<Action>();

        private ToolStripMenuItem _addHideActionToolStripMenuItem;
        private ToolStripMenuItem _addHighlightActionToolStripMenuItem;
        private ToolStripMenuItem _addHighlightmatchActionToolStripMenuItem;

        private ToolStripMenuItem _addIgnoreActionToolStripMenuItem;
        private ToolStripMenuItem _addPopupActionToolStripMenuItem;
        private ToolStripMenuItem _blueToolStripMenuItem;
        private ToolStripMenuItem _blueToolStripMenuItem1;

        /// <summary>
        ///     The max amount of characters to have in the textbox
        /// </summary>
        protected int _bufferSize = 1024*100;

        /// <summary>
        ///     Whether or not to cache events when the viewer is pa
        /// </summary>
        protected bool _cacheOnPause = true;

        private ToolStripMenuItem _cyanToolStripMenuItem;

        private ToolStripMenuItem _cyanToolStripMenuItem1;
        private ToolStripMenuItem _findToolStripMenuItem;

        /// <summary>
        ///     Color of the text in the textbox.
        /// </summary>
        protected Color _foreColor;

        private int _lastProgressValue = -1;
        private ToolStripMenuItem _limeToolStripMenuItem;

        private ToolStripMenuItem _limeToolStripMenuItem1;
        private ToolStripMenuItem _magentaToolStripMenuItem;
        private ToolStripMenuItem _magentaToolStripMenuItem1;
        private ContextMenuStrip _mnuPop;
        private ToolStripMenuItem _orangeToolStripMenuItem;
        private ToolStripMenuItem _orangeToolStripMenuItem1;

        /// <summary>
        ///     Whether or not the viewer is paused
        /// </summary>
        protected bool _paused = false;

        private ProgressBar _progress;
        private ToolStripMenuItem _redToolStripMenuItem;
        private ToolStripMenuItem _redToolStripMenuItem1;
        private SaveFileDialog _saveFileDialog1;
        private ToolStripMenuItem _saveToFileToolStripMenuItem;
        private ToolStripMenuItem _showHiddenToolStripMenuItem;
        private ToolStripSeparator _toolStripSeparator1;

        /// <summary>
        ///     Whether or not the viewer does word wrapping
        /// </summary>
        protected bool _wordWrap = false;

        private ToolStripMenuItem _yellowToolStripMenuItem;
        private ToolStripMenuItem _yellowToolStripMenuItem1; // Scrolls to the right


        /// <summary>
        ///     Creates a new <see cref="EXTLogViewer" /> instance.
        /// </summary>
		public EXTLogViewer()
		{
			InitializeComponent();
		}


#region IViewer Members
        /// <summary>
        ///     List of actions for the listener.
        /// </summary>
        public List<Action> Actions
        {
            get { return _actions; }
        }

        /// <summary>
        ///     Whether or not to cache events when the viewer is paused
        /// </summary>
        public bool CacheOnPause
        {
            get { return _cacheOnPause; }
            set { _cacheOnPause = value; }
        }

		public event EventHandler HasHiddenMessagesEvent;

		/// <summary>
		///     Adds a list of events to this viewer
		/// </summary>
		public void AddEvent(string prefix, List<string> lines, ListenerBase listenerBase)
        {
            if (InvokeRequired)
            {
                var a = new AddEventsAsync(InvokedAddEvents);
                Invoke(a, new object[] {prefix, lines});
            }
            else
            {
                InvokedAddEvents(prefix, lines);

            }
        }

        /// <summary>
        ///     Adds an event to show
        /// </summary>
        /// <param name="message">The message to show.</param>
        /// <param name="listener">The listener that received the data</param>
        public void AddEvent(string message, ListenerBase listener)
        {
            string structure = listener.IsStructured ? listener.GetConfigValue("structured") : "n/a";
            var le = new LogEvent(message, _actions, true, structure);
            try
            {
                AddEvent(le);
            }
            catch (Exception ex)
            {
                throw ExceptionManager.HandleException(GetHashCode(), ex);
            }
        }

        /// <summary>
        ///     Indicates the existence of hidden messages.
        /// </summary>
        /// <returns></returns>
        public bool HasHiddenMessages()
        {
            bool hasMessages = false;
            foreach (Action a in Actions)
            {
                if (a.ActionType == ActionTypes.Hide && a.HideCache.Count > 0)
                {
                    hasMessages = true;
                    break;
                }
            }
            return hasMessages;
        }

        /// <summary>
        ///     Clears this instance.
        /// </summary>
        public void Clear()
        {
            //Txt.Text = "";
	//		throw new NotImplementedException();
        }


        /// <summary>
        ///     Gets or sets a value indicating whether this <see cref="EXTLogViewer" /> is paused.
        /// </summary>
        /// <value>
        ///     <c>true</c> if paused; otherwise, <c>false</c>.
        /// </value>
        public bool Paused
        {
            get { return _paused; }
            //set { SetPaused(value); }
			set { _paused = value; }
        }

        /// <summary>
        ///     Wether or not to wordwrap
        /// </summary>
        public bool WordWrap
        {
            get { return _wordWrap; }
            //set { SetWordWrap(value); }
			set {_wordWrap = value; }
        }

        /// <summary>
        ///     Whether or not to remove surrounding white space.
        /// </summary>
        public bool RemoveWhitespace { get; set; }

        /// <summary>
        ///     Whether or not to show the listener prefix
        /// </summary>
        public bool ShowListenerPrefix { get; set; }

        /// <summary>
        ///     Gets or sets the size of the buffer.
        /// </summary>
        /// <value></value>
        public int BufferSize
        {
            get { return _bufferSize; }
            set { _bufferSize = value; }
        }

        /// <summary>
        ///     The filename for saving events to disk
        /// </summary>
        public string LogFile { get; set; }

        /// <summary>
        ///     When to 'roll' the log file
        /// </summary>
        public string LogRolling { get; set; }

        /// <summary>
        ///     Wether or not to save events to disk.
        /// </summary>
        public bool LogToFile { get; set; }

		/// <summary>
		///     Gets the configuration.
		/// </summary>
		/// <returns></returns>
		public string GetConfiguration()
        {
            if (_log.Enabled) _log.Debug(GetHashCode(), "GetConfiguration");
            var x = new XmlDocument();
            //<viewer type = "Text" buffer="5000" forecolor="Lime" backcolor="Black" font="Courier New" fontsize="9" >
            XmlElement viewer = x.CreateElement("viewer");
            x.AppendChild(viewer);

            XmlAttribute a = x.CreateAttribute("type");
            a.InnerText = "Text";
            viewer.Attributes.Append(a);

            a = x.CreateAttribute("buffer");
            a.InnerText = _bufferSize.ToString();
            viewer.Attributes.Append(a);

            a = x.CreateAttribute("forecolor");
       //DMOOUT     a.InnerText = Txt.ForeColor.Name;
            viewer.Attributes.Append(a);

            a = x.CreateAttribute("backcolor");
       //DMOOUT     a.InnerText = Txt.BackColor.Name;
            viewer.Attributes.Append(a);

            a = x.CreateAttribute("font");
       //DMOOUT     a.InnerText = Txt.Font.Name;
            viewer.Attributes.Append(a);

            a = x.CreateAttribute("fontsize");
       //DMOOUT     a.InnerText = Txt.Font.Size.ToString(CultureInfo.InvariantCulture);
            viewer.Attributes.Append(a);

            a = x.CreateAttribute("cacheOnPause");
            a.InnerText = _cacheOnPause.ToString();
            viewer.Attributes.Append(a);

            a = x.CreateAttribute("logFileName");
            a.InnerText = LogFile;
            viewer.Attributes.Append(a);

            a = x.CreateAttribute("logRoll");
            a.InnerText = LogRolling;
            viewer.Attributes.Append(a);

            a = x.CreateAttribute("useLog");
            a.InnerText = LogToFile.ToString();
            viewer.Attributes.Append(a);

            foreach (Action action in _actions)
            {
                XmlNode actionNode = x.CreateElement("action");
                a = x.CreateAttribute("type");
                a.Value = action.ActionType.ToString();
                actionNode.Attributes.Append(a);

                a = x.CreateAttribute("pattern");
                a.Value = action.Pattern;
                actionNode.Attributes.Append(a);

                a = x.CreateAttribute("data");
                a.Value = action.Data;
                actionNode.Attributes.Append(a);

                a = CreateActionFormatAttributes(action, actionNode);

                viewer.AppendChild(actionNode);
            }

            return x.OuterXml;
        }

        private void AddEvent(LogEvent le)
        {
            if (InvokeRequired)
            {
                var a = new AddEventAsync(InvokedAddEvent);
                Invoke(a, new object[] {le, false});
            }
            else
            {
                InvokedAddEvent(le, false);
            }
        }

        /// <summary>
        ///     Adds the event.
        /// </summary>
        private void InvokedAddEvents(string prefix, List<string> lines)
        {
            if (! _paused && lines.Count > 100)
            {
			// Display loading or percent bar here...
			#if FALSE
                _progress.Visible = true;

                Refresh();
                Application.DoEvents();

        //DMOOUT        Txt.BeginUpdate();
			#endif
            }

            int pos = 0;

            foreach (var line in lines)
            {
                var le = new LogEvent(prefix + line, _actions, true, "n/a");
                pos++;
                SetProgress(pos, lines.Count);
                try
                {
                    InvokedAddEvent(le, false);
                }
                catch (Exception ex)
                {
                    throw ExceptionManager.HandleException(GetHashCode(), ex);
                }
            }

            if (! _paused && lines.Count > 100)
            {
#if FALSE // finish up percent bar or loading dialog...
           //DMOOUT     Txt.EndUpdate();
                Refresh();
                Application.DoEvents();
			#endif
            }
            ResetProgress();
        }

        private void ResetProgress()
        {
            _progress.Visible = false;
            _lastProgressValue = -1;
            _progress.Value = 0;
        }

        private void SetProgress(int current, int max)
        {
            if (! _paused && max > 100)
            {
                if (!_progress.Visible)
                {
                    _progress.Visible = true;
                }
                float fvalue = current/(float) max;
                var value = (int) (fvalue*_progress.Maximum);
                if (value != _lastProgressValue)
                {
                    if (value > _progress.Maximum) value = _progress.Maximum;
                    _progress.Value = value;
                    Text = value.ToString();
                    Refresh();
                    Application.DoEvents();
                    _lastProgressValue = value;
                }
            }
        }

        /// <summary>
        ///     Adds the event.
        /// </summary>
        private void InvokedAddEvent(LogEvent le, bool forceShow)
        {
            if (RemoveWhitespace)
            {
                le.Message = le.Message.Trim();
            }

            if (CacheIfPuased(le)) return;

            try
            {
                var reason = ViewerUtils.IgnoreReasons.DoNotIgnore;
                var ignoreMessage = false;
                if (!forceShow)
                {
                    ignoreMessage = ViewerUtils.IgnoreEvent(le, out reason);
                }

                if (AddIgnoreMessage(ignoreMessage, reason)) return;

                
                if (le.Actions.Count > 0)
                {
                    ExecuteActions(le);
                }
                else
                {
					// This is it, this is where we start parsing... and building up
					// our structures and loading up the UI...
		//			Analyze the line of text and create our listview item

					
                }

          //DMOOUT      EnforceBufferSize();

            //DMOOUT    RaiseHasHiddenMessages(HasHiddenMessages());
            }
            catch (ObjectDisposedException ex)
            {
                _log.Warn(GetHashCode(), ex.ToString(), ex);
            }
            catch (Exception ex)
            {
                throw ExceptionManager.HandleException(GetHashCode(), ex);
            }
        }

        private void ExecuteActions(LogEvent le)
        {
#if FALSE
            Txt.SelectionStart = int.MaxValue - 1;
            int selectionStart = Txt.SelectionStart;
            Txt.SelectedText = le.Message + Environment.NewLine;
            //txt.AppendText(le.Message + Environment.NewLine);
            
            foreach (Action action in le.Actions)
            {
                switch (action.ActionType)
                {
                    case ActionTypes.Highlight:
                        ExecHighlight(action, le, selectionStart);
                        break;
                    case ActionTypes.HighlightMatch:
                        ExecHighlightMatch(action, le, selectionStart);
                        break;
                    case ActionTypes.PopUp:
                        ViewerUtils.ExecuteNonViewerAction(action, le.Message);
                        break;
                    case ActionTypes.PlaySound:
                        ViewerUtils.ExecuteNonViewerAction(action, le.Message);
                        break;
                    case ActionTypes.Execute:
                        ViewerUtils.ExecuteNonViewerAction(action, le.Message);
                        break;
                }
            }
#endif
        }

        private bool AddIgnoreMessage(bool ignoreMessage, ViewerUtils.IgnoreReasons reason)
        {
#if FALSE

            if (ignoreMessage)
            {
                switch (reason)
                {
                    case ViewerUtils.IgnoreReasons.StartedBlock:
                        Txt.AppendText("----- STARTED IGNORING -----" + Environment.NewLine);
                        break;
                    case ViewerUtils.IgnoreReasons.EndedBlock:
                        Txt.AppendText("----- ENDED IGNORING -----" + Environment.NewLine);
                        break;
                    case ViewerUtils.IgnoreReasons.Hide:
                        //The message was cached in ViewerUtils.IgnoreEvent
                        RaiseHasHiddenMessages(HasHiddenMessages());
                        break;
                }
                return true;
            }
#endif
            return false;
        }

        private bool CacheIfPuased(LogEvent le)
        {
            if (_paused)
            {
                if (_cacheOnPause)
                {
                    PauseCache.Enqueue(le);
                }
                return true;
            }
            return false;
        }

        private void ExecHighlightMatch(Action action, LogEvent le, int selectionStart)
        {
#if FALSE

            int textLength = Txt.SelectionStart;
            int l = le.Message.Length;
            int curPos = 0;
            int startPos = selectionStart;
            int foundPos = 0;
            _foreColor = Txt.ForeColor;
            Font f = Txt.Font;

            while (foundPos >= 0)
            {
                foundPos = le.Message.IndexOf(action.Pattern, curPos, StringComparison.Ordinal);

                if (foundPos == -1)
                {
                    break;
                }

                Txt.Select(startPos + foundPos, action.Pattern.Length);
                Txt.SelectionColor = action.Color;
                Txt.SelectionFont = action.Font;

                curPos = foundPos + action.Pattern.Length;
            }

            Txt.Select(textLength, 0);
            Txt.SelectionColor = _foreColor;
            Txt.SelectionFont = f;
#endif
        }

        private void ExecHighlight(Action action, LogEvent le, int selectionStart)
        {
#if FALSE
            _foreColor = Txt.ForeColor;
            Font f = Txt.Font;

            int l = le.Message.Length + 1;
            int pos = selectionStart - l;

            Txt.Select(selectionStart, l);

            Txt.SelectionColor = action.Color;
            Txt.SelectionFont = action.Font;

            Txt.SelectionStart = int.MaxValue;
            Txt.SelectionColor = _foreColor;
            Txt.SelectionFont = f;
#endif
        }


        /// <summary>
        ///     Adds an action to the list in this viewer.
        /// </summary>
        /// <param name="action">Action.</param>
        internal void AddAction(Action action)
        {
            if (action == null)
            {
                _log.Warn(GetHashCode(), "Got a null action. Error in configuration?");
                return;
            }
            bool added = false;

            _log.Info(GetHashCode(), "Adding Action: " + action.ActionType + " " + action.Pattern);

            for (int i = 0; i < _actions.Count; i++)
            {
                if ((String.Compare(action.Pattern, _actions[i].Pattern, StringComparison.Ordinal) <= 0) &&
                    (action.ActionType <= _actions[i].ActionType))
                {
                    _actions.Insert(i, action);
                    added = true;
                    break;
                }
            }
            if (!added)
            {
                _actions.Add(action);
            }
        }

        /// <summary>
        ///     removes an action from the list in this viewer.
        /// </summary>
        /// <param name="action">Action.</param>
        internal void RemoveAction(Action action)
        {
            _log.Info(GetHashCode(), "Removed Action: " + action.ActionType + " " + action.Pattern);
            _actions.Remove(action);
        }

        private static XmlAttribute CreateActionFormatAttributes(Action action, XmlNode actionNode)
        {
            Debug.Assert(actionNode.Attributes != null, "actionNode.Attributes != null");

            XmlDocument x = actionNode.OwnerDocument;

            Debug.Assert(x != null, "x != null");
            XmlAttribute a = x.CreateAttribute("color");
            a.Value = action.Color.Name;

            actionNode.Attributes.Append(a);

            a = x.CreateAttribute("font-name");
            a.Value = action.Font.Name;
            actionNode.Attributes.Append(a);

            a = x.CreateAttribute("font-size");
            a.Value = action.Font.Size.ToString();
            actionNode.Attributes.Append(a);

            a = x.CreateAttribute("font-style");
            a.Value = action.Font.Style.ToString();
            actionNode.Attributes.Append(a);

            return a;
        }

		public void ShowAllHidden()
		{
			throw new NotImplementedException();
		}

		public void GetObjectData(SerializationInfo info, StreamingContext context)
		{
			throw new NotImplementedException();
		}

#endregion

        private delegate void AddEventAsync(LogEvent le, bool forceShow);

        private delegate void AddEventsAsync(string prefix, List<string> lines);

	}
}
