#ifndef ECLab_H
#define ECLab_H

#include <epicsExport.h>
#include <iocsh.h>

#include "asynDriver.h"
#include "asynOctet.h"
#include "asynOctetSyncIO.h"
#include "asynPortDriver.h"

#include <biologic_ec_sdk/ECLibAPI.h>
#include <vector>

#define COMMUNICATION_TIMEOUT_IN_S 120

class ECLabTechnique;

class ECLab : public asynPortDriver {

public:
  ECLab(const char *portName, const char *ip, int channel_mask = 1);
  virtual ~ECLab();

  asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t nChars,
                        size_t *nActual, int *eomReason);
  asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

  virtual asynStatus checkError(ErrorCode error_code);
  virtual asynStatus connect(const char *ip, int channel_mask);
  virtual asynStatus createExperiment();

  void registerTechnique(ECLabTechnique *technique);

protected:
  int P_IP;
  int P_CHANNEL_MASK;
  int P_CONNECT;
  int P_LIB_VERSION;
  int P_DEVICE_CODE;
  int P_NUMBER_OF_SLOTS;
  int P_CHANNEL_PLUGGED;
  int P_FIRMWARE_STATUS;

private:
  ConnectionHandle connection_handle = NULL;
  DeviceInfo device_info;
  ExperimentHandle experiment_handle = NULL;
  std::vector<ECLabTechnique *> techniques_;
};

#endif
