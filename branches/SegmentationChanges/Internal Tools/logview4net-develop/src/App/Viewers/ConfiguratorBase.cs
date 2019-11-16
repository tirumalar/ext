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

namespace logview4net.Viewers
{
	/// <summary>
	/// Summary description for ConfiguratorBase.
	/// </summary>
	public abstract class ConfiguratorBase : UserControl
	{
		public ViewerBase _viewer;

		#region Constructors

		/// <summary>
		/// Creates a new <see cref="ConfiguratorBase"/> instance.
		/// </summary>
		public ConfiguratorBase()
		{

		}
		#endregion

		#region IViewerConfigurator members

		/// <summary>
		/// Gets the viewer for this configurator.
		/// </summary>
		/// <value></value>
		public virtual ViewerBase Viewer {get; set;}


		/// <summary>
		/// Gets or sets the configuration data for this configurator.
		/// </summary>
		/// <value></value>
		public virtual string Configuration { get; set; }

				/// <summary>
		/// Validates the settings in the configurator.
		/// </summary>
		/// <returns>Trus if everything seems OK, False if there is a configuration error.</returns>
		public abstract bool ValidateSettings();


		#endregion
	}
}
