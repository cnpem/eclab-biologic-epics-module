# SPDX-FileCopyrightText: 2003 Argonne National Laboratory
#
# SPDX-License-Identifier: EPICS

TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Build the IOC application

PROD_IOC = ECLab
# ECLab.dbd will be created and installed
DBD += ECLab.dbd

# ECLab.dbd will be made up from these files:
ECLab_DBD += base.dbd

# Include dbd files from all support applications:
#ECLab_DBD += xxx.dbd

# Add all the support libraries needed by this IOC
#ECLab_LIBS += xxx

# ECLab_registerRecordDeviceDriver.cpp derives from ECLab.dbd
ECLab_SRCS += ECLab_registerRecordDeviceDriver.cpp

# Build the main IOC entry point on workstation OSs.
ECLab_SRCS_DEFAULT += ECLabMain.cpp
ECLab_SRCS_vxWorks += -nil-

# Add support from base/src/vxWorks if needed
#ECLab_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Finally link to the EPICS Base libraries
ECLab_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

