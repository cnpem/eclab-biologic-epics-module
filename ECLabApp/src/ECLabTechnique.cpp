#include "ECLabTechnique.h"

#include <epicsExport.h>
#include <errlog.h>
#include <iocsh.h>

static const char *driverName = "ECLabTechnique";

ECLabTechnique::ECLabTechnique(const char *portName, ECLab *eclab,
                               TechniqueIdentifier techniqueType)
    : asynPortDriver(portName, 1,
                     asynOctetMask | asynInt32Mask | asynInt32ArrayMask |
                         asynFloat64Mask | asynFloat64ArrayMask | asynEnumMask |
                         asynDrvUserMask, /* Interface mask */
                     asynOctetMask | asynInt32Mask | asynInt32ArrayMask |
                         asynFloat64Mask | asynFloat64ArrayMask |
                         asynEnumMask, /* Interrupt mask */
                     ASYN_CANBLOCK, 1, 0, 0),
      eclab_(eclab), techniqueType_(techniqueType) {
  eclab_->registerTechnique(this);
}

asynStatus ECLabTechnique::writeInt32(asynUser *pasynUser, epicsInt32 value) {
  asynStatus status = asynSuccess;
  const char *functionName = "writeInt32";
  int function = pasynUser->reason;

  auto it = parameters_.find(function);

  if (it == parameters_.end()) {
    return asynError;
  }

  it->second.floatValue = value;
  setIntegerParam(function, value);

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

asynStatus ECLabTechnique::writeFloat64(asynUser *pasynUser,
                                        epicsFloat64 value) {
  asynStatus status = asynSuccess;
  const char *functionName = "writeFloat64";
  int function = pasynUser->reason;

  auto it = parameters_.find(function);

  if (it == parameters_.end()) {
    return asynError;
  }

  it->second.floatValue = value;
  setDoubleParam(function, value);

  /* Do callbacks so higher layers see any changes */
  callParamCallbacks();

  if (status == asynSuccess)
    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s:%s: function=%d, value=%f\n",
              driverName, functionName, function, value);
  else
    asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s:%s: function=%d, value=%f\n",
              driverName, functionName, function, value);

  return status;
}

asynStatus ECLabTechnique::writeInt32Array(asynUser *pasynUser,
                                           epicsInt32 *value,
                                           size_t nElements) {
  asynStatus status = asynSuccess;
  const char *functionName = "writeInt32Array";
  int function = pasynUser->reason;

  auto it = parameters_.find(function);

  if (it == parameters_.end()) {
    return asynError;
  }

  it->second.intArray.assign(value, value + nElements);
  doCallbacksInt32Array(it->second.intArray.data(), it->second.intArray.size(),
                        function, 0);

  /* Do callbacks so higher layers see any changes */
  callParamCallbacks();

  if (status == asynSuccess) {
    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
              "%s:%s: function=%d, nElements=%zu\n", driverName, functionName,
              function, nElements);
  } else {
    asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s:%s: function=%d, status=%d\n",
              driverName, functionName, function, status);
  }

  return status;
}

asynStatus ECLabTechnique::writeFloat64Array(asynUser *pasynUser,
                                             epicsFloat64 *value,
                                             size_t nElements) {
  asynStatus status = asynSuccess;
  const char *functionName = "writeFloat64Array";
  int function = pasynUser->reason;

  auto it = parameters_.find(function);

  if (it == parameters_.end()) {
    return asynError;
  }

  it->second.floatArray.assign(value, value + nElements);
  doCallbacksFloat64Array(it->second.floatArray.data(),
                          it->second.floatArray.size(), function, 0);

  /* Do callbacks so higher layers see any changes */
  callParamCallbacks();

  if (status == asynSuccess) {
    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
              "%s:%s: function=%d, nElements=%zu\n", driverName, functionName,
              function, nElements);
  } else {
    asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s:%s: function=%d, status=%d\n",
              driverName, functionName, function, status);
  }

  return status;
}

