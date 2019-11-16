using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;




namespace ACD2HexConvertor
{

    public partial class ACD2HEX : Form
    {
        public ACD2HEX()
        {
            InitializeComponent();

        }

        private void Form1_Load(object sender, EventArgs e)
        {


        }

        private void openFileDialog1_FileOk(object sender, CancelEventArgs e)
        {
            FileNameTB.Text = FileDialog1.FileName;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            FileDialog1.ShowDialog();
        }

        private void button2_Click(object sender, EventArgs e)
        {
            folderBrowserDialog1.ShowDialog();
            FolderName.Text = folderBrowserDialog1.SelectedPath;

        }




        private void folderBrowserDialog1_HelpRequest(object sender, EventArgs e)
        {

        }

        private void button3_Click(object sender, EventArgs e)
        {
            string path = FileNameTB.Text;
            string path2 = FolderName.Text;
            if (!File.Exists(path))
            {
                MessageBox.Show("Error: Source file does not exist");
            }
            else if (!path.Contains(".cyacd"))
            {
                MessageBox.Show("Error: Source file should be a .cyacd file");
            }
            else 
            {
                if (path2.Equals(""))
                {
                    MessageBox.Show("Select a destination folder for StringImage.h file");
                }
                else
                {

                    path2 = FolderName.Text + "\\" + "StringImage.h";


                    // Delete the file if it exists.
                    int LineNo;
                    string No_Of_Lines, size, row;

                    if (File.Exists(path2))
                    {
                        File.Delete(path2);
                    }


                    //Create the file.
                    using (FileStream fs2 = File.Create(path2))
                    {
                        StreamWriter w = new StreamWriter(fs2);

                        FileStream fs1 = File.OpenRead(path);
                     
                        /* Open the cyacd file and calculate the rowData length for the rowData array */
                        StreamReader readCyacd = new StreamReader(fs1);
                        row = readCyacd.ReadLine();
                        row = readCyacd.ReadLine();
                        size = row.Substring(7, 4);


                        w.WriteLine("/*******************************************************************************");
                        w.WriteLine("* File Name: StringImage.h");
                        w.WriteLine("* Version 1.0");
                        w.WriteLine("*");
                        w.WriteLine("* Description:");
                        w.WriteLine("* This file is created to store the bootloadable code as a stringImage");
                        w.WriteLine("* LINE_CNT is the number of lines in cyacd file");
                        w.WriteLine("*******************************************************************************/");
                        w.WriteLine("");

                        /* Open the .acd file for reading each line and parsing it to hex array */
                        using (FileStream fs3 = File.OpenRead(path))
                        {
                            StreamReader r= new StreamReader(fs3);
                            
                            /* size of the line */
                            LineNo = 1;
                            
                            /* Read till the end of the line*/
                            while ((row = r.ReadLine()) != null)
                            {
                                if (row[0] == ':')
                                {
                                    LineNo++;
                                    w.WriteLine(",");
                                    w.Write("\"");
                                    w.Write(row);
                                    w.Write("\"");
                                }
                                else
                                {                                    
                                    w.Write("const char *stringImage[] = {\"");
                                    w.Write("{0}\"", row);
                                }
                            }
                            w.WriteLine("");
                            w.WriteLine("};");
                        }
                        No_Of_Lines = LineNo.ToString();
                        w.WriteLine("\r\n/* No. of rows to be programmed */");
                        w.WriteLine("unsigned int LINE_CNT = {0};", No_Of_Lines);
                        w.Close();

                        fs2.Close();

                        MessageBox.Show(" Include the header file \n\r\n\r" + path2 + "\n\r\n\rin your project");
                    }
                    Environment.Exit(1);
                }
            }
        }
        
    }


}