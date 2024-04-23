//***************************************************************************************/
//
// File name: VolumeSourceInfo.h
//
// Synopsis:  Utility header that contains the structures describing the source
//            data information for the CalculateVolumeDiagnostic example.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/

#ifndef VOLUME_SOURCE_INFO_H
#define VOLUME_SOURCE_INFO_H

enum ReferenceType
   {
   eNone = 0,
   eXYPlane,
   eSourceFile
   };

// Interface of any source data combinations.
struct SMilSource
   {
   virtual ~SMilSource() {};
   virtual MIL_ID GetSource() const = 0;
   virtual MIL_ID GetReference() const = 0;
   };

// Template implementation of all source data combinations.
template <class REF_ID>
struct SMilSourceData: public SMilSource
   {
   SMilSourceData(MIL_UNIQUE_BUF_ID&& Source, REF_ID&& MilReference)
      {
      m_Source = std::move(Source);
      m_Reference = std::move(MilReference);
      }

   virtual MIL_ID GetSource() const { return m_Source; }
   virtual MIL_ID GetReference() const { return m_Reference; }
   MIL_UNIQUE_BUF_ID m_Source;
   REF_ID m_Reference;
   };

// Structure that describes the source data. Contains a factory that
// creates and restores the source data based on the description.
struct SSourceDataInfo
   {
   std::unique_ptr<SMilSource> MakeMilSource(MIL_ID MilSystem, MIL_STRING ExamplePath) const
      {
      MIL_UNIQUE_BUF_ID MilSourceBuf;
      auto SourceFileName = ExamplePath + SourceName + MIL_TEXT("_Source") + SourceExt;
      if(SourceExt == MIL_TEXT(".ply") || SourceExt == MIL_TEXT(".mbufc"))
         MilSourceBuf = MbufRestore(SourceFileName, MilSystem, M_UNIQUE_ID);
      else if(SourceExt == MIL_TEXT(".mim"))
         MilSourceBuf = MbufImport(SourceFileName, M_MIL_TIFF + M_WITH_CALIBRATION, M_RESTORE, MilSystem, M_UNIQUE_ID);
      else
         {
         MosPrintf(MIL_TEXT("Invalid Source extension!\n"));
         exit(0);
         }

      std::unique_ptr<SMilSource> MilSource;
      auto RefName = ExamplePath + SourceName + MIL_TEXT("_Reference") + ReferenceExt;
      if(Reference == eNone)
         MilSource.reset(new SMilSourceData<MIL_INT>(std::move(MilSourceBuf), 0));
      else if(Reference == eXYPlane)
         MilSource.reset(new SMilSourceData<MIL_INT>(std::move(MilSourceBuf), M_XY_PLANE));
      else if(ReferenceExt == MIL_TEXT(".mim"))
         {
         auto MilReference = MbufImport(RefName, M_MIL_TIFF + M_WITH_CALIBRATION, M_RESTORE, MilSystem, M_UNIQUE_ID);
         MilSource.reset(new SMilSourceData<MIL_UNIQUE_BUF_ID>(std::move(MilSourceBuf), std::move(MilReference)));
         }
      else if(ReferenceExt == MIL_TEXT(".m3dgeo"))
         {
         auto MilReference = M3dgeoRestore(RefName, MilSystem, M_DEFAULT, M_UNIQUE_ID);
         MilSource.reset(new SMilSourceData<MIL_UNIQUE_3DGEO_ID>(std::move(MilSourceBuf), std::move(MilReference)));
         }
      else
         {
         MosPrintf(MIL_TEXT("Invalid reference source definition!\n"));
         exit(0);
         }

      return MilSource;
      }

   bool IsReference3dGeo() const { return Reference == eSourceFile && ReferenceExt == MIL_TEXT(".m3dgeo"); }
   bool IsReferencePlane() const { return Reference == eXYPlane || IsReference3dGeo(); }

   ReferenceType Reference;
   MIL_STRING    SourceName;
   MIL_STRING    SourceExt;
   MIL_STRING    ReferenceExt;
   MIL_INT       DefaultView;
   };

#endif // VOLUME_SOURCE_INFO_H
