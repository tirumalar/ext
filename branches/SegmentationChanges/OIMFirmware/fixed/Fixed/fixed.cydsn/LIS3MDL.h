#ifndef LIS3MDL_h
#define LIS3MDL_h

#define uint8 unsigned char
#define uint16 unsigned short
#define int16 short

// Return values 
typedef enum
{
	MAG_SUCCESS,
	MAG_HW_ERROR,
	MAF_NOT_SUPPORTED,
	MAG_GENERIC_ERROR,
	MAG_OUT_OF_BOUNDS,
	MAG_ALL_ONES_WARNING,
	//...
} magStatus_t;    
    
struct vector
{
    float x, y, z;
    float T;
};

// register addresses
enum regAddr
{
    WHO_AM_I    = 0x0F,
    CTRL_REG1   = 0x20,
    CTRL_REG2   = 0x21,
    CTRL_REG3   = 0x22,
    CTRL_REG4   = 0x23,
    CTRL_REG5   = 0x24,
    STATUS_REG  = 0x27,
    OUT_X_L     = 0x28,
    OUT_X_H     = 0x29,
    OUT_Y_L     = 0x2A,
    OUT_Y_H     = 0x2B,
    OUT_Z_L     = 0x2C,
    OUT_Z_H     = 0x2D,
    TEMP_OUT_L  = 0x2E,
    TEMP_OUT_H  = 0x2F,
    INT_CFG     = 0x30,
    INT_SRC     = 0x31,
    INT_THS_L   = 0x32,
    INT_THS_H   = 0x33,
};

// Defines ////////////////////////////////////////////////////////////////
#define LIS3MDL_WHO_ID  0x3D

#define SINGLE_WRITE_MASK 0x3F
#define READ_MASK 0x80

//Magnetometer Registers
#define LIS3MDL_ADDRESS      0x1C
#define LIS3MDL_WHO_AM_I     0x0F  // should return 0x3D
#define LIS3MDL_CTRL_REG1    0x20
#define LIS3MDL_CTRL_REG2    0x21
#define LIS3MDL_CTRL_REG3    0x22
#define LIS3MDL_CTRL_REG4    0x23
#define LIS3MDL_CTRL_REG5    0x24
#define LIS3MDL_STATUS_REG   0x27   
#define LIS3MDL_OUT_X_L	     0x28  // data
#define LIS3MDL_OUT_X_H	     0x29
#define LIS3MDL_OUT_Y_L	     0x2A
#define LIS3MDL_OUT_Y_H	     0x2B
#define LIS3MDL_OUT_Z_L	     0x2C
#define LIS3MDL_OUT_Z_H	     0x2D
#define LIS3MDL_TEMP_OUT_L   0x2E
#define LIS3MDL_TEMP_OUT_H   0x2F  // data
#define LIS3MDL_INT_CFG	     0x30
#define LIS3MDL_INT_SRC	     0x31
#define LIS3MDL_INT_THS_L    0x32
#define LIS3MDL_INT_THS_H    0x33

void enableDefault(void);
void initLIS3MDL(void);
magStatus_t readRegRegion(uint8 *outputPointer , uint8 offset, uint8 length);
magStatus_t writeReg(uint8 offset, uint8 dataToWrite);
magStatus_t readReg(uint8* outputPointer, uint8 offset);
magStatus_t readRegInt16( int16* outputPointer, uint8 offset );
void readMagData(struct vector *m);
int16 readMagTempData(void);
float vector_normalize(struct vector *a);
void vector_cross(struct vector *a, struct vector *b, struct vector *out);
void vector_dot(struct vector *a, struct vector *b, struct vector *out);
float getMres(uint8 Mscale);
void LIS3MDLInit(void);

//This struct holds the settings the driver uses to do calculations
struct MagSensorSettings 
{
	// Magnetometer Settings
    uint8 TempSensorEnable;
    uint8 FastOdrEnable;
    uint8 SelfTestEnable;
    uint8 RebootMemory;
    uint8 SoftwareReset;
    uint8 LowPowerEnable;
    uint8 SpiInterfaceMode;
    uint8 OperatingMode;
    uint8 EndianData;
    uint8 FastDataEnable;
    uint8 BlockDataUpdate;
    uint8 XInterruptEnable;
    uint8 YInterruptEnable;
    uint8 ZInterruptEnable;
    uint8 InterruptPolarity;
    uint8 InterruptLatch;
    uint8 InterruptOnPin;
    uint8 FullScale;
    uint8 XYPerformanceMode;
    uint8 ZPerformanceMode;
    uint8 OdrDivisor;
    uint8 OutputDataRate;
};

