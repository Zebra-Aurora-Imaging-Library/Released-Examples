/******************************************************************************/
/*
 * File name: McomProfinet.cpp
 *
 * Synopsis:  This program allocates a MIL application and system.
 *            Then allocate a MIL industrial communication context to a
 *            Profinet instance.
 *
 * Notes:     This example is only available if you have the MIL Industrial Communication package,
 *            or another relevant update installed.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include <mil.h>

MIL_ID MilApplication,  /* Application identifier.  */
MilSystem,              /* System identifier.       */
MilCom;                 /* Industrial communication identifier. */

/* Control register */
MIL_UINT8 Trigger;
MIL_UINT8 ResultACK;
MIL_UINT8 value = 0;

void WriteStatus(MIL_UINT8 triggerACK, MIL_UINT8 resultReady, MIL_UINT8 resultValue);
void ReadControl(MIL_UINT8* result);
void SetInitialStatus(void);
MIL_INT WaitForTriggerFromPLC(void);
MIL_UINT8 DoProcessing(void);
void WriteResultToPLC(MIL_UINT8 result);

int MosMain(void)
{
   MIL_UINT8 processingResult;

   /* Allocate a default MIL application, system, display and image. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, M_NULL, M_NULL, M_NULL);

   McomAlloc(MilSystem, M_COM_PROTOCOL_PROFINET, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_DEFAULT, &MilCom);

   /* Wait for a key press. */
   MosPrintf(MIL_TEXT("Waiting for PLC trigger to happen.\nPress <Enter> to end.\n"));

   /* Set the data to the initial values. */
   SetInitialStatus();

   while (!MosKbhit())
   {
      /* Wait that the PLC set the trigger bit */
      /* The return value will confirm is a key has been pressed to terminate the loop */
      if (!WaitForTriggerFromPLC())
      {
         /* Do the requested processing */
         processingResult = DoProcessing();

         /* Write the result back to the PLC*/
         WriteResultToPLC(processingResult);
      }
   }

   SetInitialStatus();

   /* Free MIL objects*/
   McomFree(MilCom);
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);
   return 0;
}

void WriteStatus(MIL_UINT8 triggerACK, MIL_UINT8 resultReady, MIL_UINT8 resultValue)
{
   MIL_UINT8 status[2];

   status[0] = ((resultReady & 0x01) << 1) | (triggerACK & 0x01);
   status[1] = resultValue;

   /* Write in the input module slot id 1 as specified in PLC configuration */
   McomWrite(MilCom, MIL_TEXT("1"), 0, sizeof(status), status);
}

void ReadControl(MIL_UINT8 *result)
{
   MIL_UINT8 control[2];

   /* Read in the output module slot id 2 as specified in PLC configuration */
   McomRead(MilCom, MIL_TEXT("2"), 0, sizeof(control), control);

   Trigger = control[0] & 0x1;
   ResultACK = (control[0] & 0x2) >> 1;
   *result = control[1];
}

void SetInitialStatus(void)
{
   WriteStatus(0, 0, 0);
}

MIL_INT WaitForTriggerFromPLC(void)
{
   MIL_INT KeyHit;
   MIL_UINT8 ResultValue = 0;

   do
   {
      /* Wait for the trigger from the PLC */
      ReadControl(&ResultValue);
      MosSleep(10);
      KeyHit = MosKbhit();
   } while (Trigger == 0 && !KeyHit);

   if (!KeyHit)
   {
      MosPrintf(MIL_TEXT("Received Trigger!\n"));

      /* Set the TriggerACK and reset ResultReady */
      WriteStatus(1, 0, 0);
   }

   return KeyHit;
}

MIL_UINT8 DoProcessing(void)
{
   /* Do what need to be done when PLC send the trigger. */
   value++;
   return value;
}

void WriteResultToPLC(MIL_UINT8 result)
{
   MIL_INT KeyHit;
   MIL_UINT8 ResultCopy = 0;

   /* Set the result value */
   WriteStatus(0, 1, result);

   /* Wait that the PLC acknowledge the result */
   do
   {
      ReadControl(&ResultCopy);
      MosSleep(10);
      KeyHit = MosKbhit();
   } while (ResultACK == 0 && !KeyHit);

   if (!KeyHit)
   {
      MosPrintf(MIL_TEXT("Received result ACK! Value:%d Copy:%d\n"), value, ResultCopy);

      /* Set the result value */
      WriteStatus(0, 0, ResultCopy);
   }
}
