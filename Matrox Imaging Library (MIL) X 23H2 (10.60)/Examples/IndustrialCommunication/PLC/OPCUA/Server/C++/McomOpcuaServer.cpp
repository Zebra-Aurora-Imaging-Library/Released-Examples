/******************************************************************************/
/*
 * File name: McomOpcuaServer.cpp
 *
 * Synopsis:  This program allocates a MIL application and system.
 *            Then allocate a MIL industrial communication context to an
 *            OPC UA Server instance.
 *
 * Notes:     This example is only available if you have the MIL Industrial Communication package,
 *            or another relevant update installed.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include <mil.h>
#include <assert.h>     /* assert */


MIL_UINT16 _internalProcessingValue(0);

int MosMain(void)
{
   MIL_ID MilApplication(M_NULL);     /* Application identifier.  */
   MIL_ID MilSystem(M_NULL);          /* System identifier.       */
   MIL_ID MilCom(M_NULL);             /* Industrial communication identifier. */

   uint8_t result;
   uint8_t data;

   MIL_UINT16 processingResult(0);

   /* Allocate a default MIL application, system, display and image. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, M_NULL, M_NULL, M_NULL);
   McomAlloc(MilSystem, M_COM_PROTOCOL_OPCUA, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_DEFAULT, &MilCom);

   /* Read in the [the.answer] object */
   McomRead(MilCom, MIL_TEXT("the.answer"), 0, 1, &result);
   MosPrintf(MIL_TEXT("the.answer = %d\n"), result);

   data = ~result;
   McomWrite(MilCom, MIL_TEXT("the.answer"), 0, 1, &data);

   McomRead(MilCom, MIL_TEXT("the.answer"), 0, 1, &result);
   MosPrintf(MIL_TEXT("the.answer = %d\n"), result);

   assert(data == result);

   /* Free MIL objects*/
   McomFree(MilCom);
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
}