struct MagSensorStatus
{
    uint8 XYZDataOverrun;
    uint8 XDataOverrun;
    uint8 YDataOverrun;
    uint8 ZDataOverrun;
    uint8 XDataAvailable;
    uint8 YDataAvailable;
    uint8 ZDataAvailable;
    uint8 XYZDataAvailable;
    uint8 XPositiveThershold;
    uint8 YPositiveThershold;
    uint8 ZPositiveThershold;
    uint8 XNegativeThershold;
    uint8 YNegativeThershold;
    uint8 ZNegativeThershold;
    uint8 MeasurementRangeOverflowed;
    uint8 InterruptOccured;
};

struct MagSensorRawData
{
    uint16 MagX;
    uint16 MagY;
    uint16 MagZ;
    uint16 Temperature;
};

struct MagSensorData
{
    float MagX;
    float MagY;
    float MagZ;
    float Temperature;
};

typedef enum
{
    LIS3MDL_TEMPSENSOR_DISABLE = 0x00,
    LIS3MDL_TEMPSENSOR_ENABLE    
} LIS3MDL_TEMPSENSOR;

typedef enum
{
    LIS3MDL_FAST_ODR_DISABLE = 0x00,
    LIS3MDL_FAST_ODR_ENABLE    
} LIS3MDL_FAST_ODR;

typedef enum
{
    LIS3MDL_SELFTEST_DISABLE = 0x00,
    LIS3MDL_SELFTEST_ENABLE
} LIS3MDL_SELFTEST;

typedef enum
{
    LIS3MDL_REBOOT_MEMORY_DISABLE = 0x00,
    LIS3MDL_REBOOT_MEMORY_ENABLE
} LIS3MDL_REBOOT_MEMORY;

typedef enum
{
    LIS3MDL_SOFTWARE_RESET_DISABLE = 0x00,
    LIS3MDL_SOFTWARE_RESET_ENABLE
} LIS3MDL_SOFTWARE_RESET;

typedef enum
{
    LIS3MDL_LOWPOWER_DISABLE = 0x00,
    LIS3MDL_LOWPOWER_ENABLE
} LIS3MDL_LOWPOWER;

typedef enum
{
    LIS3MDL_SPI_INTERFACE_MODE_4WIRE = 0x00,
    LIS3MDL_SPI_INTERFACE_MODE_3WIRE
} LIS3MDL_SPI_INTERFACE_MODE;

typedef enum
{
    LIS3MDL_OPERATING_MODE_CONTINUOUS = 0x00,
    LIS3MDL_OPERATING_MODE_SINGLE,
    LIS3MDL_OPERATING_MODE_POWERDOWN1,
    LIS3MDL_OPERATING_MODE_POWERDOWN2
} LIS3MDL_OPERATING_MODE;

typedef enum
{
    LIS3MDL_ENDIAN_DATA_LSB = 0x00,
    LIS3MDL_ENDIAN_DATA_MSB
} LIS3MDL_ENDIAN_DATA;

typedef enum
{
    LIS3MDL_FAST_READ_DISABLE = 0x00,
    LIS3MDL_FAST_READ_ENABLE
} LIS3MDL_FAST_READ;

typedef enum
{
    LIS3MDL_BLOCK_DATA_UPDATE_CONTINUOUS = 0x00,
    LIS3MDL_BLOCK_DATA_UPDATE_SINGLE_READ    
} LIS3MDL_BLOCK_DATA_UPDATE;

typedef enum
{
    LIS3MDL_DATA_OVERRUN_NONE = 0x00,
    LIS3MDL_DATA_OVERRUN_OCCURED    
} LIS3MDL_DATA_OVERRUN;

typedef enum
{
    LIS3MDL_DATA_AVAILABLE_NONE = 0x00,
    LIS3MDL_DATA_AVAILABLE_YES    
} LIS3MDL_DATA_AVAILABLE;

