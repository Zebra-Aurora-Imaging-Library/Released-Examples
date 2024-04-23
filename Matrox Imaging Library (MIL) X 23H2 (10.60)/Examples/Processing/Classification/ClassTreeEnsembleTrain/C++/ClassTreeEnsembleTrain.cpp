﻿ //*************************************************************************************
 //
 // File name: ClassTreeEnsembleTrain.cpp
 //
 // Synopsis:  This program uses the classification module to train
 //            a context able to classify 6 different type of shapes.
 //
 //
 // Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 // All Rights Reserved

#include <windows.h>
#include <mil.h>
#include <string>
#include <algorithm>
#include <random>
#include <numeric>
#include <map>
#include <math.h>

// Path definitions.
#define EXAMPLE_SHAPES_IMAGE_ROOT_PATH      M_IMAGE_PATH MIL_TEXT("Classification/Shapes/")
#define EXAMPLE_SHAPES_ORIGINAL_DATA_PATH   M_IMAGE_PATH MIL_TEXT("Classification/Shapes/OriginalData/")
#define EXAMPLE_SHAPES_TRAIN_ROOT_PATH      MIL_TEXT("./Shapes/")
#define EXAMPLE_SHAPES_DATA_FOR_TRAIN_PATH  EXAMPLE_SHAPES_TRAIN_ROOT_PATH MIL_TEXT("TrainImages/")

#define EXAMPLE_DIGITS_IMAGE_ROOT_PATH      M_IMAGE_PATH MIL_TEXT("Classification/Digits/")
#define EXAMPLE_DIGITS_ORIGINAL_DATA_PATH   M_IMAGE_PATH MIL_TEXT("Classification/Digits/OriginalData/")
#define EXAMPLE_DIGITS_TRAIN_ROOT_PATH      MIL_TEXT("./Digits/")
#define EXAMPLE_DIGITS_DATA_FOR_TRAIN_PATH  EXAMPLE_DIGITS_TRAIN_ROOT_PATH MIL_TEXT("TrainImages/")

// Util constants.
#define NB_AUGMENTATION_PER_IMAGE_SHAPES         15L
#define AUGMENTED_IMAGES_SIZE_SHAPES             320L
#define AUGMENTED_IMAGES_OFFSET_SHAPES           40L
#define AUG_RNG_INIT_VALUE_SHAPES                1612L
#define NUMBER_OF_TRAINED_TREES_SHAPES           30L

#define NB_AUGMENTATION_PER_IMAGE_DIGITS         50L
#define AUGMENTED_IMAGES_SIZE_DIGITS             150L
#define AUGMENTED_IMAGES_OFFSET_DIGITS           60L
#define AUG_RNG_INIT_VALUE_DIGITS                5318L
#define NUMBER_OF_TRAINED_TREES_DIGITS           100L


// Target MIL image file specifications.
#define BINARIZE_IMAGE_THRESHOLD_VALUE_SHAPES    40L
#define BINARIZE_IMAGE_THRESHOLD_VALUE_DIGITS    30L
// Radius of the smallest particles to keep. 
#define MIN_BLOB_RADIUS_SHAPES                   3L
#define MIN_BLOB_RADIUS_DIGITS                   1L

static const MIL_INT NUMBER_OF_SHAPES_CLASSES         = 6;
static const MIL_INT NUMBER_OF_SHAPES_PREDICT_IMAGES  = 2;

static const MIL_INT NUMBER_OF_DIGITS_CLASSES         = 10;
static const MIL_INT NUMBER_OF_DIGITS_PREDICT_IMAGES  = 1;


//*****************************************************************************
// Example description.
//*****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("ClassTreeEnsembleTrain\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example trains a TREE_ENSEMBLE model to classify shapes and digits.\n")

             MIL_TEXT("Step 1: Prepare the Image dataset.\n")
             MIL_TEXT("Step 2: Generate augmented images.\n")
             MIL_TEXT("Step 3: Calculate blob features. \n")
             MIL_TEXT("Step 4: Train the context. \n")
             MIL_TEXT("Step 5: Perform predictions on test image using the trained TREE_ENSEMBLE model as\n")
             MIL_TEXT("        a final check of the expected model performance.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, buffer,\n")
             MIL_TEXT("graphic, classification.\n\n"));
   }

// Sample image for each classes.
MIL_STRING SHAPES_CLASS_NAMES[NUMBER_OF_SHAPES_CLASSES] =  {MIL_TEXT("square"),        // Class_0
                                                            MIL_TEXT("disk"),          // Class_1
                                                            MIL_TEXT("crossedcircle"), // Class_2
                                                            MIL_TEXT("circle"),        // Class_3
                                                            MIL_TEXT("cross"),         // Class_4
                                                            MIL_TEXT("label")          // Class_5
                                                            };

MIL_STRING DIGITS_CLASS_NAMES[NUMBER_OF_DIGITS_CLASSES] =  {MIL_TEXT("SEMI_0"),        // Class_0
                                                            MIL_TEXT("SEMI_1"),        // Class_1
                                                            MIL_TEXT("SEMI_2"),        // Class_2
                                                            MIL_TEXT("SEMI_3"),        // Class_3
                                                            MIL_TEXT("SEMI_4"),        // Class_4
                                                            MIL_TEXT("SEMI_5"),        // Class_5
                                                            MIL_TEXT("SEMI_6"),        // Class_6
                                                            MIL_TEXT("SEMI_7"),        // Class_7
                                                            MIL_TEXT("SEMI_8"),        // Class_8
                                                            MIL_TEXT("SEMI_9")         // Class_9
                                                            };       

// Number of images per classes.
MIL_INT SHAPES_CLASS_NB_IMAGES[NUMBER_OF_SHAPES_CLASSES] = {10, 10, 10, 10, 10, 10};
MIL_INT DIGITS_CLASS_NB_IMAGES[NUMBER_OF_DIGITS_CLASSES] = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2};

// Sample image for each classes.
MIL_STRING SHAPES_CLASS_SAMPLES[NUMBER_OF_SHAPES_CLASSES] = {EXAMPLE_SHAPES_IMAGE_ROOT_PATH MIL_TEXT("square_sample.mim"),
                                                             EXAMPLE_SHAPES_IMAGE_ROOT_PATH MIL_TEXT("disk_sample.mim"),
                                                             EXAMPLE_SHAPES_IMAGE_ROOT_PATH MIL_TEXT("crossedcircle_sample.mim"),
                                                             EXAMPLE_SHAPES_IMAGE_ROOT_PATH MIL_TEXT("circle_sample.mim"),
                                                             EXAMPLE_SHAPES_IMAGE_ROOT_PATH MIL_TEXT("cross_sample.mim"),
                                                             EXAMPLE_SHAPES_IMAGE_ROOT_PATH MIL_TEXT("label_sample.mim")
                                                             };

MIL_STRING DIGITS_CLASS_SAMPLES[NUMBER_OF_DIGITS_CLASSES] = {EXAMPLE_DIGITS_IMAGE_ROOT_PATH MIL_TEXT("SEMI_0.mim"),
                                                             EXAMPLE_DIGITS_IMAGE_ROOT_PATH MIL_TEXT("SEMI_1.mim"),
                                                             EXAMPLE_DIGITS_IMAGE_ROOT_PATH MIL_TEXT("SEMI_2.mim"),
                                                             EXAMPLE_DIGITS_IMAGE_ROOT_PATH MIL_TEXT("SEMI_3.mim"),
                                                             EXAMPLE_DIGITS_IMAGE_ROOT_PATH MIL_TEXT("SEMI_4.mim"),
                                                             EXAMPLE_DIGITS_IMAGE_ROOT_PATH MIL_TEXT("SEMI_5.mim"),
                                                             EXAMPLE_DIGITS_IMAGE_ROOT_PATH MIL_TEXT("SEMI_6.mim"),
                                                             EXAMPLE_DIGITS_IMAGE_ROOT_PATH MIL_TEXT("SEMI_7.mim"),
                                                             EXAMPLE_DIGITS_IMAGE_ROOT_PATH MIL_TEXT("SEMI_8.mim"),
                                                             EXAMPLE_DIGITS_IMAGE_ROOT_PATH MIL_TEXT("SEMI_9.mim")
                                                             };

struct FeatureAndName
   {
   MIL_INT Feature;
   MIL_STRING FeatureName;
   };

