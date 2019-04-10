using System;
using System.Runtime.InteropServices;

namespace UARTBootloaderHost
{
    /* These constants are defined in cybtldr_utils.h provided under 
     * "...\PSoC Creator\3.1\PSoC Creator\cybootloaderutils"
     */

    /// <summary>
    /// These are the different return codes returned by the Bootlaoder Host.
    /// </summary>
    public enum ReturnCodes
    {
        /// <summary>
        /// Completed successfully
        /// </summary>
        CYRET_SUCCESS = 0x00,
        /// <summary>
        /// File is not accessable
        /// </summary>
        CYRET_ERR_FILE = 0x01,
        /// <summary>
        /// Reached the end of the file
        /// </summary>
        CYRET_ERR_EOF = 0x02,
        /// <summary>
        /// The amount of data available is outside the expected range
        /// </summary>
        CYRET_ERR_LENGTH = 0x03,
        /// <summary>
        /// The data is not of the proper form
        /// </summary>
        CYRET_ERR_DATA = 0x04,
        /// <summary>
        /// The command is not recognized
        /// </summary>
        CYRET_ERR_CMD = 0x05,
        /// <summary>
        /// The expected device does not match the detected device
        /// </summary>
        CYRET_ERR_DEVICE = 0x06,
        /// <summary>
        /// The bootloader version detected is not supported
        /// </summary>
        CYRET_ERR_VERSION = 0x07,
        /// <summary>
        /// The checksum does not match the expected value
        /// </summary>
        CYRET_ERR_CHECKSUM = 0x08,
        /// <summary>
        /// The flash array is not valid
        /// </summary>
        CYRET_ERR_ARRAY = 0x09,
        /// <summary>
        /// The flash row is not valid
        /// </summary>
        CYRET_ERR_ROW = 0x0A,
        /// <summary>
        /// The bootloader is not ready to process data
        /// </summary>
        CYRET_ERR_BTLDR = 0x0B,
        /// <summary>
        /// The application is currently marked as active
        /// </summary>
        CYRET_ERR_ACTIVE = 0x0C,
        /// <summary>
        /// An unknown error occured
        /// </summary>
        CYRET_ERR_UNKNOWN = 0x0F,
        /// <summary>
        /// The operation was aborted
        /// </summary>
        CYRET_ABORT = 0xFF,

        /// <summary>
        /// The communications object reported an error
        /// </summary>
        CYRET_ERR_COMM_MASK = 0x2000,
        /// <summary>
        /// The bootloader reported an error
        /// </summary>
        CYRET_ERR_BTLDR_MASK = 0x4000,
    }

    /// <summary>
    /// These are the different status codes returned by the Bootlaoder.
    /// </summary>
    public enum StatusCodes
    {
        /// <summary>
        /// Completed successfully
        /// </summary>
        SUCCESS = 0x00,
        /// <summary>
        /// The provided key does not match the expected value
        /// </summary>
        ERR_KEY = 0x01,
        /// <summary>
        /// The verification of flash failed
        /// </summary>
        ERR_VERIFY = 0x02,
        /// <summary>
        /// The amount of data available is outside the expected range
        /// </summary>
        ERR_LENGTH = 0x03,
        /// <summary>
        /// The data is not of the proper form
        /// </summary>
        ERR_DATA = 0x04,
        /// <summary>
        /// The command is not recognized
        /// </summary>
        ERR_CMD = 0x05,
        /// <summary>
        /// The expected device does not match the detected device
        /// </summary>
        ERR_DEVICE = 0x06,
        /// <summary>
        /// The bootloader version detected is not supported
        /// </summary>
        ERR_VERSION = 0x07,
        /// <summary>
        /// The checksum does not match the expected value
        /// </summary>
        ERR_CHECKSUM = 0x08,
        /// <summary>
        /// The flash array is not valid
        /// </summary>
        ERR_ARRAY = 0x09,
        /// <summary>
        /// The flash row is not valid
        /// </summary>
        ERR_ROW = 0x0A,
        /// <summary>
        /// The flash row is protected and can not be programmed
        /// </summary>
        ERR_PROTECT = 0x0B,
        /// <summary>
        /// The application is not valid and cannot be set as active
        /// </summary>
        ERR_APP = 0x0C,
        /// <summary>
        /// The application is currently marked as active
        /// </summary>
        ERR_ACTIVE = 0x0D,
        /// <summary>
        /// An unknown error occured
        /// </summary>
        ERR_UNKNOWN = 0x0F,
    }
}