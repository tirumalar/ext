#include <stddef.h>
#include <stdint.h>

const auto SIZE_TEMPLATE_ELEMENT = 1280;
typedef unsigned char TEMPLATE_ELEMENT[SIZE_TEMPLATE_ELEMENT];

typedef struct {
  TEMPLATE_ELEMENT LeftEyeEncode;
  TEMPLATE_ELEMENT LeftEyeTag;
  TEMPLATE_ELEMENT RightEyeEncode;
  TEMPLATE_ELEMENT RightEyeTag;
  int captureMode;  // 1 = enrollment, 2 = authentication
} BIOMETRIC_TEMPLATE_DATA;

/* Event handler - passed pointer to template data, callback must return
 * non-zero to continue processing, or 0 to signal we are done */
typedef int (*BIOMETRICT_TEMPLATE_HANDLER)(
    BIOMETRIC_TEMPLATE_DATA *dataInstance, char *buffer, int *length);

#define TEMPLATE_CAPTURE_STATE_ENROLLMENT 1
#define TEMPLATE_CAPTURE_STATE_AUTHENTICATION 2

extern BIOMETRICT_TEMPLATE_HANDLER
    g_TemplateCallbackHandler;  // declared globally so pipeline can access as
                                // well as API

/* Event structure */
typedef struct {
  uint8_t Type;
  uint8_t Code;
} EVENT;

/* Event handler */
typedef void (*EVENT_HANDLER)(EVENT *Event);

/* Event types */
#define CONN_EVENT 1
#define STATE_EVENT 2
#define SESSION_EVENT 3
#define ENROLL_EVENT 4
#define GUIDE_EVENT 5
#define PROGRESS_EVENT 6

/* Connection event codes */
#define CONN_DISCONNECTED 0
#define CONN_CONNECTING 1
#define CONN_CONNECTED 2
#define CONN_DISCONNECTING 3

/* State event codes */
#define STATE_UNKNOWN 0
#define STATE_EMPTY_DB 1
#define STATE_FIRST_ENROLLING 2
#define STATE_INACTIVE_SESSION 3
#define STATE_MATCHING 4
#define STATE_ACTIVE_SESSION 5
#define STATE_ENROLLING 6

/* Session event codes */
#define SESSION_NONE 0
#define SESSION_STARTED 1
#define SESSION_TIMEOUT 2

/* Enrollment event codes */
#define ENROLL_NONE 0
#define ENROLL_SUCCESSFUL 1
#define ENROLL_FAILED 2
#define ENROLL_CANCELLED 3
#define ENROLL_FOUND_ONE_EYE 4
#define ENROLL_ALREADY_ENROLLED 5
#define ENROLL_DATABASE_FULL 6

/* Guidance event codes */
#define GUIDE_LEFT_UNDETECTED 0
#define GUIDE_LEFT_DETECTED 1
#define GUIDE_RIGHT_UNDETECTED 2
#define GUIDE_RIGHT_DETECTED 3
#define GUIDE_DISTANCE_GOOD 4
#define GUIDE_TOO_FAR 5
#define GUIDE_TOO_CLOSE 6
#define GUIDE_LEFTRIGHT_GOOD 7
#define GUIDE_TOO_LEFT 8
#define GUIDE_TOO_RIGHT 9
#define GUIDE_HIGHLOW_GOOD 10
#define GUIDE_TOO_HIGH 11
#define GUIDE_TOO_LOW 12
#define GUIDE_EYELID_GOOD 13
#define GUIDE_EYELID_LOW 14
#define GUIDE_GAZE_GOOD 15
#define GUIDE_GAZE_OFF 16
#define GUIDE_DILATION_GOOD 17
#define GUIDE_DILATION_BAD 18
#define GUIDE_FLAT_PAPER_TRUE 19
#define GUIDE_FLAT_PAPER_FALSE 20

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************/
/*                                  Management */
/**************************************************************************************/

int8_t ELK_Init(EVENT_HANDLER Handler);
void ELK_Close();
int8_t ELK_GetCurrentState();
int8_t ELK_GetVersion(char *Version);
void ELK_SetTemplateCallback(BIOMETRICT_TEMPLATE_HANDLER handler);

/**************************************************************************************/
/*                                    Device */
/**************************************************************************************/

#define MAX_NAME_LENGTH 64
#define MAX_PATH 260

typedef struct {
#ifdef __linux__
  char DevPath[MAX_PATH];
#endif
#ifdef _WIN32
  wchar_t DevPath[MAX_PATH];
  wchar_t FriendlyName[MAX_PATH];
  int UsbVid;  // -1 means invalid (if not USB device)
  int UsbPid;  // -1 means invalid (if not USB device)
#endif
  char FileName[MAX_NAME_LENGTH];
} CAPTURE_DEVICE;

typedef struct {
  uint32_t Width;
  uint32_t Height;
  uint32_t FrameRate;
} IMAGE_RESOLUTION;

int8_t ELK_GetFirstCaptureDevice(CAPTURE_DEVICE *CaptureDevice);
int8_t ELK_GetNextCaptureDevice(CAPTURE_DEVICE *CaptureDevice);
int8_t ELK_GetCurrentCaptureDevice(CAPTURE_DEVICE *CaptureDevice);
int8_t ELK_GetFirstResolution(CAPTURE_DEVICE *CaptureDevice,
                              IMAGE_RESOLUTION *Resolution);
int8_t ELK_GetNextResolution(IMAGE_RESOLUTION *Resolution);
int8_t ELK_GetCurrentResolution(IMAGE_RESOLUTION *Resolution);
int8_t ELK_SetCaptureDevice(CAPTURE_DEVICE *CaptureDevice,
                            IMAGE_RESOLUTION *Resolution);
