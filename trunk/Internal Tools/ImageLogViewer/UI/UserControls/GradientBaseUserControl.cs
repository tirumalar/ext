using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.ComponentModel;


namespace ImageLogViewer.UI.UserControls
{
	public partial class GradientBaseUserControl : UserControl
	{
		private Color m_clrTopColor = Color.White;
		private Color m_clrBottomColor = Color.FromArgb(255, 238, 238, 238);

		public GradientBaseUserControl() {}

		protected override void OnPaintBackground(PaintEventArgs e)
		{
			if (ClientRectangle.Height != 0 && ClientRectangle.Width != 0)
			{
				using (Brush b = new LinearGradientBrush(ClientRectangle, TopColor, BottomColor, LinearGradientMode.Vertical))
					e.Graphics.FillRectangle(b, ClientRectangle);
			}
		}

#region Accessors
		[EditorBrowsable(EditorBrowsableState.Always), Browsable(true),
		DesignerSerializationVisibility(DesignerSerializationVisibility.Visible),
		Bindable(true)]
		public Color TopColor
		{
			get { return m_clrTopColor; }
			set
			{
				m_clrTopColor = value;
				this.Invalidate(); // Tell the Form to repaint itself
			}
		}

		[EditorBrowsable(EditorBrowsableState.Always), Browsable(true),
		DesignerSerializationVisibility(DesignerSerializationVisibility.Visible),
		Bindable(true)]
		public Color BottomColor
		{
			get { return m_clrBottomColor; }
			set
			{
				m_clrBottomColor = value;
				this.Invalidate(); // Tell the Form to repaint itself
			}
		}
#endregion
	}
}