// Feature used for training.
std::vector<FeatureAndName> SHAPES_FEATURES {FeatureAndName{M_RECTANGULARITY,                   MIL_TEXT("Rectangularity")     },   // Feature_0
                                             FeatureAndName{M_COMPACTNESS,                      MIL_TEXT("Compactness")        },   // Feature_1
                                             FeatureAndName{M_ELONGATION,                       MIL_TEXT("Elongation")         },   // Feature_2
                                             FeatureAndName{M_BREADTH,                          MIL_TEXT("Breadth    ")        },   // Feature_3
                                             FeatureAndName{M_ROUGHNESS,                        MIL_TEXT("Roughness  ")        },   // Feature_4
                                             FeatureAndName{M_CONVEX_HULL_FILL_RATIO,           MIL_TEXT("ConvFillRat")        },   // Feature_5
                                             FeatureAndName{M_FERET_PRINCIPAL_AXIS_ELONGATION,  MIL_TEXT("PrinAxisElng")       },   // Feature_6
                                             FeatureAndName{M_FERET_MAX_DIAMETER_ELONGATION,    MIL_TEXT("MaxDiamElng")        },   // Feature_7
                                             FeatureAndName{M_FERET_MIN_DIAMETER_ELONGATION,    MIL_TEXT("MinDiamElng")        },   // Feature_8
                                             FeatureAndName{M_FERET_ELONGATION,                 MIL_TEXT("FeretElongation")    }    // Feature_9
                                             };

std::vector<FeatureAndName> DIGITS_FEATURES {FeatureAndName{M_RECTANGULARITY,                   MIL_TEXT("Rectangularity")     },   // Feature_0
                                             FeatureAndName{M_COMPACTNESS,                      MIL_TEXT("Compactness")        },   // Feature_1
                                             FeatureAndName{M_ELONGATION,                       MIL_TEXT("Elongation")         },   // Feature_2
                                             FeatureAndName{M_BREADTH,                          MIL_TEXT("Breadth    ")        },   // Feature_3
                                             FeatureAndName{M_ROUGHNESS,                        MIL_TEXT("Roughness  ")        },   // Feature_4
                                             FeatureAndName{M_CONVEX_HULL_FILL_RATIO,           MIL_TEXT("ConvFillRat")        },   // Feature_5
                                             FeatureAndName{M_FERET_PRINCIPAL_AXIS_ELONGATION,  MIL_TEXT("PrinAxisElng")       },   // Feature_6
                                             FeatureAndName{M_FERET_MAX_DIAMETER_ELONGATION,    MIL_TEXT("MaxDiamElng")        },   // Feature_7
                                             FeatureAndName{M_FERET_MIN_DIAMETER_ELONGATION,    MIL_TEXT("MinDiamElng")        },   // Feature_8
                                             FeatureAndName{M_FERET_ELONGATION,                 MIL_TEXT("FeretElongation")    },   // Feature_9
                                             FeatureAndName{M_MOMENT_HU_2,                      MIL_TEXT("HuMoment2"),         },   // Feature_10
                                             FeatureAndName{M_MOMENT_HU_3,                      MIL_TEXT("HuMoment3"),         },   // Feature_11
                                             FeatureAndName{M_MOMENT_HU_4,                      MIL_TEXT("HuMoment4"),         },   // Feature_12
                                             FeatureAndName{M_MOMENT_HU_7,                      MIL_TEXT("HuMoment7"),         },   // Feature_13
                                             FeatureAndName{M_MOMENT_CENTRAL_X0_Y3,             MIL_TEXT("CentMomentX0_Y3")    },   // Feature_14
                                             };

struct DataSettings
   {
   const MIL_STRING                    ExampleName;
   const MIL_STRING                    ExtraInformation;
   const MIL_STRING                    ImagesRootPath;
   const MIL_STRING                    TrainRootPath;
   const MIL_STRING                    DataTrainPath;
   const MIL_STRING                    OriginalDataPath;
   const MIL_INT                       NumberOfPredictImages;
   const MIL_STRING*                   ClassNames;
   const MIL_STRING*                   ClassSamples;
   const MIL_INT*                      ClassNbImages;
   const std::vector<FeatureAndName>&  ChosenFeatures;
   const MIL_INT                       NumberOfClasses;
   const MIL_INT                       NbAugmentedImages;
   const MIL_INT                       AugRngInitValue;
   const MIL_INT                       AugmentedImageSize;
   const MIL_INT                       OffsetXY;
   const MIL_INT                       NumberOfTrainedTrees;
   const MIL_INT                       MinBlobRadius;
   const MIL_DOUBLE                    BinarizeImageThreshold;
   
   void PrintHeadMessage()
      {
      MosPrintf(MIL_TEXT("\n---------------------- %s ------------------------\n\n")
                MIL_TEXT("This example trains a TREE_ENSEMBLE model to classify the %d images shown.\n"), ExampleName.c_str(), NumberOfClasses);

      if(ExtraInformation.compare(MIL_TEXT("")) != 0)
         {
         MosPrintf(ExtraInformation.c_str());
         MosPrintf(MIL_TEXT("\n"));
         }

      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n\n"));
      MosGetch();
      }
   };

//*****************************************************************************
// Example function declarations.
//*****************************************************************************
void PrepareDatasetImages(MIL_ID             SystemId,
                          MIL_ID*            DatasetImagesId,
                          const MIL_STRING&  RootPath,
                          const MIL_STRING&  DataTrainPath,
                          const MIL_STRING&  OriginalDataPath,
                          const MIL_STRING*  ClassNames,
                          const MIL_INT      NumberOfClasses,
                          const MIL_INT*     ClassNbImages);

void SplitDataset(MIL_ID  SystemId,
                  MIL_ID  DatasetImagesId,
                  MIL_ID* TrainDatasetImagesId,
                  MIL_ID* DevDatasetImagesId);

void AugmentDataset(MIL_ID        SystemId,
                    MIL_ID        TrainImagesDatasetId,
                    const MIL_INT NbAugmentPerImage,
                    const MIL_INT AugmentedImageSize,
                    const MIL_INT OffsetXY,
                    const MIL_INT AugRngInitValue);

void CalculateFeatures(MIL_ID                               SystemId,
                       MIL_ID                               TrainDatasetImagesId,
                       MIL_ID                               DevDatasetImagesId,
                       MIL_ID*                              TrainDatasetFeaturesId,
                       MIL_ID*                              DevDatasetFeaturesId,
                       MIL_ID*                              BlobContextId,
                       const MIL_STRING&                    DataTrainPath,
                       const std::vector<FeatureAndName>&   ChosenFeatures,
                       std::vector<MIL_INT64>*              ListOfEnabeledFeatures,
                       const MIL_INT                        MinBlobRadius,
                       const MIL_DOUBLE                     BinarizeImageThreshold);

void Train(MIL_ID                               SystemId,
           MIL_ID*                              TrainContextId,
           MIL_ID*                              PredictContextId,
           MIL_ID                               TrainDatasetFeaturesId,
           MIL_ID                               DevDatasetFeaturesId,
           const MIL_STRING&                    DataTrainPath,
           const std::vector<FeatureAndName>&   ChosenFeatures,
           const MIL_INT                        NumberOfTrainedTrees);

void Predict(MIL_ID                          SystemId,
             MIL_ID                          DisplayId,
             MIL_ID                          PredictContextId,
             MIL_ID                          BlobContextId,
             MIL_INT                         NumberOfFeatures,
             const std::vector<MIL_INT64>&   ListOfEnabeledFeatures,
             const MIL_STRING&               ImageRootPath,
             const MIL_STRING*               ClassNames,
             const MIL_INT                   NumberOfPredictImages,
             const MIL_INT                   MinBlobRadius,
             const MIL_DOUBLE                BinarizeImageThreshold);

//*****************************************************************************
// Utility sub-functions declarations
//*****************************************************************************

MIL_ID CreateImageOfAllClasses(MIL_ID              SystemId,
                               const MIL_STRING*   ClassSample,
                               const MIL_INT       NumberOfClasses,
                               const MIL_STRING*   ClassNames);

void FillDatasetImages(MIL_ID SystemId, MIL_ID* DatasetImagesId, const MIL_STRING& DataTrainPath, const MIL_STRING* ClassName, const MIL_INT NumberOfClasses);

void AddClassToDataset(MIL_INT ClassIndex, const MIL_STRING& DataTrainPath, const MIL_STRING& FabricName, MIL_ID Dataset);

MIL_ID ProcessImage(MIL_ID SystemId, MIL_ID ImageId, const MIL_INT MinBlobRadius, const MIL_DOUBLE BinarizeImageThreshold);

void ControlTrainContext(MIL_ID TrainContextId, const MIL_INT NumberOfTrainedTrees);

void EnableFeatures(MIL_ID BlobContextId, const std::vector<FeatureAndName>& ChosenFeatures, std::vector<MIL_INT64>* ListOfEnabeledFeatures);

void EnableFeature(MIL_ID BlobContextId,
                   MIL_INT64 ResultType,
                   std::vector<MIL_INT64>* ListOfEnabeledFeatures);

