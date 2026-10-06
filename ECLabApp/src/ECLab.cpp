#include "ECLab.h"

static const char *driverName = "ECLab";

ECLab::ECLab(const char *portName)
    : asynPortDriver(portName, 1,
                     asynOctetMask | asynInt32Mask | asynFloat64Mask |
                         asynFloat64ArrayMask | asynEnumMask |
                         asynDrvUserMask, /* Interface mask */
                     asynOctetMask | asynInt32Mask | asynFloat64Mask |
                         asynFloat64ArrayMask |
                         asynEnumMask, /* Interrupt mask */
                     ASYN_CANBLOCK, 1, 0, 0) {

  asynStatus status = asynSuccess;

  char lib_version[32];
  size_t lib_version_size = 32;
  status = checkError(BL_GetLibVersion(lib_version, &lib_version_size));
  if (status == asynSuccess) {
    printf("ECLib2 version : %s.\n\n", lib_version);
  }
}

ECLab::~ECLab() {}

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

extern "C" int ECLabDrive(const char *portName) {
  new ECLab(portName);
  return asynSuccess;
}

static const iocshArg ECLabDriveArg0 = {"Port name", iocshArgString};

static const iocshArg *const ECLabDriveArgs[] = {&ECLabDriveArg0};

static const iocshFuncDef ECLabDriveFuncDef = {"ECLabDrive", 1, ECLabDriveArgs};

static void ECLabDriveCallFunc(const iocshArgBuf *args) {
  ECLabDrive(args[0].sval);
}

static void ECLabRegister() {
  iocshRegister(&ECLabDriveFuncDef, ECLabDriveCallFunc);
}

epicsExportRegistrar(ECLabRegister);
