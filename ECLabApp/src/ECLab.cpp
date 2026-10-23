#include "ECLab.h"

static const char *driverName = "ECLab";

ECLab::ECLab(const char *portName, const char *ip, int channel_mask)
    : asynPortDriver(portName, 1,
                     asynOctetMask | asynInt32Mask | asynInt32ArrayMask |
                         asynFloat64Mask | asynFloat64ArrayMask | asynEnumMask |
                         asynDrvUserMask, /* Interface mask */
                     asynOctetMask | asynInt32Mask | asynInt32ArrayMask |
                         asynFloat64Mask | asynFloat64ArrayMask |
                         asynEnumMask, /* Interrupt mask */
                     ASYN_CANBLOCK, 1, 0, 0) {

  asynStatus status = asynSuccess;

  createParam("IP", asynParamOctet, &P_IP);
  createParam("CHANNEL_MASK", asynParamInt32, &P_CHANNEL_MASK);
  createParam("CONNECT", asynParamInt32, &P_CONNECT);
  createParam("LIB_VERSION", asynParamOctet, &P_LIB_VERSION);
  createParam("DEVICE_CODE", asynParamInt32, &P_DEVICE_CODE);
  createParam("NUMBER_OF_SLOTS", asynParamInt32, &P_NUMBER_OF_SLOTS);
  createParam("CHANNEL_PLUGGED", asynParamInt32, &P_CHANNEL_PLUGGED);
  createParam("FIRMWARE_STATUS", asynParamInt32, &P_FIRMWARE_STATUS);

  setStringParam(P_IP, ip);
  setIntegerParam(P_CHANNEL_MASK, channel_mask);

  status = connect(ip, channel_mask);
  if (status = asynSuccess) {
    setIntegerParam(P_CONNECT, 1);
  }
  status = createExperiment();

  callParamCallbacks();
}

ECLab::~ECLab() { checkError(BL_Disconnect(connection_handle)); }

asynStatus ECLab::writeOctet(asynUser *pasynUser, const char *value,
                             size_t nChars, size_t *nActual, int *eomReason) {
  asynStatus status = asynSuccess;
  const char *functionName = "writeOctet";
  int function = pasynUser->reason;

  if (function == P_IP) {
    setStringParam(P_IP, value);
    *nActual = nChars;
  }

  /* Do callbacks so higher layers see any changes */
  callParamCallbacks();

  if (status == asynSuccess)
    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s:%s: function=%d, value=%s\n",
              driverName, functionName, function, value);
  else
    asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s:%s: function=%d, value=%s\n",
              driverName, functionName, function, value);

  return status;
}

asynStatus ECLab::writeInt32(asynUser *pasynUser, epicsInt32 value) {
  asynStatus status = asynSuccess;
  const char *functionName = "writeInt32";
  int function = pasynUser->reason;

  if (function == P_CONNECT) {
    if (value) {
      std::string ip;
      int channel_mask;
      getStringParam(P_IP, ip);
      getIntegerParam(P_CHANNEL_MASK, &channel_mask);
      status = connect(ip.c_str(), channel_mask);
    } else {
      status = checkError(BL_Disconnect(connection_handle));
    }
    setIntegerParam(P_CONNECT, value);
  } else if (function == P_CHANNEL_MASK) {
    setIntegerParam(P_CHANNEL_MASK, value);
  }

  /* Do callbacks so higher layers see any changes */
  callParamCallbacks();

  if (status == asynSuccess)
    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s:%s: function=%d, value=%d\n",
              driverName, functionName, function, value);
  else
    asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s:%s: function=%d, value=%d\n",
              driverName, functionName, function, value);

  return status;
}

asynStatus ECLab::checkError(ErrorCode error_code) {
  const char *functionName = "checkError";

  if (EC_SDK_ERROR_NOERROR != error_code) {
    char error_msg[1024];
    BL_GetLastError(error_msg, 1024);

    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
              "%s:%s error code: %d, error message: %s\n", driverName,
              functionName, error_code, error_msg);

    return asynError;
  }
  return asynSuccess;
}

