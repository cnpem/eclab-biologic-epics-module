#!../../bin/linux-x86_64/ECLab

#- SPDX-FileCopyrightText: 2005 Argonne National Laboratory
#-
#- SPDX-License-Identifier: EPICS

#- You may have to change ECLab to something else
#- everywhere it appears in this file

#< envPaths

## Register all support components
dbLoadDatabase "../../dbd/ECLab.dbd"
ECLab_registerRecordDeviceDriver(pdbbase) 

## Load record instances
#dbLoadRecords("../../db/ECLab.db","user=root")

iocInit()

## Start any sequence programs
#seq sncECLab,"user=root"
