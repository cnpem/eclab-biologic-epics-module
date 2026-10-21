#ifndef ECLABTECHNIQUE_H
#define ECLABTECHNIQUE_H

#include "ECLab.h"
#include "asynPortDriver.h"

class ECLabTechnique : public asynPortDriver {
public:
  ECLabTechnique(const char *portName, ECLab *eclab,
                 TechniqueIdentifier techniqueType);

  virtual ~ECLabTechnique() = default;

  const TechniqueIdentifier &getTechniqueType() const { return techniqueType_; }

protected:
  ECLab *eclab_;
  TechniqueIdentifier techniqueType_;
};

#endif