asynStatus ECLab::connect(const char *ip, int channel_mask) {
  const char *functionName = "connect";
  asynStatus status = asynSuccess;

  char lib_version[32];
  size_t lib_version_size = 32;
  status = checkError(BL_GetLibVersion(lib_version, &lib_version_size));
  if (status == asynSuccess) {
    setStringParam(P_LIB_VERSION, lib_version);
  }

  if(connection_handle != NULL) {
    checkError(BL_Disconnect(connection_handle));
  }
  status = checkError(BL_Connect(ip, COMMUNICATION_TIMEOUT_IN_S,
                                 &connection_handle, &device_info));
  if (status == asynSuccess) {
    setIntegerParam(P_DEVICE_CODE, device_info.device_code);
    setIntegerParam(P_NUMBER_OF_SLOTS, device_info.number_of_slots);
    epicsInt32 channelPlugged_[MAX_NUMBER_OF_CHANNELS] = {};
    for (int channel = 0; channel < device_info.number_of_slots; channel++) {
      channelPlugged_[channel] = device_info.channels_plugged[channel] ? 1 : 0;
    }
    doCallbacksInt32Array(channelPlugged_, MAX_NUMBER_OF_CHANNELS,
                          P_CHANNEL_PLUGGED, 0);
  }

  bool load_fw_mask[MAX_NUMBER_OF_CHANNELS] = {};
  for (size_t channel = 0; channel < MAX_NUMBER_OF_CHANNELS; channel++) {
    load_fw_mask[channel] = (channel_mask >> channel) & 0x01;
  }

  ErrorCode load_fw_results[MAX_NUMBER_OF_CHANNELS];
  status = checkError(BL_LoadFirmware(connection_handle, load_fw_mask,
                                      MAX_NUMBER_OF_CHANNELS, false,
                                      load_fw_results));

  if (status == asynSuccess) {
    epicsInt32 firmwareStatus_[MAX_NUMBER_OF_CHANNELS] = {};
    for (size_t channel = 0; channel < MAX_NUMBER_OF_CHANNELS; channel++) {
      firmwareStatus_[channel] =
          static_cast<epicsInt32>(load_fw_results[channel]);
    }
    doCallbacksInt32Array(firmwareStatus_, MAX_NUMBER_OF_CHANNELS,
                          P_FIRMWARE_STATUS, 0);
  }

  callParamCallbacks();

  return status;
}

asynStatus ECLab::createExperiment() {
  const char *functionName = "createExperiment";
  asynStatus status = asynSuccess;

  if (experiment_handle != NULL) {
    status = checkError(BL_DeleteExperiment(experiment_handle));
  }

  status = checkError(
      BL_CreateExperiment(device_info, EC_SDK_FLOATING_MODE_FLOATING,
                          EC_SDK_CONNECTION_MODE_STANDARD, &experiment_handle));

  return status;
}

extern "C" int ECLabDrive(const char *portName, const char *ip,
                          int channel_mask) {
  new ECLab(portName, ip, channel_mask);
  return asynSuccess;
}

static const iocshArg ECLabDriveArg0 = {"Port name", iocshArgString};
static const iocshArg ECLabDriveArg1 = {"IP Address", iocshArgString};
static const iocshArg ECLabDriveArg2 = {"Channel Mask", iocshArgInt};

static const iocshArg *const ECLabDriveArgs[] = {
    &ECLabDriveArg0, &ECLabDriveArg1, &ECLabDriveArg2};

static const iocshFuncDef ECLabDriveFuncDef = {"ECLabDrive", 3, ECLabDriveArgs};

static void ECLabDriveCallFunc(const iocshArgBuf *args) {
  ECLabDrive(args[0].sval, args[1].sval, args[2].ival);
}

static void ECLabRegister() {
  iocshRegister(&ECLabDriveFuncDef, ECLabDriveCallFunc);
}

epicsExportRegistrar(ECLabRegister);