asynStatus ECLabTechnique::drvUserCreate(asynUser *pasynUser,
                                         const char *drvInfo,
                                         const char **pptypeName,
                                         size_t *psize) {
  (void)pptypeName;
  (void)psize;

  if (!drvInfo || !*drvInfo) {
    return asynError;
  }

  std::string parameterName(drvInfo);

  size_t separator = parameterName.find_last_of(':');

  if (separator == std::string::npos || separator == parameterName.size() - 1) {
    asynPrint(pasynUser, ASYN_TRACE_ERROR,
              "%s: Invalid parameter identifier: %s\n", driverName, drvInfo);
    return asynError;
  }

  char typeCode = parameterName.back();
  parameterName.erase(separator);

  // Reuse an existing parameter
  // records reference the same SDK parameter.
  for (auto &entry : parameters_) {
    const TechniqueParameter &parameter = entry.second;

    if (parameter.name == parameterName) {
      pasynUser->reason = entry.first;
      return asynSuccess;
    }
  }

  int sdkParameter;

  if (!getSdkParameterId(parameterName, sdkParameter)) {
    asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s: Unknown SDK parameter: %s\n",
              driverName, parameterName.c_str());
    return asynError;
  }

  TechniqueParameter parameter;
  parameter.name = parameterName;
  parameter.sdkParameter = sdkParameter;

  asynParamType asynType;

  switch (typeCode) {
  case 'b':
    parameter.type = ParameterType::Boolean;
    asynType = asynParamInt32;
    break;

  case 'B':
    parameter.type = ParameterType::BooleanArray;
    asynType = asynParamInt32Array;
    break;

  case 'f':
    parameter.type = ParameterType::Float;
    asynType = asynParamFloat64;
    break;

  case 'F':
    parameter.type = ParameterType::FloatArray;
    asynType = asynParamFloat64Array;
    break;

  case 'i':
    parameter.type = ParameterType::Integer;
    asynType = asynParamInt32;
    break;

  case 'I':
    parameter.type = ParameterType::IntegerArray;
    asynType = asynParamInt32Array;
    break;

  case 'e':
    parameter.type = ParameterType::Enumeration;
    asynType = asynParamInt32;
    break;

  case 'E':
    parameter.type = ParameterType::EnumerationArray;
    asynType = asynParamInt32Array;
    break;

  default:
    asynPrint(pasynUser, ASYN_TRACE_ERROR,
              "%s: Unsupported parameter type: %c\n", driverName, typeCode);
    return asynError;
  }

  int reason;

  asynStatus status = createParam(drvInfo, asynType, &reason);

  if (status != asynSuccess) {
    return status;
  }

  parameters_[reason] = parameter;
  pasynUser->reason = reason;

  asynPrint(pasynUser, ASYN_TRACE_FLOW,
            "%s: Registered parameter %s, reason=%d, type=%c\n", driverName,
            parameterName.c_str(), reason, typeCode);

  return asynSuccess;
}

