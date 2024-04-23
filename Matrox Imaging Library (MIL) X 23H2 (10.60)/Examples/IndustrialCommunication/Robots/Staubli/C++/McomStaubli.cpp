/******************************************************************************/
/*
 * File name: McomStaubli.cpp
 *
 * Synopsis:  This program allocates a MIL application and system.
 *            Then allocate a MIL industrial communication context to a
 *            Staubli robot instance.
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

/* The address and port used to communicate with the robot */
MIL_CONST_TEXT_PTR ROBOT_IP = MIL_TEXT("127.0.0.1:2000"); /* NEED TO BE CHANGED */

void GetNextPosition(MIL_DOUBLE& x, MIL_DOUBLE& y, MIL_DOUBLE& z);

int MosMain(void)
{
   MIL_INT64 opcode;
   MIL_INT64 status;
   MIL_INT64 modelid;
   MIL_DOUBLE robot_x, robot_y, robot_z;
   MIL_DOUBLE robot_rx, robot_ry, robot_rz;

   /* Allocate a default MIL application, system, display and image. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem,
      M_NULL, M_NULL, M_NULL);

   McomAlloc(MilSystem, M_COM_PROTOCOL_STAUBLI, ROBOT_IP, M_DEFAULT, M_DEFAULT, &MilCom);


   /* Wait for a key press. */
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
   while (!MosKbhit())
   {
      /* Wait for the robot to request a new position */
      McomWaitPositionRequest(MilCom, &opcode, &status, &modelid, &robot_x, &robot_y, &robot_z, &robot_rx, &robot_ry, &robot_rz, M_DEFAULT, M_DEFAULT);

      /* Find the next position to send the robot*/
      GetNextPosition(robot_x, robot_y, robot_z);

      /* Send the next position to the robot*/
      McomSendPosition(MilCom, M_COM_ROBOT_FIND_POSITION_RESULT, 0, modelid, robot_x, robot_y, robot_z, robot_rx, robot_ry, robot_rz, M_DEFAULT, M_DEFAULT);
   }

   /* Free MIL objects*/
   McomFree(MilCom);
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);
   return 0;
}


void GetNextPosition(MIL_DOUBLE& x, MIL_DOUBLE& y, MIL_DOUBLE& z)
{
   /* Only move the robot to an offset in x in this example */
   x += 15;
   if (x > 300)
      x = 0;
}
