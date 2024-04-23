//*************************************************************************************
//
// File name: AgmLabelingTool.cpp
//
// Synopsis: This program uses interactive MIL graphics to label images compatible
//           with the AGM training input format.
//
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//*************************************************************************************
#include "util.h"
#include "buttons.h"
#include "labelingtool.h"
#include "labelingtoolcontroller.h"

// ===========================================================================
// Input and output paths.
// ===========================================================================
static const MIL_STRING OUTPUT_FOLDER  = MIL_TEXT(".\\");
static const MIL_STRING IMAGES_FOLDER  = MIL_STRING(M_IMAGE_PATH) + MIL_TEXT("\\PhotometricStereoWithMotion\\");
static const MIL_STRING IMAGE_FORMAT   = MIL_TEXT("*.mim");
static const MIL_STRING CONTAINER_PATH = OUTPUT_FOLDER + MIL_TEXT("TrainContainer.mbufc");
static const MIL_STRING MODEL_PATH     = OUTPUT_FOLDER + MIL_TEXT("Model.mim");

// ===========================================================================
// Tool description.
// ===========================================================================
void PrintHeader()
   {
   MosPrintf(
      MIL_TEXT("[EXAMPLE NAME]\n")
      MIL_TEXT("AgmLabelingTool\n\n")

      MIL_TEXT("[SYNOPSIS]\n")
      MIL_TEXT("This interactive tool helps you label images according \n")
      MIL_TEXT("to the AGM training input.\n")
      MIL_TEXT("The first step allows you to select your model. \n")
      MIL_TEXT("The second step allows you to label the training images \n")
      MIL_TEXT("by adding positive (blue box) and negative (red box) samples \n")
      MIL_TEXT("with the same size as your selected model.\n")
      MIL_TEXT("To confirm the model selection or a labeling, press \"Validate\".\n\n")

      MIL_TEXT("[INPUTS]\n")
      MIL_TEXT("A folder that contains all training images to label in the same file format (%s).\n\n")

      MIL_TEXT("[OUTPUTS]\n")
      MIL_TEXT("1. A model.\n")
      MIL_TEXT("2. A container buffer with all labeled training images.\n\n")

      MIL_TEXT("[MODULES USED]\n")
      MIL_TEXT("Modules used: Application, System, Display, Buffer, Graphics. \n\n"),
      IMAGE_FORMAT.c_str());
   }

//===================================================================================
//!
bool Press1ForFalseOr2ForTrue(MIL_CONST_TEXT_PTR Choice1, MIL_CONST_TEXT_PTR Choice2)
   {
   MosPrintf(MIL_TEXT("1. "));
   MosPrintf(Choice1);
   MosPrintf(MIL_TEXT("\n\n"));
   MosPrintf(MIL_TEXT("2. "));
   MosPrintf(Choice2);
   MosPrintf(MIL_TEXT("\n\n"));
   while(1)
      {
      char KeyVal = (char)MosGetch();
      if(KeyVal == '1')
         {
         return false;
         }
      else if(KeyVal == '2')
         {
         return true;
         }
      else
         {
         MosPrintf(MIL_TEXT(" Invalid option : Select '1' or '2'.\n\n"));
         }
      }
   }

//==============================================================================
//!
MIL_UNIQUE_BUF_ID SelectModelImage(MIL_ID MilSystem)
   {
   bool SelectNewModel {true};
   MIL_INT ModelAlreadyExist {0};
   MappFileOperation(M_DEFAULT, MODEL_PATH, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &ModelAlreadyExist);
   if(ModelAlreadyExist == M_YES)
      {
      MosPrintf(MIL_TEXT("A model image already exists in the output folder <%s>\n"), OUTPUT_FOLDER.c_str());
      MosPrintf(MIL_TEXT("Select an option: \n"));
      SelectNewModel = Press1ForFalseOr2ForTrue(MIL_TEXT("Use existing model image."), MIL_TEXT("Select a new model image."));
      }
   if(SelectNewModel)
      {
      return MIL_UNIQUE_BUF_ID {};
      }
   else
      {
      return MbufRestore(MODEL_PATH, MilSystem, M_UNIQUE_ID);
      }
   }

//==============================================================================
//!
std::vector<SImage> SelectImagesToLabel(MIL_ID MilSystem)
   {
   bool UseImageFiles {true};
   MIL_INT ContainerAlreadyExist {0};
   MappFileOperation(M_DEFAULT, CONTAINER_PATH, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &ContainerAlreadyExist);
   if(ContainerAlreadyExist == M_YES)
      {
      MosPrintf(MIL_TEXT("A container already exists in the output folder <%s>\n"), OUTPUT_FOLDER.c_str());
      MosPrintf(MIL_TEXT("Select an option: \n"));
      MIL_STRING UseImageFromFolderText = MIL_TEXT("Use images from folder <") + (IMAGES_FOLDER) + MIL_TEXT(">");
      UseImageFiles = Press1ForFalseOr2ForTrue(MIL_TEXT("Use existing container."), UseImageFromFolderText.c_str());
      }

   std::vector<SImage> Images;
   if(UseImageFiles)
      {
      std::vector<MIL_STRING> FilesInFolder = ListImagesInFolder(IMAGES_FOLDER + IMAGE_FORMAT);
      if(FilesInFolder.size() == 0)
         {
         MosPrintf(MIL_TEXT("No image was found in the input folder <%s> \n"), IMAGES_FOLDER.c_str());
         MosPrintf(MIL_TEXT("Check the folder then restart the labeling tool.\n"));
         }
      for(MIL_INT i = 0; i < (MIL_INT)FilesInFolder.size(); i++)
         {
         Images.emplace_back(MilSystem, IMAGES_FOLDER, FilesInFolder[i]);
         }
      }
   else
      {
      auto ExistingContainer = MbufRestore(CONTAINER_PATH, MilSystem, M_UNIQUE_ID);
      std::vector<MIL_ID> ImagesId;
      MbufInquireContainer(ExistingContainer, M_CONTAINER, M_COMPONENT_LIST, ImagesId);

      if(ImagesId.empty())
         {
         MosPrintf(MIL_TEXT("No image was found in the container <%s> :\n"), CONTAINER_PATH.c_str());
         MosPrintf(MIL_TEXT("Check the container then restart the labeling tool.\n"));
         }

      for(MIL_INT i = 0; i < (MIL_INT)ImagesId.size(); i++)
         {
         MIL_STRING ImageName = MIL_TEXT("Component ") + M_TO_STRING(i);
         Images.emplace_back(MilSystem, ImagesId[i], ImageName);
         }
      }
   return Images;
   }

// ===========================================================================
// Main.
// ===========================================================================
int MosMain(void)
   {
   PrintHeader();
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MIL_UNIQUE_APP_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_SYS_ID MilSystem = MsysAlloc(MilApplication, MIL_TEXT("M_SYSTEM_HOST"), M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   auto ModelImage = SelectModelImage(MilSystem);
   auto Images = SelectImagesToLabel(MilSystem);

   CLabelingTool LabelingTool(MilSystem, Images, ModelImage);
   LabelingTool.SetSavedModelImagePath(MODEL_PATH);
   LabelingTool.SetSavedLabeledImagesPath(CONTAINER_PATH);
   CLabelingToolView View(LabelingTool);
   CLabelingToolController Controller(LabelingTool, View);

   do
      {
      MosPrintf(MIL_TEXT("Press <Esc> to exit\n"));
      } while((MosGetch() != 27));

      return 0;
   }
