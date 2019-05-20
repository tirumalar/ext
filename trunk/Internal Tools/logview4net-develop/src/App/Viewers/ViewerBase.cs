/*
 * This file is part of logview4net (logview4net.sourceforge.net)
 * Copyright 2008 Johan Idstam
 * 
 * 
 * This source code is released under the Artistic License 2.0.
 */

using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;
using System.Xml;
using logview4net.Controls;
using System.Reflection;
using System.Collections.Generic;

namespace logview4net.Viewers
{
	/// <summary>
	/// Summary description for ConfiguratorBase.
	/// </summary>
	public class ViewerBase : UserControl
	{
	    /// <summary>
        ///     The list of <see cref="Action" /> that are registered for the current <see cref="Session" />.
        /// </summary>
        protected List<Action> _actions = new List<Action>();

		#region Constructors

		/// <summary>
		/// Creates a new <see cref="ViewerBase"/> instance.
		/// </summary>
		public ViewerBase()
		{

		}
		#endregion

        /// <summary>
        ///     List of actions for the listener.
        /// </summary>
       public List<Action> Actions
        {
            get { return _actions; }
        }


#if FALSE
		#region IViewerConfigurator members

		/// <summary>
		/// Gets the viewer for this configurator.
		/// </summary>
		/// <value></value>
		public IViewer Viewer
		{
			get { return _viewer; }
			set
			{
				_viewer = value;
			}
		}

		#endregion
#endif
	}
}
