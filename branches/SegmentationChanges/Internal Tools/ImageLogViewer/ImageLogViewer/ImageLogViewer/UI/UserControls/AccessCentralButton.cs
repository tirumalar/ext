using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace EyEnterprise.UI.UserControls
{
    public partial class AccessCentralButton : UserControl
    {
        [EditorBrowsable(EditorBrowsableState.Always), Browsable(true),
        DesignerSerializationVisibility(DesignerSerializationVisibility.Visible),
        Bindable(true)]
        public Color ButtonColor { get; set; }

        [EditorBrowsable(EditorBrowsableState.Always), Browsable(true),
        DesignerSerializationVisibility(DesignerSerializationVisibility.Visible),
        Bindable(true)]
        public string ButtonText { get; set; }

        public AccessCentralButton()
        {
            InitializeComponent();
        }

        private void AccessCentralButton_Load(object sender, EventArgs e)
        {
            // Set the button's color
            btnAccessCentral.ForeColor = ButtonColor;
            lblAccessCentral.Text = ButtonText;
            PerformLayout(); // To re-layout the text control for centering
        }
    }
}
