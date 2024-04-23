/******************************************************************************/
/*
 * File name: McomCclink.cpp
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
#include <mil.h>
#include "CclinkSlave.h"

const MIL_INT TO_PLC_DATA_READY_FLAG(0);
const MIL_INT TO_PLC_DATA_REGISTER(0);
const MIL_INT FROM_PLC_TRIGGER_FLAG(0);
const MIL_INT FROM_PLC_DATA_ACKNOWLEDGE_FLAG(1);
const MIL_INT FROM_PLC_DATA_REGISTER(0);

MIL_UINT16 _internalProcessingValue(0);

static MIL_INT WaitForTriggerFromPLC(CclinkSlave &slave);
static MIL_UINT16 DoProcessing(void);
static void WriteResultToPLC(CclinkSlave &slave, const MIL_UINT16 result);

int MosMain(void)
{
   MIL_ID MilApplication(M_NULL);     /* Application identifier.  */
   MIL_ID MilSystem(M_NULL);          /* System identifier.       */
   MIL_ID MilCom(M_NULL);             /* Industrial communication identifier. */

   MIL_UINT16 processingResult(0);

   /* Allocate a default MIL application, system, display and image. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, M_NULL, M_NULL, M_NULL);
   McomAlloc(MilSystem, M_COM_PROTOCOL_CCLINK, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_DEFAULT, &MilCom);

   /* Create a CC-Link Slave */
   auto slave = CclinkSlave(MilCom);

   if (slave.isSlaveStopped())
   {
      MosPrintf(MIL_TEXT("Please start CC-Link Master Cyclic Transmission\n"));
      MosPrintf(MIL_TEXT("Aborting sample code\n"));
      goto abort;
   }

   /* Wait for a key press. */
   MosPrintf(MIL_TEXT("Press <Enter> to end loop.\n"));

   /* Set the data to the initial values. */
   slave.ClearFlag(TO_PLC_DATA_READY_FLAG);
   slave.WriteRegister(TO_PLC_DATA_REGISTER, 0);

   while (!MosKbhit())
   {
      /* Wait that the PLC set the trigger bit */
      /* The return value will confirm is a key has been pressed to terminate the loop */
      if (!WaitForTriggerFromPLC(slave))
      {
         /* Do the requested processing */
         processingResult = DoProcessing();

         /* Write the result back to the PLC*/
         WriteResultToPLC(slave, processingResult);
      }
   }

   /* Set the data to the initial values. */
   slave.ClearFlag(TO_PLC_DATA_READY_FLAG);
   slave.WriteRegister(TO_PLC_DATA_REGISTER, 0);

   /* Free MIL objects*/
abort:
   McomFree(MilCom);
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
}

MIL_INT WaitForTriggerFromPLC(CclinkSlave &slave)
{
   auto _isTriggerSet = MIL_BOOL(false);
   auto _KeyHit = MIL_INT(0);

   MosPrintf(MIL_TEXT("Waiting for trigger from PLC.\n"));
   do
   {
      /* Wait for the trigger from the PLC */
      _isTriggerSet = slave.ReadFlag(FROM_PLC_TRIGGER_FLAG);
      MosSleep(10);
      _KeyHit = MosKbhit();
   } while (!_isTriggerSet && !_KeyHit);

   if (!_KeyHit)
      MosPrintf(MIL_TEXT("Received Trigger from PLC!\n"));

   return _KeyHit;
}

MIL_UINT16 DoProcessing(void)
{
   /* Do what need to be done when PLC send the trigger. */
   MosPrintf(MIL_TEXT("Computing new data value\n"));
   _internalProcessingValue++;
   return _internalProcessingValue;
}

void WriteResultToPLC(CclinkSlave &slave, const MIL_UINT16 result)
{
   auto _isResultRead = MIL_BOOL(false);
   auto _KeyHit = MIL_INT(0);

   /* Set the Result Ready Flag and result value */
   slave.WriteRegister(TO_PLC_DATA_REGISTER, result);
   slave.SetFlag(TO_PLC_DATA_READY_FLAG);

   /* Wait that the PLC acknowledge the result */
   MosPrintf(MIL_TEXT("Waiting fo PLC to acknowledge the data.\n"));
   do
   {
      _isResultRead = slave.ReadFlag(FROM_PLC_DATA_ACKNOWLEDGE_FLAG);
      MosSleep(10);
      _KeyHit = MosKbhit();
   } while (!_isResultRead && !_KeyHit);

   if (!_KeyHit)
   {
      auto valueFromPLC = slave.ReadRegister(FROM_PLC_DATA_REGISTER);

      MosPrintf(MIL_TEXT("Received result ACK! Value:%d CopyBack:%d\n"), result, valueFromPLC);

      /* Clear the DATA Ready Flag */
      slave.ClearFlag(TO_PLC_DATA_READY_FLAG);
   }
}
