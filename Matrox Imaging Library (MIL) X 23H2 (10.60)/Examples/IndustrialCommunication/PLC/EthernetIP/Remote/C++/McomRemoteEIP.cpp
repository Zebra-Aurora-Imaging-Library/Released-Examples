/******************************************************************************/
/*
* File name: McomRemoteEIP.cpp
*
* Synopsis:  This program allocates a MIL application and system.
*            Then allocate a MIL industrial communication context to an
*            Ethernet/IP instance and do remote UCMM call.
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
MilCom;                 /* Industrial communication identifier.       */

/* Status register */
MIL_UINT8 ResultReady;

void WriteControl(MIL_UINT8 trigger, MIL_UINT8 resultACK, MIL_UINT8 resultCopy);
void ReadStatus(MIL_UINT8 *result);
void SetInitialControl(void);
void GenerateTrigger(void);
MIL_UINT WaitForResultFromSlave(MIL_UINT8 *result);
void WriteResultCopy(MIL_UINT8 result);

#define EIP_SLAVE_ADDRESS MIL_TEXT("192.168.0.9")

int MosMain(void)
{
   MIL_UINT8 processingResult;

   /* Allocate a default MIL application, system, display and image. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, M_NULL, M_NULL, M_NULL);

   McomAlloc(MilSystem, M_COM_PROTOCOL_ETHERNETIP, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_DEFAULT, &MilCom);

   /* Wait for a key press. */
   MosPrintf(MIL_TEXT("Sending triggers to Ethernet/IP slave on IP ") EIP_SLAVE_ADDRESS MIL_TEXT(".\nPress <Enter> to end.\n"));

   /* Set the data to the initial values. */
   SetInitialControl();

   while (!MosKbhit())
   {
      /* Generate a trigger to begin processing on the slave */
      GenerateTrigger();

      /* Wait that the slave finish the processing */
      /* The return value will confirm is a key has been pressed to terminate the loop */
      if (!WaitForResultFromSlave(&processingResult))
      {
         /* Write back the result to simulate work with result */
         WriteResultCopy(processingResult);
      }
   }

   SetInitialControl();

   /* Free MIL objects*/
   McomFree(MilCom);
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);
   return 0;
}

void WriteControl(MIL_UINT8 trigger, MIL_UINT8 resultACK, MIL_UINT8 resultCopy)
{
   MIL_UINT8 control[2];

   control[0] = ((resultACK & 0x01) << 1) | (trigger & 0x01);
   control[1] = resultCopy;

   McomWrite(MilCom, MIL_TEXT("mcom://") EIP_SLAVE_ADDRESS MIL_TEXT("/111"), 0, sizeof(control), control);
}

void ReadStatus(MIL_UINT8 *result)
{
   MIL_UINT8 status[2];

   McomRead(MilCom, MIL_TEXT("mcom://") EIP_SLAVE_ADDRESS MIL_TEXT("/110"), 0, sizeof(status), status);
   ResultReady = (status[0] & 0x2) >> 1;
   *result = status[1];
}

void SetInitialControl(void)
{
   WriteControl(0, 0, 0);
}

void GenerateTrigger(void)
{
   MosSleep(1000);
   WriteControl(1, 0, 0);
   MosPrintf(MIL_TEXT("Send Trigger!\n"));
}

MIL_UINT WaitForResultFromSlave(MIL_UINT8 *result)
{
   MIL_INT KeyHit;
   MIL_UINT8 ResultValue = 0;

   do
   {
      /* Wait for the result from the slave */
      ReadStatus(&ResultValue);
      MosSleep(10);
      KeyHit = MosKbhit();
   } while (ResultReady == 0 && !KeyHit);

   if (!KeyHit)
   {
      MosPrintf(MIL_TEXT("Received result ready! Value:%d\n"), ResultValue);

      /* Set the acknowledge of the result */
      WriteControl(0, 1, ResultValue);
   }

   *result = ResultValue;

   return KeyHit;
}

void WriteResultCopy(MIL_UINT8 result)
{
   WriteControl(0, 1, result);
}
