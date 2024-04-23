/******************************************************************************/
/*
* File name: CclinkSlave.cpp
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
#include "CclinkSlave.h"

CclinkSlave::CclinkSlave(const MIL_ID SystemId)
{
   _systemId = SystemId;
}

CclinkSlave::~CclinkSlave()
{
}

bool CclinkSlave::isSlaveStopped()
{
   unsigned int state;

   McomInquire(_systemId, M_COM_GET_CONNECTION_STATE, &state);

   return !state;
}

void CclinkSlave::WriteRegister(const MIL_INT deviceNumber, const MIL_UINT16 value)
{
   McomWrite(_systemId, M_COM_CCLINK_INPUT_REGISTER, deviceNumber, 1, &value);
}

MIL_UINT16 CclinkSlave::ReadRegister(const MIL_INT deviceNumber)
{
   auto value = MIL_UINT16(0);

   McomRead(_systemId, M_COM_CCLINK_OUTPUT_REGISTER, deviceNumber, 1, &value);
   return value;
}

void CclinkSlave::SetFlag(const MIL_INT deviceNumber)
{
   WriteFlag(deviceNumber, true);
}

void CclinkSlave::ClearFlag(const MIL_INT deviceNumber)
{
   WriteFlag(deviceNumber, false);
}

void CclinkSlave::WriteFlag(const MIL_INT deviceNumber, const MIL_BOOL value)
{
   McomWrite(_systemId, M_COM_CCLINK_INPUT_FLAG, deviceNumber, 1, &value);
}

MIL_BOOL CclinkSlave::ReadFlag(const MIL_INT deviceNumber)
{
   auto state = MIL_BOOL(false);

   McomRead(_systemId, M_COM_CCLINK_OUTPUT_FLAG, deviceNumber, 1, &state);
   return state;
}