void CalculateFeaturesForDataset(MIL_ID SystemId,
                                 MIL_ID DatasetImagesId,
                                 MIL_ID BlobContextId,
                                 const std::vector<MIL_INT64>& ListOfEnabeledFeatures,
                                 MIL_ID* DatasetFeaturesId,
                                 const MIL_INT NumberOfFeatures,
                                 const MIL_INT MinBlobRadius,
                                 const MIL_DOUBLE BinarizeImageThreshold);

void AddFeaturesToVector(MIL_ID BlobResultId,
                         MIL_INT BlobIndex,
                         const std::vector<MIL_INT64>& ListOfEnabeledFeatures,
                         std::vector<MIL_DOUBLE>* FeaturesVector);

void DisplayPredictedResults(MIL_ID    ImageId,
                             MIL_INT   BlobResultId,
                             MIL_ID    BlobIndex,
                             MIL_INT   PredictedClassIndex,
                             const MIL_STRING* ClassNames);

void PrepareDataForTrainFolder(const MIL_STRING& TrainRootPath, const MIL_STRING& DataForTrainPath, const MIL_STRING* ClassNames, const MIL_INT NumberOfClasses);

void CopyOriginalDataToDataForTrainFolder(const MIL_STRING* ClassName,
                                          const MIL_INT NumberOfClasses,
                                          const MIL_INT* ClassNbImages,
                                          const MIL_STRING& OriginalDataPath,
                                          const MIL_STRING& DataForTrainPath);

void PredictOnImageAndDisplayResults(MIL_ID SystemId,
                                     MIL_ID DisplayId,
                                     MIL_ID PredictContextId,
                                     MIL_ID PredictImageId,
                                     MIL_ID BlobContextId,
                                     const MIL_INT NumberOfFeatures,
                                     const std::vector<MIL_INT64>& ListOfEnabeledFeatures,
                                     const MIL_STRING* ClassNames,
                                     const MIL_INT MinBlobRadius,
                                     const MIL_DOUBLE BinarizeImageThreshold);

MIL_STRING GetExampleCurrentDirectory();

void ListFilesInFolder(const MIL_STRING& FolderName, std::vector<MIL_STRING>& FilesInFolder);

void DeleteFilesInFolder(const MIL_STRING& FolderName);

void DeleteFiles(const std::vector<MIL_STRING>& Files);

void DeleteFileIfExisiting(MIL_STRING FileName);

//*****************************************************************************
// Main
//*****************************************************************************
int MosMain()
   {
   MIL_ID ApplicationId,
          SystemId,
          DisplayId;

   std::vector<DataSettings> Data;

   // Allocate defaults.
   ApplicationId = MappAlloc(M_NULL, M_DEFAULT, M_NULL);
   SystemId      = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_NULL);
   DisplayId     = MdispAlloc(SystemId, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_NULL);

   // Print Header.
   PrintHeader();

   // Shapes settings
   Data.push_back(DataSettings{MIL_TEXT("SHAPES"),
                               MIL_TEXT(""),
                               EXAMPLE_SHAPES_IMAGE_ROOT_PATH,
                               EXAMPLE_SHAPES_TRAIN_ROOT_PATH,
                               EXAMPLE_SHAPES_DATA_FOR_TRAIN_PATH,
                               EXAMPLE_SHAPES_ORIGINAL_DATA_PATH,
                               NUMBER_OF_SHAPES_PREDICT_IMAGES,
                               SHAPES_CLASS_NAMES,
                               SHAPES_CLASS_SAMPLES,
                               SHAPES_CLASS_NB_IMAGES,
                               SHAPES_FEATURES,
                               NUMBER_OF_SHAPES_CLASSES,
                               NB_AUGMENTATION_PER_IMAGE_SHAPES,
                               AUG_RNG_INIT_VALUE_SHAPES,
                               AUGMENTED_IMAGES_SIZE_SHAPES,
                               AUGMENTED_IMAGES_OFFSET_SHAPES,
                               NUMBER_OF_TRAINED_TREES_SHAPES,
                               MIN_BLOB_RADIUS_SHAPES,
                               BINARIZE_IMAGE_THRESHOLD_VALUE_SHAPES});

   // Digits settings
   Data.push_back(DataSettings{MIL_TEXT("DIGITS"),
                               MIL_TEXT("Central moment X0_Y3 and Hu moment invariants are used to increase robustness "
                                        "of train and prediction for all digits.\nIn this example, central moment X0_Y3 "
                                        "also makes it possible to discriminate 6 and 9.\n"),
                               EXAMPLE_DIGITS_IMAGE_ROOT_PATH,
                               EXAMPLE_DIGITS_TRAIN_ROOT_PATH,
                               EXAMPLE_DIGITS_DATA_FOR_TRAIN_PATH,
                               EXAMPLE_DIGITS_ORIGINAL_DATA_PATH,
                               NUMBER_OF_DIGITS_PREDICT_IMAGES,
                               DIGITS_CLASS_NAMES,
                               DIGITS_CLASS_SAMPLES,
                               DIGITS_CLASS_NB_IMAGES,
                               DIGITS_FEATURES,
                               NUMBER_OF_DIGITS_CLASSES,
                               NB_AUGMENTATION_PER_IMAGE_DIGITS,
                               AUG_RNG_INIT_VALUE_DIGITS,
                               AUGMENTED_IMAGES_SIZE_DIGITS,
                               AUGMENTED_IMAGES_OFFSET_DIGITS,
                               NUMBER_OF_TRAINED_TREES_DIGITS,
                               MIN_BLOB_RADIUS_DIGITS,
                               BINARIZE_IMAGE_THRESHOLD_VALUE_DIGITS});

   // Launching example for each element in Data
   for (DataSettings DataElement : Data)
      {
      MIL_ID DatasetImagesId,
             TrainDatasetImagesId,
             DevDatasetImagesId,
             TrainDatasetFeaturesId,
             DevDatasetFeaturesId,
             BlobContextId,
             TrainContextId,
             PredictContextId;

      std::vector<MIL_INT64> ListOfEnabeledFeatures;

      //Display a representative image of all classes.
      MIL_ID AllClassesImage = CreateImageOfAllClasses(SystemId, DataElement.ClassSamples,
                                                       DataElement.NumberOfClasses, DataElement.ClassNames);
      MdispSelect(DisplayId, AllClassesImage);

      DataElement.PrintHeadMessage();
      
      // Fill ImagesDataset with images's form folder.
      PrepareDatasetImages(SystemId, &DatasetImagesId, DataElement.TrainRootPath, DataElement.DataTrainPath,
                           DataElement.OriginalDataPath, DataElement.ClassNames, DataElement.NumberOfClasses,
                           DataElement.ClassNbImages);

      // Split Dataset
      SplitDataset(SystemId, DatasetImagesId, &TrainDatasetImagesId, &DevDatasetImagesId);

      // Generate augmented images and add them to ImagesDataset.
      AugmentDataset(SystemId, TrainDatasetImagesId, DataElement.NbAugmentedImages,
                     DataElement.AugmentedImageSize, DataElement.OffsetXY, DataElement.AugRngInitValue);

      // Calculate features and store them in FeatureDataset.
      CalculateFeatures(SystemId, TrainDatasetImagesId, DevDatasetImagesId, &TrainDatasetFeaturesId,
                        &DevDatasetFeaturesId, &BlobContextId, DataElement.DataTrainPath,
                        DataElement.ChosenFeatures, &ListOfEnabeledFeatures, DataElement.MinBlobRadius,
                        DataElement.BinarizeImageThreshold);

      // Train TREE_ENSEMBLE context.
      Train(SystemId, &TrainContextId, &PredictContextId, TrainDatasetFeaturesId, DevDatasetFeaturesId,
            DataElement.DataTrainPath, DataElement.ChosenFeatures, DataElement.NumberOfTrainedTrees);

      // Predict using TREE_ENSEMBLE model.
      Predict(SystemId, DisplayId, PredictContextId, BlobContextId, DataElement.ChosenFeatures.size(),
              ListOfEnabeledFeatures, DataElement.ImagesRootPath, DataElement.ClassNames,
              DataElement.NumberOfPredictImages, DataElement.MinBlobRadius, DataElement.BinarizeImageThreshold);
         
      // Deselect the buffer on the display.
      MdispSelect(DisplayId, M_NULL);

      // Free buffer.
      MbufFree(AllClassesImage);
      }

   MosPrintf(MIL_TEXT("Press <Enter> to quit.\n\n"));
   MosGetch();

   // Free defaults.
   MdispFree(DisplayId);
   MsysFree(SystemId);
   MappFree(ApplicationId);
   }


