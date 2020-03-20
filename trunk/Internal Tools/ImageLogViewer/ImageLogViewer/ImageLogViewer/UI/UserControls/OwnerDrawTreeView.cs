using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Drawing;


namespace EyEnterprise.UI.UserControls
{
	public class OwnerDrawTreeView : TreeView
	{
		bool isLeftMouseDown = false;
		bool isRightMouseDown = false;
		public OwnerDrawTreeView()
		{
			DrawMode = TreeViewDrawMode.OwnerDrawText;
		}

		protected override void OnMouseDown(MouseEventArgs e)
		{
			TrackMouseButtons(e);
			base.OnMouseDown(e);
		}

		protected override void OnMouseUp(MouseEventArgs e)
		{
			TrackMouseButtons(e);
			base.OnMouseUp(e);
		}
		protected override void OnMouseMove(MouseEventArgs e)
		{
			TrackMouseButtons(e);
			base.OnMouseMove(e);
		}

		private void TrackMouseButtons(MouseEventArgs e)
		{
			isLeftMouseDown = e.Button == MouseButtons.Left;
			isRightMouseDown = e.Button == MouseButtons.Right;
		}

		protected override void OnDrawNode(DrawTreeNodeEventArgs e)
		{
			// don't call the base or it will goof up your display!
			// capture the selected/focused states
			bool isFocused = (e.State & TreeNodeStates.Focused) != 0;
			bool isSelected = (e.State & TreeNodeStates.Selected) != 0;
			// set up default colors.

			Color color = SystemColors.WindowText;
			Color backColor = BackColor;

			if (isFocused && isRightMouseDown)
			{
				// right clicking on a 
				color = SystemColors.HighlightText;
				backColor = SystemColors.Highlight;
			}
			else if (isSelected && !isRightMouseDown)
			{
				// if the node is selected and we're not right clicking on another node.
				color = SystemColors.HighlightText;
				backColor = SystemColors.Highlight;
			}

			using (Brush sb = new SolidBrush(backColor))
				e.Graphics.FillRectangle(sb, e.Bounds);

			TextFormatFlags flags = TextFormatFlags.Left | TextFormatFlags.SingleLine |
			   TextFormatFlags.VerticalCenter | TextFormatFlags.EndEllipsis;

			TextRenderer.DrawText(e.Graphics, e.Node.Text, Font, e.Bounds, color, backColor, flags);
		}
	}
}

#if null 

private void TreeViewControl_DrawNode(Object sender, DrawTreeNodeEventArgs e)
{
    //What might seem like strange positioning/offset is to ensure that our custom drawing falls in
    //  line with where the base drawing would appear.  Otherwise, click handlers (hit tests) fail 
    //  to register properly if our custom-drawn checkbox doesn't fall within the expected coordinates.

    Int32 boxSize = 16;
    Int32 offset = e.Node.Parent == null ? 3 : 21;
    Rectangle bounds = new Rectangle(new Point(e.Bounds.X + offset, e.Bounds.Y + 1), new Size(boxSize, boxSize));
    ControlPaint.DrawCheckBox(e.Graphics, bounds, e.Node.Checked ? ButtonState.Checked : ButtonState.Normal);
    if (e.Node.Parent != null)
    {
        Color c = Color.Black;
        String typeName = e.Node.Name.Remove(0, 4);
        Object o = Enum.Parse(typeof(CalendarDataProvider.CalendarDataItemType), typeName);
        if (o != null && (o is CalendarDataProvider.CalendarDataItemType))
            c = CalendarDataProvider.GetItemTypeColor((CalendarDataProvider.CalendarDataItemType)o);
        bounds = new Rectangle(new Point(bounds.X + boxSize + 2, e.Bounds.Y + 1), new Size(13, 13));
        using (SolidBrush b = new SolidBrush(c))
            e.Graphics.FillRectangle(b, bounds);
        e.Graphics.DrawRectangle(Pens.Black, bounds);
        e.Graphics.DrawLine(Pens.Black, new Point(bounds.X + 1, bounds.Bottom + 1), new Point(bounds.Right + 1, bounds.Bottom + 1));
        e.Graphics.DrawLine(Pens.Black, new Point(bounds.Right + 1, bounds.Y + 1), new Point(bounds.Right + 1, bounds.Bottom + 1));
    }
    Font font = new Font("Microsoft Sans Serif", 9f, e.Node.Parent == null ? FontStyle.Bold : FontStyle.Regular);
    bounds = new Rectangle(new Point(bounds.X + boxSize + 2, e.Bounds.Y), new Size(e.Bounds.Width - offset - 2, boxSize));
    e.Graphics.DrawString(e.Node.Text, font, Brushes.Black, bounds);

	#endif
