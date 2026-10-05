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
  char lib_version[32];
  size_t lib_version_size = 32;
  BL_GetLibVersion(lib_version, &lib_version_size);
  printf("ECLib2 version : %s.\n\n", lib_version);
}

ECLab::~ECLab() {}

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