//*****************************************************************************
// Example function definitions
//*****************************************************************************
 void PrepareDatasetImages(MIL_ID SystemId,
                           MIL_ID* DatasetImagesId,
                           const MIL_STRING& TrainRootPath,
                           const MIL_STRING& DataTrainPath,
                           const MIL_STRING& OriginalDataPath,
                           const MIL_STRING* ClassNames,
                           const MIL_INT NumberOfClasses,
                           const MIL_INT* ClassNbImages)
   {
    MosPrintf(MIL_TEXT("-------------------------------------------------\n"));
    MosPrintf(MIL_TEXT("Step 1 : Importing data... \n"));

    // If not already existing we will create the appropriate
    // DataForTrainPath folders structure.
    // If the structure is already existing, then we will remove previous
    // data to ensure repeatability
    PrepareDataForTrainFolder(TrainRootPath, DataTrainPath, ClassNames, NumberOfClasses);

    // We copy the original data to the DataForTrainPath folder to ensure we can
    // modify/pre-process this data later without affecting the original data.
    CopyOriginalDataToDataForTrainFolder(ClassNames, NumberOfClasses, ClassNbImages, OriginalDataPath, DataTrainPath);

    FillDatasetImages(SystemId, DatasetImagesId, DataTrainPath, ClassNames, NumberOfClasses);

    MosPrintf(MIL_TEXT("...completed.\n\n"));
    MosPrintf(MIL_TEXT("-------------------------------------------------\n"));
    MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
    MosGetch();
   }


 void SplitDataset(MIL_ID SystemId,
                   MIL_ID DatasetImagesId,
                   MIL_ID* TrainDatasetImagesId,
                   MIL_ID* DevDatasetImagesId)
    {
    MclassAlloc(SystemId, M_DATASET_IMAGES, M_DEFAULT, TrainDatasetImagesId);
    MclassAlloc(SystemId, M_DATASET_IMAGES, M_DEFAULT, DevDatasetImagesId);
    
    MclassSplitDataset(M_SPLIT_CONTEXT_FIXED_SEED, DatasetImagesId, *TrainDatasetImagesId, *DevDatasetImagesId, 70, M_NULL, M_DEFAULT);

    MclassFree(DatasetImagesId);
    }

