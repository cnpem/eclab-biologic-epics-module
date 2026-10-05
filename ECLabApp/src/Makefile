TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

LIBRARY_IOC += ECLab

# install ECLab.dbd into <top>/dbd
DBD += ECLab.dbd

# specify all source files to be compiled and added to the library
#ECLab_SRCS += xxx

#=============================
# BioLogic SDK

ifeq ($(OS_CLASS),WIN32)
    # Windows SDK import library
    USR_LDFLAGS += /LIBPATH:$(ECLIB_DIR)
    ECLab_SYS_LIBS += eclib64
else
    # Linux SDK installed system-wide
    ECLab_SYS_LIBS += eclib64
endif

ECLab_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE