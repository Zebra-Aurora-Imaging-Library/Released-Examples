//***************************************************************************************/
//
// File name: RobotArm.h
//
// Synopsis:  This file provides crude graphical animations for a robot arm.
//            It does not accurately represent the movement of a real robot; mil does not provide this sort of functionality.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/

#include <mil.h>
#include <math.h>

static const MIL_DOUBLE ANIMATION_FPS = 30;       // Animation frames per second.

enum class EOrientation
   {
   eZUp,
   eZDown
   };

class CRobotArmAnimation
   {
   public:
      // Constructor.
      CRobotArmAnimation(MIL_ID       Display,
                         MIL_DOUBLE   BasePosX,
                         MIL_DOUBLE   BasePosY,
                         MIL_DOUBLE   BasePosZ,
                         MIL_DOUBLE   Radius,
                         MIL_DOUBLE   LengthA,
                         MIL_DOUBLE   LengthB,
                         MIL_DOUBLE   LengthC,
                         MIL_DOUBLE   Speed,
                         MIL_INT64    ArmColor,
                         MIL_INT64    JointColor,
                         EOrientation Orientation);

      // Moves the robot.
      void Move(MIL_ID Matrix, MIL_DOUBLE SafetyHeight);
      void Move(MIL_ID Matrix);
      void MoveInstant(MIL_ID Matrix);

      // Graphic annotations. Public so we can attach more graphics on the robot.
      MIL_INT64           m_SectionA;
      MIL_INT64           m_SectionB;
      MIL_INT64           m_SectionC;
      MIL_INT64           m_JointAB;
      MIL_INT64           m_JointBC;

   private:
      // Non-owned objects.
      MIL_ID              m_System;
      MIL_ID              m_Display;
      MIL_ID              m_GraList;

      // Temporary geometries used for movement.
      MIL_UNIQUE_3DGEO_ID m_Parallel;
      MIL_UNIQUE_3DGEO_ID m_Perpendicular;

      // Size and position.
      MIL_DOUBLE m_BasePosX;        // Position of the robot base
      MIL_DOUBLE m_BasePosY;
      MIL_DOUBLE m_BasePosZ;
      MIL_DOUBLE m_Radius;          // Arm radius
      MIL_DOUBLE m_LengthA;         // Arm section lengths
      MIL_DOUBLE m_LengthB;
      MIL_DOUBLE m_LengthC;
      MIL_DOUBLE m_Speed;           // Robot speed
      MIL_INT64  m_ArmColor;        // Color of the arm's main sections
      MIL_INT64  m_JointColor;      // Color of the arm's joints
      EOrientation m_Orientation;   // Whether up is +Z or -Z
   };


//****************************************************************************
// Setup the robot arm graphics.
//****************************************************************************
CRobotArmAnimation::CRobotArmAnimation(MIL_ID       Display,
                                       MIL_DOUBLE   BasePosX,
                                       MIL_DOUBLE   BasePosY,
                                       MIL_DOUBLE   BasePosZ,
                                       MIL_DOUBLE   Radius,
                                       MIL_DOUBLE   LengthA,
                                       MIL_DOUBLE   LengthB,
                                       MIL_DOUBLE   LengthC,
                                       MIL_DOUBLE   Speed,
                                       MIL_INT64    ArmColor,
                                       MIL_INT64    JointColor,
                                       EOrientation Orientation):
   m_Display(Display),
   m_System(MobjInquire(Display, M_OWNER_SYSTEM, M_NULL)),
   m_GraList((MIL_ID)M3ddispInquire(Display, M_3D_GRAPHIC_LIST_ID, M_NULL)),
   m_Parallel(M3dgeoAlloc(m_System, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID)),
   m_Perpendicular(M3dgeoAlloc(m_System, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID)),
   m_BasePosX(BasePosX),
   m_BasePosY(BasePosY),
   m_BasePosZ(BasePosZ),
   m_Radius(Radius),   
   m_LengthA(LengthA),   
   m_LengthB(LengthB),
   m_LengthC(LengthC),
   m_Speed(Speed),
   m_ArmColor(ArmColor),
   m_JointColor(JointColor),
   m_Orientation(Orientation)
   {
   M3ddispControl(m_Display, M_UPDATE, M_DISABLE);

   // Base.
   M3dgraControl(m_GraList, M_DEFAULT_SETTINGS, M_COLOR, m_JointColor);
   M3dgraSphere(m_GraList, M_ROOT_NODE, m_BasePosX, m_BasePosY, m_BasePosZ, m_Radius, M_DEFAULT);

   // Section C.
   m_SectionC = M3dgraNode(m_GraList, M_ROOT_NODE, M_DEFAULT, M_DEFAULT);
   M3dgraBox(m_GraList, m_SectionC, M_CENTER_AND_DIMENSION, 0, 0, m_Radius / 4, m_Radius * 1.4, m_Radius * 1.4, m_Radius / 2, M_DEFAULT, M_DEFAULT);
   M3dgraCylinder(m_GraList, m_SectionC, M_POINT_AND_VECTOR, 0, 0, m_Radius / 2, 0, 0, m_Radius / 2, m_Radius, M_DEFAULT, M_DEFAULT);
   m_JointBC = M3dgraSphere(m_GraList, m_SectionC, 0, 0, m_LengthC + m_Radius, m_Radius, M_DEFAULT);

   M3dgraControl(m_GraList, M_DEFAULT_SETTINGS, M_COLOR, m_ArmColor);
   M3dgraCylinder(m_GraList, m_SectionC, M_POINT_AND_VECTOR, 0, 0, m_Radius, 0, 0, m_LengthC, m_Radius, M_DEFAULT, M_DEFAULT);

   // Temp section A and B.
   m_SectionA = M3dgraNode(m_GraList, M_ROOT_NODE, M_DEFAULT, M_DEFAULT);
   m_SectionB = M3dgraNode(m_GraList, M_ROOT_NODE, M_DEFAULT, M_DEFAULT);
   m_JointAB = M3dgraNode(m_GraList, M_ROOT_NODE, M_DEFAULT, M_DEFAULT);

   // This takes care of properly creating sections A and B.
   MoveInstant(M_IDENTITY_MATRIX);
   }

//****************************************************************************
// Moves the robot arm to a new position instantly.
//****************************************************************************
void CRobotArmAnimation::MoveInstant(MIL_ID Matrix)
   {
   M3ddispControl(m_Display, M_UPDATE, M_DISABLE);

   // Move section C to the desired position.
   M3dgraCopy(Matrix, M_DEFAULT, m_GraList, m_SectionC, M_TRANSFORMATION_MATRIX, M_DEFAULT);

   // Get the joint position.
   MIL_DOUBLE Joint_BC_X, Joint_BC_Y, Joint_BC_Z;
   M3dgraInquire(m_GraList, m_JointBC, M_POSITION_X + M_RELATIVE_TO_ROOT, &Joint_BC_X);
   M3dgraInquire(m_GraList, m_JointBC, M_POSITION_Y + M_RELATIVE_TO_ROOT, &Joint_BC_Y);
   M3dgraInquire(m_GraList, m_JointBC, M_POSITION_Z + M_RELATIVE_TO_ROOT, &Joint_BC_Z);

   // Get the distance from the anchor to the joint to calculate the positions of sections A and B.
   M3dgeoLine(m_Parallel, M_TWO_POINTS, m_BasePosX, m_BasePosY, m_BasePosZ, Joint_BC_X, Joint_BC_Y, Joint_BC_Z, M_DEFAULT, M_DEFAULT);
   MIL_DOUBLE Distance = M3dgeoInquire(m_Parallel, M_LENGTH, M_NULL);
   if(Distance >= m_LengthA + m_LengthB || m_LengthA >= m_LengthB + Distance || m_LengthB >= m_LengthA + Distance)
      { // The arm is not long enough.
      M3ddispControl(m_Display, M_UPDATE, M_ENABLE);
      return;
      }

   // Use the cosine law to find the joint between sections A and B.
   MIL_DOUBLE ParallelDistance = (Distance * Distance + m_LengthA * m_LengthA - m_LengthB * m_LengthB) / (2 * Distance);
   MIL_DOUBLE PerpendicularDistance = sqrt(m_LengthA * m_LengthA - ParallelDistance * ParallelDistance);
   M3dgeoLine(m_Perpendicular, M_TWO_POINTS, m_BasePosX, m_BasePosY, m_BasePosZ, Joint_BC_X, Joint_BC_Y, Joint_BC_Z, abs(ParallelDistance), M_DEFAULT);
   if(ParallelDistance < 0) // M3dgeoLine does not accept negative lengths.
      M3dimScale(m_Perpendicular, m_Perpendicular, -1, -1, -1, m_BasePosX, m_BasePosY, m_BasePosZ, M_DEFAULT);
   M3dgeoConstruct(m_Perpendicular, M_NULL, m_Perpendicular, M_LINE, M_FLIP, M_DEFAULT, M_DEFAULT);
   M3dgeoLine(m_Perpendicular, M_POINT_AND_VECTOR, M_UNCHANGED, M_UNCHANGED, M_UNCHANGED, 0, 0, (m_Orientation == EOrientation::eZUp) ? 1 : -1, PerpendicularDistance, M_DEFAULT);
   M3dmetFeatureEx(M_DEFAULT, m_Perpendicular, m_Parallel, M_NULL, m_Perpendicular, M_ORTHOGONALIZE, M_DEFAULT, M_DEFAULT);
   MIL_DOUBLE Joint_AB_X = M3dgeoInquire(m_Perpendicular, M_END_POINT_X, M_NULL);
   MIL_DOUBLE Joint_AB_Y = M3dgeoInquire(m_Perpendicular, M_END_POINT_Y, M_NULL);
   MIL_DOUBLE Joint_AB_Z = M3dgeoInquire(m_Perpendicular, M_END_POINT_Z, M_NULL);

   // Redraw sections A and B in the right positions.
   M3dgraRemove(m_GraList, m_SectionA, M_DEFAULT);
   M3dgraRemove(m_GraList, m_SectionB, M_DEFAULT);
   M3dgraRemove(m_GraList, m_JointAB, M_DEFAULT);

   M3dgraControl(m_GraList, M_DEFAULT_SETTINGS, M_APPEARANCE, M_SOLID);
   M3dgraControl(m_GraList, M_DEFAULT_SETTINGS, M_COLOR, m_ArmColor);
   M3dgraControl(m_GraList, M_DEFAULT_SETTINGS, M_FILL_COLOR, M_SAME_AS_COLOR);

   m_SectionA = M3dgraCylinder(m_GraList, M_ROOT_NODE, M_TWO_POINTS,
                               m_BasePosX, m_BasePosY, m_BasePosZ,
                               Joint_AB_X, Joint_AB_Y, Joint_AB_Z,
                               m_Radius, M_DEFAULT, M_DEFAULT);
   m_SectionB = M3dgraCylinder(m_GraList, M_ROOT_NODE, M_TWO_POINTS,
                               Joint_AB_X, Joint_AB_Y, Joint_AB_Z,
                               Joint_BC_X, Joint_BC_Y, Joint_BC_Z,
                               m_Radius, M_DEFAULT, M_DEFAULT);

   M3dgraControl(m_GraList, M_DEFAULT_SETTINGS, M_COLOR, m_JointColor);
   m_JointAB = M3dgraSphere(m_GraList, M_ROOT_NODE, Joint_AB_X, Joint_AB_Y, Joint_AB_Z, m_Radius, M_DEFAULT);

   M3ddispControl(m_Display, M_UPDATE, M_ENABLE);
   }

//****************************************************************************
// Moves the robot arm to a new position in a straight line.    
//****************************************************************************
void CRobotArmAnimation::Move(MIL_ID DstMatrix)
   {
   // Create the required matrices.
   auto SrcMatrix = M3dgeoAlloc(m_System, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);       // Position before.
   auto CurrentMatrix = M3dgeoAlloc(m_System, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);   // Intermediate position.
   M3dgraCopy(m_GraList, m_SectionC, SrcMatrix, M_DEFAULT, M_TRANSFORMATION_MATRIX, M_DEFAULT);

   // Calculate the distance the robot needs to travel.
   MIL_DOUBLE Distance = M3dmetFeature(SrcMatrix, DstMatrix, M_DISTANCE, M_DEFAULT, M_NULL);
   MIL_DOUBLE TotalTime = Distance / m_Speed;

   // Interpolate the matrices to create a smooth animation.
   MIL_DOUBLE StartTime = MappTimer(M_TIMER_READ, M_NULL);
   while(!MosKbhit())
      {
      MIL_DOUBLE Time = MappTimer(M_TIMER_READ, M_NULL);
      if(Time - StartTime >= TotalTime)
         break;

      M3dmetFeatureEx(M_DEFAULT, SrcMatrix, DstMatrix, M_NULL, CurrentMatrix, M_INTERPOLATION, (Time - StartTime) / TotalTime, M_DEFAULT);
      MoveInstant(CurrentMatrix);

      MIL_DOUBLE RemainingTime = 1 / ANIMATION_FPS - (MappTimer(M_TIMER_READ, M_NULL) - Time);
      if(RemainingTime > 0)
         MosSleep((MIL_INT)(RemainingTime * 1000));
      }

   // Move to the final position.
   MoveInstant(DstMatrix);
   }

//****************************************************************************
// Moves the robot arm to a new position in a realistic animation: First moving up, then above the new position, then down. 
//****************************************************************************
void CRobotArmAnimation::Move(MIL_ID DstMatrix, MIL_DOUBLE SafetyHeight)
   {
   auto SrcMatrix = M3dgeoAlloc(m_System, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   auto Translation = M3dgeoAlloc(m_System, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   auto DesiredPosition = M3dgeoAlloc(m_System, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);

   M3dgeoMatrixSetTransform(Translation, M_TRANSLATION, 0, 0, SafetyHeight, M_DEFAULT, M_DEFAULT);
   M3dgraCopy(m_GraList, m_SectionC, SrcMatrix, M_DEFAULT, M_TRANSFORMATION_MATRIX, M_DEFAULT);
   M3dgeoMatrixSetTransform(DesiredPosition, M_COMPOSE_TWO_MATRICES, SrcMatrix, Translation, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   Move(DesiredPosition);

   M3dgeoMatrixSetTransform(DesiredPosition, M_COMPOSE_TWO_MATRICES, DstMatrix, Translation, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   Move(DesiredPosition);

   Move(DstMatrix);
   }
