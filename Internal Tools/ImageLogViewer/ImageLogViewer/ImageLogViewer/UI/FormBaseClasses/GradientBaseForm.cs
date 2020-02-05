using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Imaging;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Drawing.Drawing2D;

namespace ImageLogViewer.UI.FormBaseClasses
{
    public partial class GradientBaseForm : Form
    {
		private Color m_clrTopColor = Color.Blue;
		private Color m_clrBottomColor = Color.FromArgb(255, 238, 238, 238);


        public GradientBaseForm()
        {
            InitializeComponent();
        }

		protected override void OnPaintBackground(PaintEventArgs e)
		{
            if (ClientRectangle.Width == 0 || ClientRectangle.Height == 0)
                base.OnPaintBackground(e);
            else
            {
                using (Brush b = new LinearGradientBrush(ClientRectangle, m_clrTopColor, m_clrBottomColor, LinearGradientMode.Vertical))
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
