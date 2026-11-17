#ifndef ECLABTECHNIQUE_H
#define ECLABTECHNIQUE_H

#include "ECLab.h"
#include "asynPortDriver.h"

#include <vector>
#include <map>
#include <unordered_map>

enum class ParameterType {
  Boolean,
  BooleanArray,
  Float,
  FloatArray,
  Integer,
  IntegerArray,
  Enumeration,
  EnumerationArray
};

struct TechniqueParameter {
  std::string name;
  int sdkParameter;
  ParameterType type;

  epicsInt32 intValue = 0;
  epicsFloat64 floatValue = 0.0;
  std::vector<epicsInt32> intArray;
  std::vector<epicsFloat64> floatArray;
};

class ECLabTechnique : public asynPortDriver {
public:
  ECLabTechnique(const char *portName, ECLab *eclab,
                 TechniqueIdentifier techniqueType);

  virtual ~ECLabTechnique() = default;

  const TechniqueIdentifier &getTechniqueType() const { return techniqueType_; }

protected:
  asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value) override;
  asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value) override;
  asynStatus writeInt32Array(asynUser *pasynUser, epicsInt32 *value,
                             size_t nElements) override;
  asynStatus writeFloat64Array(asynUser *pasynUser, epicsFloat64 *value,
                               size_t nElements) override;

  asynStatus drvUserCreate(asynUser *pasynUser, const char *drvInfo,
                           const char **pptypeName, size_t *psize) override;
  bool getSdkParameterId(const std::string &name, int &sdkParameter);

  ECLab *eclab_;
  TechniqueIdentifier techniqueType_;

  std::map<int, TechniqueParameter> parameters_;
};

#endif
