#ifndef ECLab_H
#define ECLab_H

#include <epicsExport.h>
#include <iocsh.h>

#include "asynDriver.h"
#include "asynOctet.h"
#include "asynOctetSyncIO.h"
#include "asynPortDriver.h"

#include <biologic_ec_sdk/ECLibAPI.h>

#define COMMUNICATION_TIMEOUT_IN_S 120

class ECLab : public asynPortDriver {

public:
  ECLab(const char *portName, const char *ip, int channel_mask = 1);
  virtual ~ECLab();

  virtual asynStatus checkError(ErrorCode error_code);
  virtual asynStatus connect(const char *ip, int channel_mask);

private:
  ConnectionHandle connection_handle = NULL;
  DeviceInfo device_info;
};

#endif
