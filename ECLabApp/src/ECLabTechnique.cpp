#include "ECLabTechnique.h"

#include <epicsExport.h>
#include <errlog.h>
#include <iocsh.h>

static const char *driverName = "ECLabTechnique";

ECLabTechnique::ECLabTechnique(const char *portName, ECLab *eclab,
                               TechniqueIdentifier techniqueType)
    : asynPortDriver(portName, 1,
                     asynInt32Mask | asynFloat64Mask | asynOctetMask,
                     asynInt32Mask | asynFloat64Mask | asynOctetMask,
                     ASYN_CANBLOCK, 1, 0, 0),
      eclab_(eclab), techniqueType_(techniqueType) {
  eclab_->registerTechnique(this);
}

static asynStatus createTechnique(const char *portName, const char *parentPort,
                                  TechniqueIdentifier techniqueType) {
  void *driver = findAsynPortDriver(parentPort);

  if (driver == nullptr) {
    errlogPrintf("%s: Parent port '%s' was not found\n", driverName,
                 parentPort);
    return asynError;
  }

  auto *parent = dynamic_cast<ECLab *>(static_cast<asynPortDriver *>(driver));

  if (parent == nullptr) {
    errlogPrintf("%s: Port '%s' is not an ECLab driver\n", driverName,
                 parentPort);
    return asynError;
  }

  new ECLabTechnique(portName, parent, techniqueType);
  return asynSuccess;
}

static const iocshArg techniqueArg0 = {"Technique port name", iocshArgString};
static const iocshArg techniqueArg1 = {"ECLab port name", iocshArgString};

static const iocshArg *const techniqueArgs[] = {&techniqueArg0, &techniqueArg1};

static const iocshFuncDef techniqueOCVFuncDef = {"TechniqueOCV", 2,
                                                 techniqueArgs};
static const iocshFuncDef techniqueCAFuncDef = {"TechniqueCA", 2,
                                                techniqueArgs};
static const iocshFuncDef techniqueCPFuncDef = {"TechniqueCP", 2,
                                                techniqueArgs};
static const iocshFuncDef techniqueCVFuncDef = {"TechniqueCV", 2,
                                                techniqueArgs};
static const iocshFuncDef techniquePEISFuncDef = {"TechniquePEIS", 2,
                                                  techniqueArgs};
static const iocshFuncDef techniqueGEISFuncDef = {"TechniqueGEIS", 2,
                                                  techniqueArgs};
static const iocshFuncDef techniqueVSCANFuncDef = {"TechniqueVSCAN", 2,
                                                   techniqueArgs};
static const iocshFuncDef techniqueISCANFuncDef = {"TechniqueISCAN", 2,
                                                   techniqueArgs};
static const iocshFuncDef techniqueLOOPSTARTFuncDef = {"TechniqueLOOPSTART", 2,
                                                       techniqueArgs};
static const iocshFuncDef techniqueLOOPENDFuncDef = {"TechniqueLOOPEND", 2,
                                                     techniqueArgs};

static void TechniqueOCVCallFunc(const iocshArgBuf *args) {
  createTechnique(args[0].sval, args[1].sval, EC_SDK_TECHNIQUE_OCV);
}

static void TechniqueCACallFunc(const iocshArgBuf *args) {
  createTechnique(args[0].sval, args[1].sval, EC_SDK_TECHNIQUE_CA);
}

static void TechniqueCPCallFunc(const iocshArgBuf *args) {
  createTechnique(args[0].sval, args[1].sval, EC_SDK_TECHNIQUE_CP);
}

static void TechniqueCVCallFunc(const iocshArgBuf *args) {
  createTechnique(args[0].sval, args[1].sval, EC_SDK_TECHNIQUE_CV);
}

static void TechniquePEISCallFunc(const iocshArgBuf *args) {
  createTechnique(args[0].sval, args[1].sval, EC_SDK_TECHNIQUE_PEIS);
}

static void TechniqueGEISCallFunc(const iocshArgBuf *args) {
  createTechnique(args[0].sval, args[1].sval, EC_SDK_TECHNIQUE_GEIS);
}

static void TechniqueVSCANCallFunc(const iocshArgBuf *args) {
  createTechnique(args[0].sval, args[1].sval, EC_SDK_TECHNIQUE_VSCAN);
}

static void TechniqueISCANCallFunc(const iocshArgBuf *args) {
  createTechnique(args[0].sval, args[1].sval, EC_SDK_TECHNIQUE_ISCAN);
}

static void TechniqueLOOPSTARTCallFunc(const iocshArgBuf *args) {
  createTechnique(args[0].sval, args[1].sval, EC_SDK_TECHNIQUE_LOOP_START);
}

static void TechniqueLOOPENDCallFunc(const iocshArgBuf *args) {
  createTechnique(args[0].sval, args[1].sval, EC_SDK_TECHNIQUE_LOOP_START);
}

static void ECLabTechniqueRegister() {
  iocshRegister(&techniqueOCVFuncDef, TechniqueOCVCallFunc);
  iocshRegister(&techniqueCAFuncDef, TechniqueCACallFunc);
  iocshRegister(&techniqueCPFuncDef, TechniqueCPCallFunc);
  iocshRegister(&techniqueCVFuncDef, TechniqueCVCallFunc);
  iocshRegister(&techniquePEISFuncDef, TechniquePEISCallFunc);
  iocshRegister(&techniqueGEISFuncDef, TechniqueGEISCallFunc);
  iocshRegister(&techniqueVSCANFuncDef, TechniqueVSCANCallFunc);
  iocshRegister(&techniqueISCANFuncDef, TechniqueISCANCallFunc);
  iocshRegister(&techniqueLOOPSTARTFuncDef, TechniqueLOOPSTARTCallFunc);
  iocshRegister(&techniqueLOOPENDFuncDef, TechniqueLOOPENDCallFunc);
}

epicsExportRegistrar(ECLabTechniqueRegister);
