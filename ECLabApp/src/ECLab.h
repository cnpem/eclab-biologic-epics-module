#ifndef ECLab_H
#define ECLab_H

#include <epicsExport.h>
#include <iocsh.h>

#include "asynDriver.h"
#include "asynOctet.h"
#include "asynOctetSyncIO.h"
#include "asynPortDriver.h"

#include <biologic_ec_sdk/ECLibAPI.h>

class ECLab : public asynPortDriver {

public:
  ECLab(const char *portName);
  virtual ~ECLab();
};

#endif