int8_t ELK_GetConnectionStatus();
int8_t ELK_GetDeviceInfo(uint8_t InfoType, char *Info);

/* Info types */
#define DEV_INFO_FW_VERSION 0
#define DEV_INFO_PRODUCT_NAME 2
#define DEV_INFO_SERIAL_NUMBER 6
#define DEV_INFO_BOARD_TYPE 7
#define DEV_INFO_BOARD_REV 8
#define DEV_INFO_IMAGER_TYPE 9

/**************************************************************************************/
/*                                 Database */
/**************************************************************************************/

typedef struct {
  char ID[MAX_NAME_LENGTH];
  char Name[MAX_NAME_LENGTH];
  uint8_t isAdmin;
} PERSON_1;

#define PERSON PERSON_1

int32_t ELK_GetPersonCount();
int32_t ELK_GetMaxPersonCount();
int8_t ELK_GetPerson(char *PersonID, PERSON *Person);
int8_t ELK_GetFirstPerson(PERSON *Person);
int8_t ELK_GetNextPerson(int8_t Iterator, PERSON *Person);
int8_t ELK_GetPersonDone(int8_t Iterator);
int8_t ELK_UpdatePerson(char *PersonID, PERSON *Person);
int8_t ELK_DeletePerson(char *PersonID);
int8_t ELK_DeleteAllPeople();

/**************************************************************************************/
/*                                 Configuration */
/**************************************************************************************/

#define MAX_VALUE_LENGTH 128

typedef struct {
  char Name[MAX_NAME_LENGTH];
  char Value[MAX_VALUE_LENGTH];
  uint8_t ValueType;
  uint8_t ReadAccess;
  uint8_t WriteAccess;
} SETTING;

/* Datatype of the setting value */
#define VALTYPE_STRING 0
#define VALTYPE_INT 1
#define VALTYPE_FLOAT 2
#define VALTYPE_BOOL 3

/* Access level for ReadAccess and WriteAccess */
#define ACCESS_NORMAL 0
#define ACCESS_ADMIN 1
#define ACCESS_OEM 2
#define ACCESS_FACTORY 3

int8_t ELK_GetSetting(char *Name, SETTING *Setting);
int8_t ELK_GetFirstSetting(SETTING *Setting);
int8_t ELK_GetNextSetting(int8_t Iterator, SETTING *Setting);
int8_t ELK_GetSettingDone(int8_t Iterator);
int8_t ELK_ReadSetting(char *Name, char *Value);
int8_t ELK_WriteSetting(char *Name, char *Value);
int8_t ELK_ResetSetting(char *Name);
int8_t ELK_ResetAllSettings();
int8_t ELK_GetCurrentAccessLevel();

/**************************************************************************************/
/*                                  Enrollment */
/**************************************************************************************/

int8_t ELK_StartEnrollment(PERSON *Person, uint8_t BothEyesRequired);
int8_t ELK_StartReEnrollment(PERSON *Person, uint8_t BothEyesRequired);
int8_t ELK_CancelEnrollment();

/**************************************************************************************/
/*                                  Session */
/**************************************************************************************/

int8_t ELK_StartSession();
int8_t ELK_EndSession();
int8_t ELK_IsAdminSession();
int8_t ELK_GetCurrentUser(PERSON *Person);

/**************************************************************************************/
/*                                  Preview */
/**************************************************************************************/

typedef void (*PREVIEW_HANDLER)(uint8_t *Image);

int8_t ELK_StartPreview(PREVIEW_HANDLER Handler, uint32_t *Height,
                        uint32_t *Width);
int8_t ELK_StopPreview();

#ifdef __cplusplus
}
#endif

/**************************************************************************************/
/*                                  Result Codes */
/**************************************************************************************/

/* Result codes (returned from routines) */
inline bool ElErr(int8_t code) { return code < 0; }
#define RESULT_SUCCESS 0
#define RESULT_NO_ITERATORS (-1)
#define RESULT_DB_EMPTY (-2)
#define RESULT_NOT_FOUND (-3)
#define RESULT_INVALID_ITERATOR (-4)
#define RESULT_RECORD_CORRUPT (-5)
#define RESULT_SCHEMA_UNSUPPORTED (-6)
#define RESULT_ID_ALREADY_IN_DB (-7)
#define RESULT_DB_FULL (-8)
#define RESULT_FILE_NOT_FOUND (-9)
#define RESULT_FILE_INVALID (-10)
#define RESULT_NOT_IDLE (-11)
#define RESULT_PLUGIN_LOAD_FAILED (-12)
#define RESULT_OUT_OF_MEMORY (-13)
#define RESULT_INVALID_STATE (-14)
#define RESULT_NOT_AUTHORIZED (-15)
#define RESULT_THREADCT_UNSUPPORTED (-16)
#define RESULT_ALLOCATIONS_FAILED (-17)
#define RESULT_THREAD_START_FAILED (-18)
#define RESULT_INVALID_VALUE (-19)
#define RESULT_ENCRYPT_FAILED (-20)
#define RESULT_DECRYPT_FAILED (-21)
#define RESULT_HASH_FAILED (-22)
#define RESULT_DB_ERROR (-23)
#define RESULT_DIRECTSHOW_ERROR (-24)
#define RESULT_INVALID_OPERATION (-25)
#define RESULT_CONFIG_ERROR (-26)
#define RESULT_IO_ERROR (-27)
#define RESULT_INVALID_PARAMETER (-28)
#define RESULT_NO_CAMERA (-29)
#define RESULT_USER_BAD_PLACEMENT (-30)
