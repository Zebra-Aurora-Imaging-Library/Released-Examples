/******************************************************************************/
/*
* File name: CclinkSlave.h
*
* Synopsis:  This program allocates a MIL application and system.
*            Then allocate a MIL industrial communication context to an
*            CC-Link IE Field Basic Slave instance.
*
* Notes:     This example is only available if you have the MIL Industrial Communication package,
*            or another relevant update installed.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/
#pragma once

#include <mil.h>

class CclinkSlave
{
public:
	CclinkSlave(MIL_ID SystemId);
	~CclinkSlave();

	bool isSlaveStopped(void);

   void WriteRegister(const MIL_INT deviceNumber, const MIL_UINT16 value);
   void SetFlag(const MIL_INT deviceNumber);
   void ClearFlag(const MIL_INT deviceNumber);

   MIL_UINT16 ReadRegister(const MIL_INT deviceNumber);
   MIL_BOOL ReadFlag(const MIL_INT deviceNumber);

private:
   void WriteFlag(const MIL_INT deviceNumber, const MIL_BOOL value);
   MIL_ID _systemId;
};