bool ECLabTechnique::getSdkParameterId(const std::string &name,
                                       int &sdkParameter) {
  static const std::unordered_map<std::string, int> parameters = {
      // EnumParameter
      {"EC_SDK_SWEEP_MODE", EC_SDK_SWEEP_MODE},

      // EnumArrayParameter
      {"EC_SDK_VS_INITIAL", EC_SDK_VS_INITIAL},

      // IntParameter
      {"EC_SDK_AVERAGE_N_TIMES", EC_SDK_AVERAGE_N_TIMES},
      {"EC_SDK_DIGITAL_FILTER_ORDER", EC_SDK_DIGITAL_FILTER_ORDER},
      {"EC_SDK_FREQUENCY_NUMBER", EC_SDK_FREQUENCY_NUMBER},
      {"EC_SDK_LOOP_ID", EC_SDK_LOOP_ID},
      {"EC_SDK_LOOP_N_TIMES", EC_SDK_LOOP_N_TIMES},
      {"EC_SDK_N_CYCLES", EC_SDK_N_CYCLES},
      {"EC_SDK_SCAN_NUMBER", EC_SDK_SCAN_NUMBER},
      {"EC_SDK_STEP_NUMBER", EC_SDK_STEP_NUMBER},
      {"EC_SDK_EXTRA_RECORDS", EC_SDK_EXTRA_RECORDS},

      // IntArrayParameter
      {"EC_SDK_GOTO_STEP_DESTINATION", EC_SDK_GOTO_STEP_DESTINATION},
      {"EC_SDK_GOTO_STEP_NUMBER_OF_ITERATIONS",
       EC_SDK_GOTO_STEP_NUMBER_OF_ITERATIONS},

      // BoolParameter
      {"EC_SDK_ENABLE_AVERAGE_EVERY_DE", EC_SDK_ENABLE_AVERAGE_EVERY_DE},
      {"EC_SDK_ENABLE_DRIFT_CORRECTION", EC_SDK_ENABLE_DRIFT_CORRECTION},
      {"EC_SDK_ENABLE_I_BOOST", EC_SDK_ENABLE_I_BOOST},
      {"EC_SDK_RECORD_EVERY_DT_ARRAY_MODE", EC_SDK_RECORD_EVERY_DT_ARRAY_MODE},
      {"EC_SDK_RECORD_EVERY_DE_ARRAY_MODE", EC_SDK_RECORD_EVERY_DE_ARRAY_MODE},
      {"EC_SDK_RECORD_EVERY_DI_ARRAY_MODE", EC_SDK_RECORD_EVERY_DI_ARRAY_MODE},

      // BoolArrayParameter
      {"EC_SDK_BOOL_ARRAY_RESERVED", EC_SDK_BOOL_ARRAY_RESERVED},

      // FloatParameter
      {"EC_SDK_AMPLITUDE_CURRENT_IN_AMP", EC_SDK_AMPLITUDE_CURRENT_IN_AMP},
      {"EC_SDK_AMPLITUDE_VOLTAGE_IN_V", EC_SDK_AMPLITUDE_VOLTAGE_IN_V},
      {"EC_SDK_BEGIN_MEASURING_E", EC_SDK_BEGIN_MEASURING_E},
      {"EC_SDK_BEGIN_MEASURING_I", EC_SDK_BEGIN_MEASURING_I},
      {"EC_SDK_DIGITAL_CUTOFF_FREQ_HIGH_IN_HZ",
       EC_SDK_DIGITAL_CUTOFF_FREQ_HIGH_IN_HZ},
      {"EC_SDK_DIGITAL_CUTOFF_FREQ_LOW_IN_HZ",
       EC_SDK_DIGITAL_CUTOFF_FREQ_LOW_IN_HZ},
      {"EC_SDK_CA_1_DC_VALUE", EC_SDK_CA_1_DC_VALUE},
      {"EC_SDK_CA_2_DC_VALUE", EC_SDK_CA_2_DC_VALUE},
      {"EC_SDK_DISPLACEMENT_VALUE", EC_SDK_DISPLACEMENT_VALUE},
      {"EC_SDK_CONTROL_DYNAMIC_RANGE", EC_SDK_CONTROL_DYNAMIC_RANGE},
      {"EC_SDK_END_MEASURING_E", EC_SDK_END_MEASURING_E},
      {"EC_SDK_END_MEASURING_I", EC_SDK_END_MEASURING_I},
      {"EC_SDK_FINAL_FREQUENCY_IN_HZ", EC_SDK_FINAL_FREQUENCY_IN_HZ},
      {"EC_SDK_INITIAL_FREQUENCY_IN_HZ", EC_SDK_INITIAL_FREQUENCY_IN_HZ},
      {"EC_SDK_REST_TIME_IN_S", EC_SDK_REST_TIME_IN_S},
      {"EC_SDK_S2S1_SHIFT_VALUE_IN_V", EC_SDK_S2S1_SHIFT_VALUE_IN_V},
      {"EC_SDK_S3S2_SHIFT_VALUE_IN_V", EC_SDK_S3S2_SHIFT_VALUE_IN_V},
      {"EC_SDK_WAIT_FOR_STEADY_IN_SINE_PERIOD",
       EC_SDK_WAIT_FOR_STEADY_IN_SINE_PERIOD},

      // FloatArrayParameter
      {"EC_SDK_CURRENT_STEP_IN_A", EC_SDK_CURRENT_STEP_IN_A},
      {"EC_SDK_DURATION_STEP_IN_S", EC_SDK_DURATION_STEP_IN_S},
      {"EC_SDK_RECORD_EVERY_DE", EC_SDK_RECORD_EVERY_DE},
      {"EC_SDK_RECORD_EVERY_DI", EC_SDK_RECORD_EVERY_DI},
      {"EC_SDK_RECORD_EVERY_DT", EC_SDK_RECORD_EVERY_DT},
      {"EC_SDK_SCAN_RATE_IN_V_PER_S", EC_SDK_SCAN_RATE_IN_V_PER_S},
      {"EC_SDK_SCAN_RATE_IN_AMP_PER_S", EC_SDK_SCAN_RATE_IN_AMP_PER_S},
      {"EC_SDK_VOLTAGE_STEP_IN_V", EC_SDK_VOLTAGE_STEP_IN_V},
  };

  auto it = parameters.find(name);

  if (it == parameters.end()) {
    return false;
  }

  sdkParameter = it->second;
  return true;
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
