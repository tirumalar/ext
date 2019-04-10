using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using UARTBootloaderHost;

namespace UARTBootloaderHost
{
    class Bootload_Utils
    {
        /// <summary>
        /// Delegate used as a callback from native code for opening a communications connection
        /// </summary>
        /// <returns>Integer representing success == 0 or failure </returns>
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int OpenConnection_UART();
        /// <summary>
        /// Delegate used as a callback from native code for closing a communications connection
        /// </summary>
        /// <returns>Integer representing success == 0 or failure </returns>
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int CloseConnection_UART();
        /// <summary>
        /// Delegate used as a callback from native code for reading data from a communications connection
        /// </summary>
        /// <param name="buffer">The buffer to store the read data in</param>
        /// <param name="size">The number of bytes of data to read</param>
        /// <returns>Integer representing success == 0 or failure </returns>
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int ReadData_UART(IntPtr buffer, int size);
        /// <summary>
        /// Delegate used as a callback from native code for writing data over a communications connection
        /// </summary>
        /// <param name="buffer">The buffer containing data to write</param>
        /// <param name="size">The number of bytes to write</param>
        /// <returns>Integer representing success == 0 or failure </returns>
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int WriteData_UART(IntPtr buffer, int size);
        /// <summary>
        /// Delegate used as a callback from native code for notifying that a row is complete
        /// </summary>
        /// <param name="arrayID">The array ID that was accessed</param>
        /// <param name="rowNum">The row number within the array that was accessed</param>
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void CyBtldr_ProgressUpdate(byte arrayID, ushort rowNum);

        /// <summary>
        /// Structure used to pass communication data down to the unmanged native C code
        /// that handles the bootloading operations.
        /// </summary>
        [StructLayout(LayoutKind.Sequential)]
        public struct CyBtldr_CommunicationsData
        {
            /// <summary>
            /// Function used to open the communications connection
            /// </summary>
            public OpenConnection_UART OpenConnection;
            /// <summary>
            /// Function used to close the communications connection
            /// </summary>
            public CloseConnection_UART CloseConnection;
            /// <summary>
            /// Function used to read data over the communications connection
            /// </summary>
            public ReadData_UART ReadData;
            /// <summary>
            /// Function used to write data over the communications connection
            /// </summary>
            public WriteData_UART WriteData;
            /// <summary>
            /// Value used to specify the maximum number of bytes that can be trasfered at a time
            /// </summary>
            public uint MaxTransferSize;
        };


        [DllImport("BootLoad_Utils.dll", CharSet = CharSet.Ansi, BestFitMapping = false, ThrowOnUnmappableChar = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int CyBtldr_Program([MarshalAs(UnmanagedType.LPStr)] string file, ref CyBtldr_CommunicationsData comm, CyBtldr_ProgressUpdate update);

        /// <summary>
        /// Function used to start the Bootloader Erase oparation
        /// </summary>
        /// <param name="file">The *.cyacd file containing the bootloader data</param>
        /// <param name="comm">The communications object for sending data to the bootloader</param>
        /// <param name="update">Callback function to notify that a row was finished</param>
        /// <returns>Integer representing success == 0 or failure </returns>
        [DllImport("BootLoad_Utils.dll", CharSet = CharSet.Ansi, BestFitMapping = false, ThrowOnUnmappableChar = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int CyBtldr_Erase([MarshalAs(UnmanagedType.LPStr)] string file, ref CyBtldr_CommunicationsData comm, CyBtldr_ProgressUpdate update);

        /// <summary>
        /// Function used to start the Bootloader Verify operation
        /// </summary>
        /// <param name="file">The *.cyacd file containing the bootloader data</param>
        /// <param name="comm">The communications object for sending data to the bootloader</param>
        /// <param name="update">Callback function to notify that a row was finished</param>
        /// <returns>Integer representing success == 0 or failure </returns>
        [DllImport("BootLoad_Utils.dll", CharSet = CharSet.Ansi, BestFitMapping = false, ThrowOnUnmappableChar = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int CyBtldr_Verify([MarshalAs(UnmanagedType.LPStr)] string file, ref CyBtldr_CommunicationsData comm, CyBtldr_ProgressUpdate update);

        /// <summary>
        /// Aborts the current bootloader host operation
        /// </summary>
        /// <returns>Integer representing success == 0 or failure </returns>
        [DllImport("BootLoad_Utils.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int CyBtldr_Abort();
    }

}