void AugmentDataset(MIL_ID SystemId, MIL_ID TrainImagesDatasetId, const MIL_INT NbAugmentPerImage, const MIL_INT AugmentedImageSize, const MIL_INT OffsetXY, const MIL_INT AugRngInitValue)
   {
   MosPrintf(MIL_TEXT("Step 2 : Data augmentation...\n\n"));

   MIL_ID AugmentContextId = MimAlloc(SystemId, M_AUGMENTATION_CONTEXT, M_DEFAULT, M_NULL);
   MIL_ID AugmentResultId = MimAllocResult(SystemId, M_DEFAULT, M_AUGMENTATION_RESULT, M_NULL);

   // Seed the augmentation to ensure repeatability.
   MimControl(AugmentContextId, M_AUG_SEED_MODE, M_RNG_INIT_VALUE);
   MimControl(AugmentContextId, M_AUG_RNG_INIT_VALUE, AugRngInitValue);

   MimControl(AugmentContextId, M_AUG_TRANSLATION_X_OP, M_ENABLE);
   MimControl(AugmentContextId, M_AUG_TRANSLATION_X_OP_MAX, 10);
   MimControl(AugmentContextId, M_AUG_TRANSLATION_Y_OP, M_ENABLE);
   MimControl(AugmentContextId, M_AUG_TRANSLATION_Y_OP_MAX, 10);

   MimControl(AugmentContextId, M_AUG_SCALE_OP, M_ENABLE);
   MimControl(AugmentContextId, M_AUG_SCALE_OP_FACTOR_MIN, 0.8);
   MimControl(AugmentContextId, M_AUG_SCALE_OP_FACTOR_MAX, 1.2);

   MimControl(AugmentContextId, M_AUG_ROTATION_OP, M_ENABLE);
   MimControl(AugmentContextId, M_AUG_ROTATION_OP_ANGLE_DELTA, 45.0);

   MimControl(AugmentContextId, M_AUG_NOISE_SALT_PEPPER_OP, M_ENABLE);
   MimControl(AugmentContextId, M_AUG_NOISE_SALT_PEPPER_OP_DENSITY, M_DEFAULT);
   MimControl(AugmentContextId, M_AUG_NOISE_SALT_PEPPER_OP_DENSITY_DELTA, M_DEFAULT);

   MimControl(AugmentContextId, M_AUG_SMOOTH_GAUSSIAN_OP, M_ENABLE);
   MimControl(AugmentContextId, M_AUG_SMOOTH_GAUSSIAN_OP_STDDEV_MIN, 0.0);
   MimControl(AugmentContextId, M_AUG_SMOOTH_GAUSSIAN_OP_STDDEV_MAX, 1.0);

   MIL_INT NumOriginalImages = MclassInquire(TrainImagesDatasetId, M_DEFAULT, M_NUMBER_OF_ENTRIES, M_NULL);
   MIL_INT PosInAugmentDataset = NumOriginalImages;

   for(MIL_INT ImageIdx = 0; ImageIdx < NumOriginalImages; ImageIdx++)
      {
      MIL_STRING FilePath;
      MclassInquireEntry(TrainImagesDatasetId, ImageIdx, M_DEFAULT_KEY, M_DEFAULT, M_ENTRY_IMAGE_PATH, FilePath);
      MIL_INT GroundTruthIndex;
      MclassInquireEntry(TrainImagesDatasetId, ImageIdx, M_DEFAULT_KEY, M_REGION_INDEX(0), M_CLASS_INDEX_GROUND_TRUTH + M_TYPE_MIL_INT, &GroundTruthIndex);

      // Add the augmentations.
      MIL_ID OriginalImage = MbufRestore(FilePath, SystemId, M_NULL);

      MIL_ID OriginalImageResized = MbufAlloc2d(SystemId, AugmentedImageSize, AugmentedImageSize, 8 + M_UNSIGNED, M_IMAGE + M_DISP + M_PROC, M_NULL);
      MbufClear(OriginalImageResized, 0.0);
      MbufTransfer(OriginalImage, OriginalImageResized, 0, 0, M_DEFAULT, M_DEFAULT, M_DEFAULT, OffsetXY, OffsetXY, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_COPY, M_DEFAULT, M_NULL, M_NULL);

      MIL_ID AugmentedImage = MbufClone(OriginalImageResized, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_NULL);

      for(MIL_INT AugIndex = 0; AugIndex < NbAugmentPerImage; AugIndex++)
         {
         MbufClear(AugmentedImage, 0.0);
         MimAugment(AugmentContextId, OriginalImageResized, AugmentedImage, M_DEFAULT, M_DEFAULT);

         MIL_TEXT_CHAR Suffix[128];
         MosSprintf(Suffix, 128, MIL_TEXT("_Aug_%d"), AugIndex);

         MIL_STRING AugFileName = FilePath;
         std::size_t DotPos = AugFileName.rfind(MIL_TEXT("."));
         AugFileName.insert(DotPos, Suffix);
         MIL_STRING AugFileNameWithDir = GetExampleCurrentDirectory() + AugFileName;
         MbufSave(AugFileNameWithDir, AugmentedImage);

         // Add the augmented image.
         MclassControl(TrainImagesDatasetId, M_DEFAULT, M_ENTRY_ADD, M_DEFAULT);
         MclassControlEntry(TrainImagesDatasetId, PosInAugmentDataset, M_DEFAULT_KEY, M_REGION_INDEX(0), M_CLASS_INDEX_GROUND_TRUTH, GroundTruthIndex, M_NULL, M_DEFAULT);
         MclassControlEntry(TrainImagesDatasetId, PosInAugmentDataset, M_DEFAULT_KEY, M_DEFAULT, M_ENTRY_IMAGE_PATH, M_DEFAULT, AugFileName, M_DEFAULT);
         MclassControlEntry(TrainImagesDatasetId, PosInAugmentDataset, M_DEFAULT_KEY, M_DEFAULT, M_AUGMENTATION_SOURCE, ImageIdx, M_NULL, M_DEFAULT);

         MosPrintf(MIL_TEXT("%d of %d completed.\r"), PosInAugmentDataset - NumOriginalImages + 1, NumOriginalImages*NbAugmentPerImage );
         PosInAugmentDataset++;
         }

      MbufFree(OriginalImage);
      MbufFree(OriginalImageResized);
      MbufFree(AugmentedImage);
      }

   MimFree(AugmentResultId);
   MimFree(AugmentContextId);

   MosPrintf(MIL_TEXT("...completed.        \n\n"));
   MosPrintf(MIL_TEXT("-------------------------------------------------\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }


void CalculateFeatures(MIL_ID SystemId,
                       MIL_ID TrainDatasetImagesId,
                       MIL_ID DevDatasetImagesId,
                       MIL_ID* TrainDatasetFeaturesId,
                       MIL_ID* DevDatasetFeaturesId,
                       MIL_ID* BlobContextId,
                       const MIL_STRING&    DataTrainPath,
                       const std::vector<FeatureAndName>& ChosenFeatures,
                       std::vector<MIL_INT64>* ListOfEnabeledFeatures,
                       const MIL_INT MinBlobRadius,
                       const MIL_DOUBLE BinarizeImageThreshold)
   {
   MosPrintf(MIL_TEXT("Step 3 : Calculating features... \n\n"));

   // Allocate a blob context. 
   MblobAlloc(SystemId, M_DEFAULT, M_DEFAULT, BlobContextId);

   // Enable Features to be used for training.
   EnableFeatures(*BlobContextId, ChosenFeatures, ListOfEnabeledFeatures);

   // Calculate features for TrainDataset.
   CalculateFeaturesForDataset(SystemId,
                               TrainDatasetImagesId,
                               *BlobContextId,
                               *ListOfEnabeledFeatures,
                               TrainDatasetFeaturesId,
                               ChosenFeatures.size(),
                               MinBlobRadius,
                               BinarizeImageThreshold);

   // Calculate features for DevDataset.
   CalculateFeaturesForDataset(SystemId,
                               DevDatasetImagesId,
                               *BlobContextId,
                               *ListOfEnabeledFeatures,
                               DevDatasetFeaturesId,
                               ChosenFeatures.size(),
                               MinBlobRadius,
                               BinarizeImageThreshold);

   MosPrintf(MIL_TEXT("...completed.        \n\n"));

   // Export dataset features.
   MosPrintf(MIL_TEXT("Exported the train dataset entries in: "));
   MosPrintf(DataTrainPath.c_str());
   MosPrintf(MIL_TEXT("TrainDatasetFeatures.csv\n"));
   MclassExport(DataTrainPath + MIL_TEXT("TrainDatasetFeatures.csv"), M_FORMAT_CSV, *TrainDatasetFeaturesId, M_DEFAULT, M_ENTRIES, M_DEFAULT);

   MosPrintf(MIL_TEXT("Exported the dev dataset entries in: "));
   MosPrintf(DataTrainPath.c_str());
   MosPrintf(MIL_TEXT("DevDatasetFeatures.csv.\n\n"));
   MclassExport(DataTrainPath + MIL_TEXT("DevDatasetFeatures.csv"), M_FORMAT_CSV, *DevDatasetFeaturesId, M_DEFAULT, M_ENTRIES, M_DEFAULT);

   MosPrintf(MIL_TEXT("-------------------------------------------------\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

void Train(MIL_ID                               SystemId,
           MIL_ID*                              TrainContextId,
           MIL_ID*                              PredictContextId,
           MIL_ID                               TrainDatasetFeaturesId,
           MIL_ID                               DevDatasetFeaturesId,
           const MIL_STRING&                    DataTrainPath,
           const std::vector<FeatureAndName>&   ChosenFeatures,
           const MIL_INT                        NumberOfTrainedTrees)
   {
   MosPrintf(MIL_TEXT("Step 4 : Training... \n\n"));

   // Allocate a train context. 
   MclassAlloc(SystemId, M_TRAIN_TREE_ENSEMBLE, M_DEFAULT, TrainContextId);

   // Allocate a train context. 
   MIL_ID TrainResultId = MclassAllocResult(SystemId, M_TRAIN_TREE_ENSEMBLE_RESULT, M_DEFAULT, M_NULL);

   ControlTrainContext(*TrainContextId, NumberOfTrainedTrees);
   MclassPreprocess(*TrainContextId, M_DEFAULT);
   MclassTrain(*TrainContextId, M_NULL, TrainDatasetFeaturesId, DevDatasetFeaturesId, TrainResultId, M_DEFAULT);

   MosPrintf(MIL_TEXT("...completed.\n\n"));

   // Get train results
   MIL_INT NbTreesTrained(0);
   MclassGetResult(TrainResultId, M_DEFAULT, M_NUMBER_OF_TREES_TRAINED + M_TYPE_MIL_INT, &NbTreesTrained);
   
   MIL_DOUBLE TrainSetAccuracy(0), DevSetAccuracy(0), OOBAccuracy(0);
   MclassGetResult(TrainResultId, M_DEFAULT, M_TRAIN_DATASET_ACCURACY, &TrainSetAccuracy);
   MclassGetResult(TrainResultId, M_DEFAULT, M_DEV_DATASET_ACCURACY  , &DevSetAccuracy);
   MclassGetResult(TrainResultId, M_DEFAULT, M_OUT_OF_BAG_ACCURACY   , &OOBAccuracy);

   std::vector<MIL_DOUBLE> FeatureImportance;
   MclassGetResult(TrainResultId, M_DEFAULT, M_FEATURE_IMPORTANCE, FeatureImportance);

   // Save the training report in text file and the first tree in dot file.
   MosPrintf(MIL_TEXT("\nExported the training report in: "));
   MosPrintf(DataTrainPath.c_str());
   MosPrintf(MIL_TEXT("TrainReport.txt\n"));
   MosPrintf(MIL_TEXT("Exported the first tree in DOT format in: "));
   MosPrintf(DataTrainPath.c_str());
   MosPrintf(MIL_TEXT("TrainTree.dot\n\n"));
   MclassExport(DataTrainPath + MIL_TEXT("TrainReport.txt"), M_FORMAT_TXT, TrainResultId, M_DEFAULT, M_TRAIN_REPORT, M_DEFAULT);
   MclassExport(DataTrainPath + MIL_TEXT("TrainTree.dot"), M_FORMAT_DOT, TrainResultId, 0, M_TRAIN_TREE, M_DEFAULT);

   MclassAlloc(SystemId, M_CLASSIFIER_TREE_ENSEMBLE, M_DEFAULT, PredictContextId);
   MclassCopyResult(TrainResultId, M_DEFAULT, *PredictContextId, M_DEFAULT, M_TRAINED_CLASSIFIER, M_DEFAULT);
   MclassCopyResult(TrainResultId, M_DEFAULT, *PredictContextId, M_DEFAULT, M_TRAINED_CLASSIFIER, M_DEFAULT);
   MclassPreprocess(*PredictContextId, M_DEFAULT);

   MosPrintf(MIL_TEXT("\n*************** Train results *************** \n\n"));

   MosPrintf(MIL_TEXT("\t Number of trained trees = %d\n"), NbTreesTrained);
   MosPrintf(MIL_TEXT("\t Train accuracy      = %0.2f %%\n"), TrainSetAccuracy);
   MosPrintf(MIL_TEXT("\t Dev accuracy        = %0.2f %%\n"), DevSetAccuracy);
   MosPrintf(MIL_TEXT("\t Out Of Bag accuracy = %0.2f %%\n"), OOBAccuracy);

   MosPrintf(MIL_TEXT("\n\t Feature importance :\n\n"), OOBAccuracy);
   for(MIL_INT i = 0; i < (MIL_INT)FeatureImportance.size(); i++)
      MosPrintf(MIL_TEXT("\t  [%d] %s\t%0.2f %% \n"), (int)i, ChosenFeatures[i].FeatureName.c_str(), FeatureImportance[i] * 100);
      
   MosPrintf(MIL_TEXT("\n-------------------------------------------------\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MclassFree(TrainDatasetFeaturesId);
   MclassFree(DevDatasetFeaturesId);
   MclassFree(TrainResultId);
   MclassFree(*TrainContextId);
   }


void Predict(MIL_ID SystemId,
             MIL_ID DisplayId,
             MIL_ID PredictContextId,
             MIL_ID BlobContextId,
             MIL_INT NumberOfFeatures,
             const std::vector<MIL_INT64>& ListOfEnabeledFeatures,
             const MIL_STRING& ImageRootPath,
             const MIL_STRING* ClassNames,
             const MIL_INT NumberOfPredictImages,
             const MIL_INT MinBlobRadius,
             const MIL_DOUBLE BinarizeImageThreshold)
   {
   MosPrintf(MIL_TEXT("Step 5 : Predicting... \n\n"));

   for(MIL_INT ImageIdx = 0; ImageIdx < NumberOfPredictImages; ImageIdx++)
      {
      MosPrintf(MIL_TEXT("Showing prediction results in green for TestImage_%d.mim.\n\n"), ImageIdx);

      // Restore the image used for prediction. Image names are TestImage_0.mim, TestImage_1.mim, ...
      MIL_TEXT_CHAR ImageName[512];
      MosSprintf(ImageName, 512, MIL_TEXT("%s/%s%d.mim"), ImageRootPath.c_str(), MIL_TEXT("TestImage_"), ImageIdx);
      MIL_ID PredictImageId = MbufRestore(ImageName, SystemId, M_NULL);

      // Display the buffer. 
      MdispSelect(DisplayId, PredictImageId);

      // Predict the class label for all blobs found in the image, and show the results on the display.
      PredictOnImageAndDisplayResults(SystemId, DisplayId, PredictContextId, PredictImageId, BlobContextId, NumberOfFeatures,
                                      ListOfEnabeledFeatures, ClassNames, MinBlobRadius, BinarizeImageThreshold);

      // Deselect the buffer from the display.
      MdispSelect(DisplayId, M_NULL);

      // Free buffer.
      MbufFree(PredictImageId);
      }

   MosPrintf(MIL_TEXT("...completed.\n\n"));
   MosPrintf(MIL_TEXT("-------------------------------------------------\n"));

   // Free allocations.
   MblobFree(BlobContextId);
   MclassFree(PredictContextId);
   }

//*****************************************************************************
// Utility sub-functions definitions
//*****************************************************************************

MIL_ID CreateImageOfAllClasses(MIL_ID SystemId, const MIL_STRING* ClassSample, const MIL_INT NumberOfClasses, const MIL_STRING* ClassNames)
   {
   MIL_INT MaxSizeX = MIL_INT_MIN;
   MIL_INT MaxSizeY = MIL_INT_MIN;

   std::vector<MIL_ID> SamplesToDisplay;
   for(MIL_INT i = 0; i < NumberOfClasses; i++)
      {
      SamplesToDisplay.push_back(MbufRestore(ClassSample[i], SystemId, M_NULL));
      MIL_INT SizeX = MbufInquire(SamplesToDisplay.back(), M_SIZE_X, M_NULL);
      MIL_INT SizeY = MbufInquire(SamplesToDisplay.back(), M_SIZE_Y, M_NULL);

      MaxSizeX = std::max<MIL_INT>(SizeX, MaxSizeX);
      MaxSizeY = std::max<MIL_INT>(SizeY, MaxSizeY);
      }

   MIL_ID AllClassesImage = MbufAllocColor(SystemId, 3, MaxSizeX * NumberOfClasses / 2, 2 * MaxSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_NULL);
   MbufClear(AllClassesImage, 0.0);

   MIL_ID GraContext = MgraAlloc(SystemId, M_NULL);
   MgraColor(GraContext, M_COLOR_LIGHT_BLUE);

   MIL_INT CurXOffset = 0;
   MIL_INT CurYOffset = 0;
   for(MIL_INT ImageIndex = 0; ImageIndex < NumberOfClasses; ImageIndex++)
      {
      MIL_INT SizeX = MbufInquire(SamplesToDisplay[ImageIndex], M_SIZE_X, M_NULL);
      MIL_INT SizeY = MbufInquire(SamplesToDisplay[ImageIndex], M_SIZE_Y, M_NULL);

      MbufCopyClip(SamplesToDisplay[ImageIndex], AllClassesImage, CurXOffset, CurYOffset);
      MgraRect(GraContext, AllClassesImage, CurXOffset, CurYOffset, CurXOffset + SizeX - 1, CurYOffset + SizeY - 1);
      MgraText(GraContext, AllClassesImage, CurXOffset + 5, CurYOffset + SizeY - 20, ClassNames[ImageIndex]);

      CurXOffset = (ImageIndex == NumberOfClasses / 2 - 1) ? 0 : CurXOffset + SizeX;
      CurYOffset = (ImageIndex < NumberOfClasses / 2 - 1)  ? 0 : MaxSizeY;

      MbufFree(SamplesToDisplay[ImageIndex]);
      }

   MgraFree(GraContext);

   return AllClassesImage;
   }

void FillDatasetImages(MIL_ID SystemId, MIL_ID* DatasetImagesId, const MIL_STRING& DataTrainPath, const MIL_STRING* ClassName, const MIL_INT NumberOfClasses)
   {
   MclassAlloc(SystemId, M_DATASET_IMAGES, M_DEFAULT, DatasetImagesId);
   MclassControl(*DatasetImagesId, M_CONTEXT, M_ROOT_PATH, GetExampleCurrentDirectory());

   MosPrintf(MIL_TEXT("\n   Adding images to ImageDataset ...\n\n"));
   for(MIL_INT i = 0; i < NumberOfClasses; i++)
      {
      // Add class description to the dataset before adding entries to it.
      MclassControl(*DatasetImagesId, M_DEFAULT, M_CLASS_ADD, ClassName[i]);
      AddClassToDataset(i, DataTrainPath, ClassName[i], *DatasetImagesId);
      }
   }

void AddClassToDataset(MIL_INT ClassIndex, const MIL_STRING& DataTrainPath, const MIL_STRING& ShapeName, MIL_ID Dataset)
   {
   MIL_INT NbEntries;
   MclassInquire(Dataset, M_DEFAULT, M_NUMBER_OF_ENTRIES + M_TYPE_MIL_INT, &NbEntries);

   MIL_STRING FolderName = DataTrainPath + ShapeName + MIL_TEXT("/");

   std::vector<MIL_STRING> FilesInFolder;
   ListFilesInFolder(FolderName, FilesInFolder);

   MIL_INT CurImageIndex = 0;
   for(const auto& File : FilesInFolder)
      {
      MclassControl(Dataset, M_DEFAULT, M_ENTRY_ADD, M_DEFAULT);
      MclassControlEntry(Dataset, NbEntries + CurImageIndex, M_DEFAULT_KEY, M_REGION_INDEX(0), M_CLASS_INDEX_GROUND_TRUTH, ClassIndex, M_NULL, M_DEFAULT);
      MclassControlEntry(Dataset, NbEntries + CurImageIndex, M_DEFAULT_KEY, M_DEFAULT, M_ENTRY_IMAGE_PATH, M_DEFAULT, File, M_DEFAULT);
      CurImageIndex++;
      }
   }


void CalculateFeaturesForDataset(MIL_ID SystemId,
                                 MIL_ID DatasetImagesId,
                                 MIL_ID BlobContextId,
                                 const std::vector<MIL_INT64>& ListOfEnabeledFeatures,
                                 MIL_ID* DatasetFeaturesId,
                                 const MIL_INT NumberOfFeatures,
                                 const MIL_INT MinBlobRadius,
                                 const MIL_DOUBLE BinarizeImageThreshold)
   {
   // Allocate a FeatureDataset. 
   MclassAlloc(SystemId, M_DATASET_FEATURES, M_DEFAULT, DatasetFeaturesId);

   // Copy class definitions from ImageDataset to FeatureDataset.
   MclassCopy(DatasetImagesId, M_DEFAULT, *DatasetFeaturesId, M_DEFAULT, M_CLASS_DEFINITIONS, M_DEFAULT);
   MclassCopy(DatasetImagesId, M_DEFAULT, *DatasetFeaturesId, M_DEFAULT, M_AUTHORS, M_DEFAULT);

   // Control the features.
   MIL_ID MilBlobResultId = MblobAllocResult(SystemId, M_DEFAULT, M_DEFAULT, M_NULL);
   MIL_INT NumberOfImages = MclassInquire(DatasetImagesId, M_DEFAULT, M_NUMBER_OF_ENTRIES, M_NULL);

   for(MIL_INT ImageIdx = 0; ImageIdx < NumberOfImages; ImageIdx++)
      {
      // Get Image from ImageDataset.
      MIL_STRING FilePath;
      MIL_INT GroundTruthIndex, AugmentationSource;
      MclassInquireEntry(DatasetImagesId, ImageIdx, M_DEFAULT_KEY, M_DEFAULT, M_ENTRY_IMAGE_PATH, FilePath);
      MclassInquireEntry(DatasetImagesId, ImageIdx, M_DEFAULT_KEY, M_REGION_INDEX(0), M_CLASS_INDEX_GROUND_TRUTH + M_TYPE_MIL_INT, &GroundTruthIndex);
      MclassInquireEntry(DatasetImagesId, ImageIdx, M_DEFAULT_KEY, M_DEFAULT, M_AUGMENTATION_SOURCE + M_TYPE_MIL_INT, &AugmentationSource);

      MIL_ID ImageId = MbufRestore(FilePath, SystemId, M_NULL);

      // Use a binary image buffer for fast processing. 
      MIL_ID BinImageId = ProcessImage(SystemId, ImageId, MinBlobRadius, BinarizeImageThreshold);
      MblobCalculate(BlobContextId, BinImageId, M_NULL, MilBlobResultId);

      MIL_INT NumTotalBlobs = 0;
      std::vector<MIL_DOUBLE> Features;
      Features.reserve(NumberOfFeatures);

      MblobGetResult(MilBlobResultId, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &NumTotalBlobs);

      MIL_INT BigBlobIndex = NumTotalBlobs - 1;
      AddFeaturesToVector(MilBlobResultId, BigBlobIndex, ListOfEnabeledFeatures, &Features);

      // Fill FeaturesDataset. 
      MclassControl(*DatasetFeaturesId, M_DEFAULT, M_ENTRY_ADD, M_DEFAULT);
      MclassControlEntry(*DatasetFeaturesId, ImageIdx, M_DEFAULT_KEY, M_DEFAULT, M_RAW_DATA, M_DEFAULT, Features, M_NULL);
      MclassControlEntry(*DatasetFeaturesId, ImageIdx, M_DEFAULT_KEY, M_DEFAULT, M_CLASS_INDEX_GROUND_TRUTH, GroundTruthIndex, M_NULL, M_DEFAULT);
      MclassControlEntry(*DatasetFeaturesId, ImageIdx, M_DEFAULT_KEY, M_DEFAULT, M_AUGMENTATION_SOURCE, AugmentationSource, M_NULL, M_DEFAULT);

      MosPrintf(MIL_TEXT("%d of %d completed.\r"), ImageIdx, NumberOfImages);

      MbufFree(BinImageId);
      MbufFree(ImageId);
      }

   MblobFree(MilBlobResultId);
   MclassFree(DatasetImagesId);
   }


MIL_ID ProcessImage(MIL_ID SystemId, MIL_ID ImageId, const MIL_INT MinBlobRadius, const MIL_DOUBLE BinarizeImageThreshold)
   {
   MIL_ID  MilBinImage;    // Binary image buffer identifier. 

   MIL_INT SizeX,          // Size X of the source buffer.    
           SizeY;          // Size Y of the source buffer.    

   // Allocate a binary image buffer for fast processing. 
   MbufInquire(ImageId, M_SIZE_X, &SizeX);
   MbufInquire(ImageId, M_SIZE_Y, &SizeY);
   MbufAlloc2d(SystemId, SizeX, SizeY, 1 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &MilBinImage);

   // Binarize image.
   MimBinarize(ImageId, MilBinImage, M_FIXED + M_GREATER_OR_EQUAL, BinarizeImageThreshold, M_NULL);

   // Remove small particles and then remove small holes.
   MimOpen(MilBinImage, MilBinImage, MinBlobRadius, M_BINARY);
   MimClose(MilBinImage, MilBinImage, MinBlobRadius, M_BINARY);

   return MilBinImage;
   }


void ControlTrainContext(MIL_ID TrainContextId, const MIL_INT NumberOfTrainedTrees)
   {
   MclassControl(TrainContextId, M_DEFAULT, M_NUMBER_OF_TREES, NumberOfTrainedTrees);
   MclassControl(TrainContextId, M_DEFAULT, M_SEED_VALUE, M_DEFAULT);
   MclassControl(TrainContextId, M_DEFAULT, M_FEATURE_IMPORTANCE_MODE, M_MEAN_DECREASE_IMPURITY);
   MclassControl(TrainContextId, M_DEFAULT, M_BOOTSTRAP, M_WITH_REPLACEMENT);
   }


void DisplayPredictedResults(MIL_ID GraList,
                             MIL_INT BlobResultId,
                             MIL_ID BlobIndex,
                             MIL_INT PredictedClassIndex,
                             const MIL_STRING* ClassNames)
   {
   MIL_DOUBLE XMin, XMax, YMax;
   MblobGetResult(BlobResultId, M_BLOB_INDEX(BlobIndex), M_BOX_X_MIN + M_BINARY, &XMin);
   MblobGetResult(BlobResultId, M_BLOB_INDEX(BlobIndex), M_BOX_X_MAX + M_BINARY, &XMax);
   MblobGetResult(BlobResultId, M_BLOB_INDEX(BlobIndex), M_BOX_Y_MAX + M_BINARY, &YMax);

   MblobDraw(M_DEFAULT, BlobResultId, GraList, M_DRAW_BOX, M_BLOB_INDEX(BlobIndex), M_DEFAULT);
   MblobDraw(M_DEFAULT, BlobResultId, GraList, M_DRAW_BOX_CENTER, M_BLOB_INDEX(BlobIndex), M_DEFAULT);
   MgraText(M_DEFAULT, GraList, (XMin + XMax) / 2 - 20, YMax + 10, ClassNames[PredictedClassIndex]);
   }


void EnableFeatures(MIL_ID BlobContextId,
                    const std::vector<FeatureAndName>& ChosenFeatures,
                    std::vector<MIL_INT64>*     ListOfEnabeledFeatures)
   {
   ListOfEnabeledFeatures->reserve(ChosenFeatures.size());

   // Use M_AREA to sort blobs, then only the first blob will represent a valid shape in each image.
   MblobControl(BlobContextId, M_SORT1, M_AREA);
   MblobControl(BlobContextId, M_IDENTIFIER_TYPE, M_BINARY);
   MblobControl(BlobContextId, M_BLOB_IDENTIFICATION_MODE, M_INDIVIDUAL);

   // Enable a large number of ferets to have more precise results.
   MblobControl(BlobContextId, M_NUMBER_OF_FERETS, 90);

   // Only use features that are robust to translation and rotation
   for(MIL_UINT FeatureIndex = 0; FeatureIndex < ChosenFeatures.size(); FeatureIndex++)
      {
      EnableFeature(BlobContextId, ChosenFeatures[FeatureIndex].Feature, ListOfEnabeledFeatures);
      }
   }

void EnableFeature(MIL_ID BlobContextId,
                   MIL_INT64 ResultType,
                   std::vector<MIL_INT64>* ListOfEnabeledFeatures)
   {
   if(ResultType == M_CONVEX_HULL_FILL_RATIO)
      {
      MblobControl(BlobContextId, M_CONVEX_HULL, M_ENABLE);
      ListOfEnabeledFeatures->push_back(ResultType);
      return;
      }

   if(ResultType == M_FERET_ELONGATION)
      {
      MblobControl(BlobContextId, M_ELONGATION, M_ENABLE);
      ListOfEnabeledFeatures->push_back(ResultType);
      return;
      }

   if((ResultType == M_MOMENT_CENTRAL_X1_Y2) ||
      (ResultType == M_MOMENT_CENTRAL_X2_Y1) ||
      (ResultType == M_MOMENT_CENTRAL_X3_Y0) ||
      (ResultType == M_MOMENT_CENTRAL_X0_Y3) ||
      (ResultType == M_MOMENT_HU_1) ||
      (ResultType == M_MOMENT_HU_2) ||
      (ResultType == M_MOMENT_HU_3) ||
      (ResultType == M_MOMENT_HU_4) ||
      (ResultType == M_MOMENT_HU_5) ||
      (ResultType == M_MOMENT_HU_6) ||
      (ResultType == M_MOMENT_HU_7)
      )
      {
      MblobControl(BlobContextId, M_MOMENT_THIRD_ORDER, M_ENABLE);
      ListOfEnabeledFeatures->push_back(ResultType);
      return;
      }

   MblobControl(BlobContextId, ResultType, M_ENABLE);
   ListOfEnabeledFeatures->push_back(ResultType);
   }


void AddFeaturesToVector(MIL_ID BlobResultId,
                         MIL_INT BlobIndex,
                         const std::vector<MIL_INT64>& ListOfEnabeledFeatures,
                         std::vector<MIL_DOUBLE>* FeaturesVector)
   {
   for(const auto& ResultType : ListOfEnabeledFeatures)
      {
      MIL_DOUBLE FeatureVal;
      MblobGetResult(BlobResultId, M_BLOB_INDEX(BlobIndex), ResultType + M_BINARY + M_TYPE_MIL_DOUBLE, &FeatureVal);
      FeaturesVector->push_back(FeatureVal);
      }
   }


MIL_STRING GetExampleCurrentDirectory()
   {
   DWORD CurDirStrSize = GetCurrentDirectory(0, NULL) + 1;

   std::vector<MIL_TEXT_CHAR> vCurDir(CurDirStrSize, 0);
   GetCurrentDirectory(CurDirStrSize, (LPTSTR)&vCurDir[0]);
   MIL_STRING SS = MIL_TEXT("\\");
   MIL_STRING sRet = &vCurDir[0] + SS;
   return sRet;
   }


void PrepareDataForTrainFolder(const MIL_STRING& TrainRootPath, const MIL_STRING& DataForTrainPath, const MIL_STRING* ClassNames, const MIL_INT NumberOfClasses)
   {
   MIL_INT FileExists;
   MappFileOperation(M_DEFAULT, DataForTrainPath, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FileExists);

   if(FileExists != M_YES)
      {
      MosPrintf(MIL_TEXT("\n   Creating the %s folder for TrainData...\n"), DataForTrainPath.c_str());

      // Create DataForTrainPath folder since it does not exist.
      MappFileOperation(M_DEFAULT, TrainRootPath, M_NULL, M_NULL, M_FILE_MAKE_DIR, M_DEFAULT, M_NULL);
      MappFileOperation(M_DEFAULT, DataForTrainPath, M_NULL, M_NULL, M_FILE_MAKE_DIR, M_DEFAULT, M_NULL);
      for(MIL_INT i = 0; i < NumberOfClasses; i++)
         {
         // Create one folder for each class name.
         MappFileOperation(M_DEFAULT, DataForTrainPath + ClassNames[i], M_NULL, M_NULL, M_FILE_MAKE_DIR, M_DEFAULT, M_NULL);
         }
      }
   else
      {
      // Delete Train Files
      DeleteFileIfExisiting(MIL_TEXT("TrainDatasetFeatures.csv"));
      DeleteFileIfExisiting(MIL_TEXT("DevDatasetFeatures.csv"));
      DeleteFileIfExisiting(MIL_TEXT("TrainReport.txt"));
      DeleteFileIfExisiting(MIL_TEXT("TrainTree.dot"));

      // If DataForTrainPath folder is existing, delete files already in there
      // Create the folder if not existing.
      MosPrintf(MIL_TEXT("\n   Deleting files in the %s folder to ensure example repeatability...\n"), DataForTrainPath.c_str());

      for(MIL_INT i = 0; i < NumberOfClasses; i++)
         {
         MappFileOperation(M_DEFAULT, DataForTrainPath + ClassNames[i], M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FileExists);
         if(FileExists)
            DeleteFilesInFolder(DataForTrainPath + ClassNames[i] + MIL_TEXT("/"));
         else
            MappFileOperation(M_DEFAULT, DataForTrainPath + ClassNames[i], M_NULL, M_NULL, M_FILE_MAKE_DIR, M_DEFAULT, M_NULL);
         }
      }
   }


void CopyOriginalDataToDataForTrainFolder(const MIL_STRING* ClassName,
                                          const MIL_INT  NumberOfClasses,
                                          const MIL_INT* ClassNbImages,
                                          const MIL_STRING& OriginalDataPath,
                                          const MIL_STRING& DataForTrainPath)
   {
   MosPrintf(MIL_TEXT("\n   Copying original train data from %s to %s ...\n"), OriginalDataPath.c_str(), DataForTrainPath.c_str());
   for(MIL_INT ClassIndex = 0; ClassIndex < NumberOfClasses; ClassIndex++)
      {
      MIL_INT NbImages = ClassNbImages[ClassIndex];
      // Image names are 0.mim, 1.mim, ..., (NbImages-1).mim
      for(MIL_INT i = 0; i < NbImages; i++)
         {
         MIL_TEXT_CHAR OriginalFileName[512];
         MosSprintf(OriginalFileName, 512, MIL_TEXT("%s%s/%d.mim"), OriginalDataPath.c_str(), ClassName[ClassIndex].c_str(), i);
         MIL_TEXT_CHAR DestFileName[512];
         MosSprintf(DestFileName, 512, MIL_TEXT("%s%s/%d.mim"), DataForTrainPath.c_str(), ClassName[ClassIndex].c_str(), i);
         MappFileOperation(M_DEFAULT, OriginalFileName, M_DEFAULT, DestFileName, M_FILE_COPY, M_DEFAULT, M_NULL);
         }
      }
   }


void PredictOnImageAndDisplayResults(MIL_ID SystemId,
                                     MIL_ID DisplayId,
                                     MIL_ID PredictContextId,
                                     MIL_ID PredictImageId,
                                     MIL_ID BlobContextId,
                                     const MIL_INT NumberOfFeatures,
                                     const std::vector<MIL_INT64>& ListOfEnabeledFeatures,
                                     const MIL_STRING* ClassNames,
                                     const MIL_INT MinBlobRadius,
                                     const MIL_DOUBLE BinarizeImageThreshold)
   {
   // Allocate a graphic list to hold the subpixel annotations to draw. 
   MIL_UNIQUE_GRA_ID GraList = MgraAllocList(SystemId, M_DEFAULT, M_UNIQUE_ID);
   MgraColor(M_DEFAULT, M_COLOR_GREEN);

   // Associate the graphic list to the display. 
   MdispControl(DisplayId, M_ASSOCIATED_GRAPHIC_LIST_ID, GraList);

   // Allocate blob result and predict results.
   MIL_UNIQUE_BLOB_ID BlobResultId = MblobAllocResult(SystemId, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_CLASS_ID PredictResultId = MclassAllocResult(SystemId, M_PREDICT_TREE_ENSEMBLE_RESULT, M_DEFAULT, M_UNIQUE_ID);

   // Process the image.
   MIL_ID BinPredictImageId = ProcessImage(SystemId, PredictImageId, MinBlobRadius, BinarizeImageThreshold);

   // Calculate selected features for each blob. 
   MblobCalculate(BlobContextId, BinPredictImageId, M_NULL, BlobResultId);

   // Get the total number of selected blobs. 
   MIL_INT NumberOfBlobs;
   MblobGetResult(BlobResultId, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &NumberOfBlobs);

   MIL_UNIQUE_BUF_ID MilDataArray = MbufAlloc1d(SystemId, NumberOfFeatures, 32 + M_FLOAT, M_ARRAY, M_UNIQUE_ID);

   for(MIL_INT BlobIndex = 0; BlobIndex < NumberOfBlobs; BlobIndex++)
      {
      std::vector<MIL_DOUBLE> Features;
      Features.reserve(NumberOfFeatures);
      AddFeaturesToVector(BlobResultId, BlobIndex, ListOfEnabeledFeatures, &Features);

      // Convert from M_TYPE_MIL_DOUBLE to M_TYPE_MIL_FLOAT and put predict data in a buffer
      std::vector<MIL_FLOAT> FeaturesF(NumberOfFeatures, 0.0);
      for(MIL_INT i = 0; i < NumberOfFeatures; i++)
         FeaturesF[i] = static_cast<MIL_FLOAT>(Features[i]);

      MbufPut1d(MilDataArray, 0, NumberOfFeatures, FeaturesF);

      MclassPredict(PredictContextId, MilDataArray, PredictResultId, M_DEFAULT);

      MIL_INT PredictedLabel;
      MIL_DOUBLE PredictScore;
      MclassGetResult(PredictResultId, M_DEFAULT, M_BEST_CLASS_INDEX + M_TYPE_MIL_INT, &PredictedLabel);
      MclassGetResult(PredictResultId, M_DEFAULT, M_BEST_CLASS_SCORE, &PredictScore);
      DisplayPredictedResults(GraList, BlobResultId, BlobIndex, PredictedLabel, ClassNames);
      }
   MbufFree(BinPredictImageId);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }


void DeleteFiles(const std::vector<MIL_STRING>& Files)
   {
   for(const auto& FileName : Files)
      {
      MappFileOperation(M_DEFAULT, FileName, M_NULL, M_NULL, M_FILE_DELETE, M_DEFAULT, M_NULL);
      }
   }


void ListFilesInFolder(const MIL_STRING& FolderName, std::vector<MIL_STRING>& FilesInFolder)
   {
   MIL_STRING FileToSearch = FolderName;
   FileToSearch += MIL_TEXT("*.*");

   WIN32_FIND_DATA FindFileData;
   HANDLE hFind;
   hFind = FindFirstFile(FileToSearch.c_str(), &FindFileData);

   if(hFind == INVALID_HANDLE_VALUE)
      {
      MosPrintf(MIL_TEXT("FindFirstFile failed (%d)\n"), GetLastError());
      return;
      }

   do
      {
      if(!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
         {
         FilesInFolder.push_back(FolderName + FindFileData.cFileName);
         }
      } while(FindNextFile(hFind, &FindFileData) != 0);

      FindClose(hFind);
   }

void DeleteFileIfExisiting(MIL_STRING FileName)
   {
   MIL_INT FileExists;
   MappFileOperation(M_DEFAULT, FileName, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FileExists);

   if(FileExists == M_YES)
      {
      MappFileOperation(M_DEFAULT, FileName, M_NULL, M_NULL, M_FILE_DELETE, M_DEFAULT, M_NULL);
      }
   }

void DeleteFilesInFolder(const MIL_STRING& FolderName)
   {
   std::vector<MIL_STRING> FilesInFolder;
   ListFilesInFolder(FolderName, FilesInFolder);
   DeleteFiles(FilesInFolder);
   }
