/**
 * The first thing to know about are types. The available types in Thrift are:
 *
 *  bool        Boolean, one byte
 *  byte        Signed byte
 *  i16         Signed 16-bit integer
 *  i32         Signed 32-bit integer
 *  i64         Signed 64-bit integer
 *  double      64-bit floating point value
 *  string      String
 *  binary      Blob (byte array)
 *  map<t1,t2>  Map from one type to another
 *  list<t1>    Ordered list of one type
 *  set<t1>     Set of unique elements of one type
Other examples:
typedef i32 MyInteger

Thrift also lets you define constants for use across languages. Complex
types and structs are specified using JSON notation.

const i32 INT32CONSTANT = 9853
const map<string,string> MAPCONSTANT = {'hello':'world', 'goodnight':'moon'}

enum Operation {
  ADD = 1,
  SUBTRACT = 2,
  MULTIPLY = 3,
  DIVIDE = 4
}

 * Structs are the basic complex data structures. They are comprised of fields
 * which each have an integer identifier, a type, a symbolic name, and an
 * optional default value.
 *
 * Fields can be declared "optional", which ensures they will not be included
 * in the serialized output if they aren't set.  Note that this requires some
 * manual management in some languages.
struct Work {
  1: i32 num1 = 0,
  2: i32 num2,
  3: Operation op,
  4: optional string comment,
}


 */

namespace cpp EyelockNano

typedef string GUID

enum Error_Code{
    Success,	
	User_Already_Present,
	Iris_Not_Mapped_To_User,
	User_Not_Enrolled,
	App_Already_Protected,
	Iris_Already_Present,
	Iris_Mapped_To_Other_Enrolled_User,
    Unknown,
	User_Not_Primary,
	No_device_Found,
    Connection_Already_Exists,
	Connection_Refused,
	Invalid_Input,
	No_Good_Image_Found,
	No_Eye_Found,
	No_Image_Found,
	No_Match_Found,
	Device_Offline,
	Tampered,
	Not_Tampered,
	Device_Has_Better_Version,
	Integrity_Check_Failed,
	File_Not_Accessible,
	Failure,
	Invalid_Record_Id,
	Not_Supported,
	No_Records_Found,
	No_Logs_Found
}

enum ActivityType {
  INFO = 1,
  WARN = 2,
  ERR = 3
}

struct Iris{
  1: binary m_iris_code,
  2: binary m_iris_mask,
  3: i32 m_iris_type,
}

struct ActivityLog
{ 1: i32 Log_id ;
  2: string email;
  3: string m_host_machine ;
  4: string m_Act_type;
  5: string m_Act_Desc ;
  6: string m_timeStamp ;
} 

exception EyelockNanoDeviceException {
  1: Error_Code custom_error,
  2: i32 what,
  3: string why
}

enum UpdateType {
  APP = 0,
  UNIFIED = 1,
  OS = 2,  
  FW = 3,
}

enum ELKNS_RestartTypes
{
   REBOOT_DEVICE=0,
   REBOOT_EYELOCK,
}

enum ELKNS_EventTypes
{
   DEVICE_TAMPERED=0,
   PERSON_MATCHED,
   DEVICE_DISCONNECTED,
}
enum ACD_Type
{
	WIEGAND = 0,
	F2F,
	RELAY,
	PAC,
	NONE
}

enum ELKNS_ImageFormats
{
  CENTER_CROPPED_IMAGES=0,
  CENTERED_IMAGES,
  EYE_CROPPED_IMAGES
}

enum ELKNS_RelayTypes
{
	GRANT_RELAY=0,
	DENY_RELAY
}

struct GetIntReturn
{
	1: i32 status,
	2: i64 value
}
struct GetDoubleReturn
{
	1: i32 status,
	2: double value
}
struct GetBoolReturn
{
	1: i32 status,
	2: bool value
}
struct GetStrReturn
{
	1: i32 status,
	2: string value
}

service EyelockNanoDevice 
{
    /* API from thrift */
	i32 startImageStream (1:string ipaddress, 2:string portno, 3:bool secure, 4: ELKNS_ImageFormats format);
	i32 stopImageStream (1:string ipaddress, 2:string portno);
	i32 ChangeLedColor (1:byte mask, 2:i32 time);
	map<string,string> GetFirmwareRevision(1:i32 reType);
	i32 SetAudiolevel(1:double vol);
	double GetAudiolevel();
	i32 IsDeviceTampered();
	/* dbtype param is deprecated */
	i32 pushDB(1:binary fullDB,2:ACD_Type dbtype);
	i32 ResetFirmware();
	i32 PingDevice();
	i64 GetTime();
	i32 SyncTime(1:i64 nanoTime, 2:i64 hostTime, 3:i32 pingTimeout);
	i32 RestartDevice(1:ELKNS_RestartTypes restart);
	/* dbtype param is deprecated */
	i32 updateDB(1:binary upDB,2:ACD_Type dbtype);
	map<i32, string> GetConfigParameters();
	i32 SetConfigParameters(1:map<i32, string> confMap);
	string RetreiveAllIDs();
	map<string, string> receiveChunkAndAppendFile(1:list<string> chunkList);
	list<string> neededChunkFromFile(1:map<string,string> neededchunkInfo);
	i32 RegisterCallBack(1:string ipaddress, 2:string portno, 3:ELKNS_EventTypes Event);
	i32 UnregisterCallBack(1:string ipaddress, 2:string portno, 3:ELKNS_EventTypes Event);
	map<string,string>RetrieveLogs();
	string GetSlaveVersion();
	string getDBCheckSum();
	i32 DeleteDeviceFile(1:string path,2:bool isDirectory);
	i32 SendRelayCommand (1:ELKNS_RelayTypes relayType, 2:i32 duration);
	list<string> getRestorePoints();
	i32 UpdateFirmware(1:map<string, string>filenamemap);
	i32 RestoreFirmware(1:string restorePointName, 2:map<string, string>argMap);
	i32 DeleteRestorePoint(1: string restorePointName);
	
	binary CreateCustomKey();
	i32 ConfirmCustomKey(1: string keyMd5);
	i32 SwitchToDefaultKey();	
	i32 GetKeyType();
	
	string GetDeviceId();
	
	i32 SetPassword(1: string userName, 2: string oldPassword, 3: string newPassword);

	i32 GetRecordsCount();

	string GetFileChecksum(1: string file);

	// no unsigned types are supported
	i32 SetIntParameter(1:string paramName, 2:i64 value);
	GetIntReturn GetIntParameter(1:string paramName);
	i32 SetUIntParameter(1:string paramName, 2:i64 value);
	GetIntReturn GetUIntParameter(1:string paramName);
	i32 SetDoubleParameter(1:string paramName, 2:double value);
	GetDoubleReturn GetDoubleParameter(1:string paramName);
	i32 SetBoolParameter(1:string paramName, 2:bool value);
	GetBoolReturn GetBoolParameter(1:string paramName);
	i32 SetStrParameter(1:string paramName, 2:string value);
	GetStrReturn GetStrParameter(1:string paramName);
	i32 ResetConfigParameters();

	}