typedef enum
{
    LIS3MDL_INTERRUPT_DISABLE = 0x00,
    LIS3MDL_INTERRUPT_ENABLE
} LIS3MDL_INTERRUPT;

typedef enum
{
    LIS3MDL_INTERRUPT_POLARITY_ACTIVE_LOW = 0x00,
    LIS3MDL_INTERRUPT_POLARITY_ACTIVE_HIGH
} LIS3MDL_INTERRUPT_POLARITY;

typedef enum
{
    LIS3MDL_INTERRUPT_LATCH_NONE = 0x00,
    LIS3MDL_INTERRUPT_LATCH_YES
} LIS3MDL_INTERRUPT_LATCH;

typedef enum
{
    LIS3MDL_INTERRPUT_ON_PIN_DISABLE = 0x00,
    LIS3MDL_INTERRPUT_ON_PIN_ENABLE
} LIS3MDL_INTERRPUT_ON_PIN;

typedef enum
{
    LIS3MDL_POSITIVE_THRESHOLD_NOT_EXCEEDED = 0x00,
    LIS3MDL_POSITIVE_THRESHOLD_EXCEEDED
} LIS3MDL_POSITIVE_THRESHOLD;

typedef enum
{
    LIS3MDL_NEGATIVE_THRESHOLD_NOT_EXCEEDED = 0x00,
    LIS3MDL_NEGATIVE_THRESHOLD_EXCEEDED
} LIS3MDL_NEGATIVE_THRESHOLD;

typedef enum
{
    LIS3MDL_MEASUREMENT_RANGE_OVERFLOW_FALSE = 0x00,
    LIS3MDL_MEASUREMENT_RANGE_OVERFLOW_TRUE
} LIS3MDL_MEASUREMENT_RANGE_OVERFLOW;

typedef enum
{
    LIS3MDL_INTERRUPT_OCCURED_FALSE = 0x00,
    LIS3MDL_INTERRUPT_OCCURED_TRUE
} LIS3MDL_INTERRUPT_OCCURED;

typedef enum  // Mscale 
{
  LIS3MDL_FULLSCALE_4GAUSS = 0x00,  // 0.15 mG per LSB
  LIS3MDL_FULLSCALE_8GAUSS,      // 0.30 mG per LSB
  LIS3MDL_FULLSCALE_12GAUSS,     // 0.60 mG per LSB
  LIS3MDL_FULLSCALE_16GAUSS      // 1.20 mG per LSB
} LIS3MDL_FULLSCALE;

typedef enum  // Mopmode // Mom
{
  LIS3MDL_PERFORMANCE_MODE_LOWPOWER = 0x00,   
  LIS3MDL_PERFORMANCE_MODE_NORMAL,       
  LIS3MDL_PERFORMANCE_MODE_HIGH,      
  LIS3MDL_PERFORMANCE_MODE_ULTRAHIGH       
} LIS3MDL_PERFORMANCE_MODE;

typedef enum
{ 
  LIS3MDL_ODR_DIVISOR_1 = 0x00,    // default, magnetometer ODR is 1/1 of the accel ODR
  LIS3MDL_ODR_DIVISOR_2,
  LIS3MDL_ODR_DIVISOR_4,
  LIS3MDL_ODR_DIVISOR_8,  
  LIS3MDL_ODR_DIVISOR_16,
  LIS3MDL_ODR_DIVISOR_32,
  LIS3MDL_ODR_DIVISOR_64, 
  LIS3MDL_ODR_DIVISOR_128 
} LIS3MDL_ODR_DIVISOR;

typedef enum
{        
  LIS3MDL_OUTPUT_DATARATE_0_625Hz = 0x00,
  LIS3MDL_OUTPUT_DATARATE_1_25Hz,
  LIS3MDL_OUTPUT_DATARATE_2_5Hz,
  LIS3MDL_OUTPUT_DATARATE_5Hz,  
  LIS3MDL_OUTPUT_DATARATE_10Hz,
  LIS3MDL_OUTPUT_DATARATE_20Hz,
  LIS3MDL_OUTPUT_DATARATE_40Hz, 
  LIS3MDL_OUTPUT_DATARATE_80Hz 
} LIS3MDL_OUTPUT_DATARATE;

#endif



