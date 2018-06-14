// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "EyelockNanoDevice.h"
#include "EyeLockThread.h"
#include "LEDConsolidator.h"
#include "AudioDispatcher.h"
#include "NwMatchManager.h"
#include "SDKDispatcher.h"
#include "F2FDispatcher.h"
#include "ELKNS.h"
#include "EyelockConfiguration.h"

using std::map;
using std::string;

using namespace  ::EyelockNano;

void* ProcessforRestartFirmware(void *ptr);

class EyelockNanoDeviceHandler : virtual public EyelockNanoDeviceIf {
 public:
  EyelockNanoDeviceHandler(Configuration *_conf);
  int32_t startImageStream(const std::string& ipaddress, const std::string& portno, const bool secure, const ELKNS_ImageFormats::type format);
  int32_t stopImageStream(const std::string& ipaddress, const std::string& portno);
  void SetEyelockThread(EyeLockThread *ptr);
  void GetFirmwareRevision(std::map<std::string, std::string> & _return, const int32_t reType);
  void SetEyelockFirmwareVersion(std::string firmwareRevision);
  void SetRGBcontrollerNano(RGBController *ptr);
  void SetConsolidator(LEDConsolidator  *ptr);
  void SetSDKDispatcher(SDKDispatcher  *ptr);
  void SetF2FDispatcher(F2FDispatcher  *ptr);
  int32_t ChangeLedColor(const int8_t mask, const int32_t time);
  void SetAudioDispatcher(AudioDispatcher *ptr);
  AudioDispatcher *GetAudioDispatcher();
  int32_t SetAudiolevel(const double volumelevel);
  double GetAudiolevel(void);
  void setNwMatchManager(NwMatchManager* _nwMatchManager);
  int32_t pushDB(const std::string& fullDB, const ACD_Type::type dbtype);
  int32_t IsDeviceTampered();
  int32_t ResetFirmware();
  int32_t PingDevice();
  int64_t GetTime();
  int32_t SyncTime(const int64_t nanoTime, const int64_t hostTime, const int32_t pingTimeout);
  int32_t RestartDevice(const ELKNS_RestartTypes::type restart);
  int32_t updateDB(const std::string& upDB, const ACD_Type::type dbtype);
  void GetConfigParameters(std::map<int32_t, std::string> & confMap);
  int32_t SetConfigParameters(const std::map<int32_t, std::string> & confMap) ;
  void WriteINI(const char* fpath,const std::map<std::string, std::pair<std::string, std::string> > & confMap);
  void RetreiveAllIDs(std::string& _return);
  void receiveChunkAndAppendFile(std::map<std::string, std::string> & _return, const std::vector<std::string> & chunkList);
  void neededChunkFromFile(std::vector<std::string> & _return, const std::map<std::string, std::string> & neededchunkInfo);
  int32_t RegisterCallBack(const std::string& ipaddress, const std::string& portno, const ELKNS_EventTypes::type Event);
  int32_t UnregisterCallBack(const std::string& ipaddress, const std::string& portno, const ELKNS_EventTypes::type Event);
  void RetrieveLogs(std::map<std::string, std::string> & logMap);
  void getDBCheckSum(std::string& _return);
  int32_t ThriftcallforMasterSlave();
  void GetSlaveVersion(std::string& _return);
  int32_t DeleteDeviceFile(const std::string& path, const bool isDirectory);
  int32_t SendRelayCommand(const ELKNS_RelayTypes::type relayType, const int32_t duration);
  int32_t UpdateFirmware(const std::map<std::string, std::string> & argMap);
  int32_t RestoreFirmware(const std::string& restorePointName, const std::map<std::string, std::string> & argMap);
  void getRestorePoints(std::vector<std::string>& _return);
  int32_t DeleteRestorePoint(const std::string& restorePointName);
  void CreateCustomKey(std::string& _return);
  int32_t ConfirmCustomKey(const std::string& keyMd5);
  int32_t SwitchToDefaultKey();
  int32_t GetKeyType();
  void GetDeviceId(std::string& _return);
  void RemoveTempDbFiles();
  int32_t SetPassword(const std::string& userName, const std::string& oldPassword, const std::string& newPassword);
  int32_t GetRecordsCount();
  void GetFileChecksum(std::string& _return, const std::string& file);

  int32_t SetIntParameter(const std::string& paramName, const int64_t value);
  void GetIntParameter(GetIntReturn& _return, const std::string& paramName);
  int32_t SetUIntParameter(const std::string& paramName, const int64_t value);
  void GetUIntParameter(GetIntReturn& _return, const std::string& paramName);
  int32_t SetDoubleParameter(const std::string& paramName, const double value);
  void GetDoubleParameter(GetDoubleReturn& _return, const std::string& paramName);
  int32_t SetBoolParameter(const std::string& paramName, const bool value);
  void GetBoolParameter(GetBoolReturn& _return, const std::string& paramName);
  int32_t SetStrParameter(const std::string& paramName, const std::string& value);
  void GetStrParameter(GetStrReturn& _return, const std::string& paramName);
  int32_t ResetConfigParameters();

 private:
  int32_t ACD_DataTypeValidation(const ACD_Type::type dbtype);
  std::string GetIcmVersion();
  int32_t getSlaveStatus(int32_t status);
  int32_t slaveConnectionValidation();
  int32_t createRestorePoint(std::string restorePointName);
  std::string constructRestorePointName(std::string currentVersion);
  int32_t deleteOldestRestorePoint(int limit);
  int32_t runSdkScript(std::string scriptFile, std::string args);
  int32_t ValidatePassword(const std::string& userName, const std::string& password);
  static bool ValidatePasswordStrength(const std::string& userName, const std::string& password);
  int32_t GenerateSaltStr(std::string& salt);
  void InitPasswordStorage();
  EyeLockThread*   m_pEyeLockThread;
  std::string      m_sStream_IPAddress;
  std::string      m_sStream_PortNo;
  std::string	   m_sVersion;
  int 			   m_nNano_Mode;
  int 			   m_nFrameType;
  bool             m_bSecurelistener;
  bool 			   m_bDetectEyes;
  RGBController*   m_pRGBController;
  std::string      m_sMask;
  LEDConsolidator* pledConsolidator;
  AudioDispatcher* m_pAudioDispatcher;
  NwMatchManager*  m_pNwMatchManager;
  SDKDispatcher*   m_pSDKDispatcher;
  F2FDispatcher*   m_pF2FDispatcher;

  EyelockNanoDeviceClient* m_pSlave;
  float		 m_volume;
  Configuration *m_conf;
  EyelockConfigurationNS::EyelockConfiguration m_eyelockConf;
  ELKNS_RestartTypes::type m_restartType;
  ELKNS_EventTypes::type Event;

  bool m_bMaster;
  bool m_bSlavePresent;
  bool m_DBMutexEnable;

  std::string m_dataDir;

  std::string m_passStorage;
  std::string m_passStorageDefault;
  std::string m_passStorageCustom;
  std::string m_unameRoFs; //indicator that device has OS installed on read-only partition, so default linux password storage cannot be modified
  bool m_bIsRoFs;
};

