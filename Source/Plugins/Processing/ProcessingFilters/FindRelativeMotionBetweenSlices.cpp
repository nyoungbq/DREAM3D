/* ============================================================================
 * Copyright (c) 2009-2016 BlueQuartz Software, LLC
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of BlueQuartz Software, the US Air Force, nor the names of its
 * contributors may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The code contained herein was partially funded by the following contracts:
 *    United States Air Force Prime Contract FA8650-07-D-5800
 *    United States Air Force Prime Contract FA8650-10-D-5210
 *    United States Prime Contract Navy N00173-07-C-2068
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "FindRelativeMotionBetweenSlices.h"

#ifdef SIMPL_USE_PARALLEL_ALGORITHMS
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <tbb/partitioner.h>
#endif

#include <QtCore/QTextStream>

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/Common/TemplateHelpers.h"
#include "SIMPLib/DataContainers/DataContainer.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/ChoiceFilterParameter.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/IntFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedPathCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"
#include "SIMPLib/Math/MatrixMath.h"

#include "Processing/ProcessingConstants.h"
#include "Processing/ProcessingVersion.h"

/**
 * @brief The CalcRelativeMotion class implements a templated threaded algorithm for
 * determining the relative motion between a series of slices through a 3D volume.
 */
template <typename T>
class CalcRelativeMotion
{

public:
  CalcRelativeMotion(T* data, float* motionDir, int32_t* patchPoints, int32_t* searchPoints, bool* validPoints, size_t numPP, size_t numSP)
  : m_Data(data)
  , m_MotionDirection(motionDir)
  , m_PatchPoints(patchPoints)
  , m_SearchPoints(searchPoints)
  , m_ValidPoints(validPoints)
  , m_NumPatchPoints(numPP)
  , m_NumSearchPoints(numSP)
  {
  }
  virtual ~CalcRelativeMotion() = default;

  void convert(size_t start, size_t end) const
  {
    int32_t patchPoint = 0, comparePoint = 0;
    float val = 0.0f, minVal = 0.0f;
    for(size_t i = start; i < end; i++)
    {
      if(m_ValidPoints[i] == true)
      {
        minVal = std::numeric_limits<float>::max();
        for(size_t j = 0; j < m_NumSearchPoints; j++)
        {
          val = 0;
          for(size_t k = 0; k < m_NumPatchPoints; k++)
          {
            patchPoint = i + m_PatchPoints[k];
            comparePoint = patchPoint + m_SearchPoints[4 * j];
            val += float((m_Data[patchPoint] - m_Data[comparePoint])) * float((m_Data[patchPoint] - m_Data[comparePoint]));
          }
          if(val < minVal)
          {
            minVal = val;
            m_MotionDirection[3 * i + 0] = m_SearchPoints[4 * j + 1];
            m_MotionDirection[3 * i + 1] = m_SearchPoints[4 * j + 2];
            m_MotionDirection[3 * i + 2] = m_SearchPoints[4 * j + 3];
          }
        }
      }
    }
  }

#ifdef SIMPL_USE_PARALLEL_ALGORITHMS
  void operator()(const tbb::blocked_range<size_t>& r) const
  {
    convert(r.begin(), r.end());
  }
#endif
private:
  T* m_Data;
  float* m_MotionDirection;
  int32_t* m_PatchPoints;
  int32_t* m_SearchPoints;
  bool* m_ValidPoints;
  size_t m_NumPatchPoints;
  size_t m_NumSearchPoints;
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindRelativeMotionBetweenSlices::FindRelativeMotionBetweenSlices() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindRelativeMotionBetweenSlices::~FindRelativeMotionBetweenSlices() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindRelativeMotionBetweenSlices::setupFilterParameters()
{
  FilterParameterVectorType parameters;
  {
    ChoiceFilterParameter::Pointer parameter = ChoiceFilterParameter::New();
    parameter->setHumanLabel("Plane of Interest");
    parameter->setPropertyName("Plane");
    parameter->setSetterCallback(SIMPL_BIND_SETTER(FindRelativeMotionBetweenSlices, this, Plane));
    parameter->setGetterCallback(SIMPL_BIND_GETTER(FindRelativeMotionBetweenSlices, this, Plane));

    std::vector<QString> choices;
    choices.push_back("XY");
    choices.push_back("XZ");
    choices.push_back("YZ");
    parameter->setChoices(choices);
    parameter->setCategory(FilterParameter::Category::Parameter);
    parameters.push_back(parameter);
  }
  parameters.push_back(SIMPL_NEW_INTEGER_FP("Patch Size 1", PSize1, FilterParameter::Category::Parameter, FindRelativeMotionBetweenSlices));
  parameters.push_back(SIMPL_NEW_INTEGER_FP("Patch Size 2", PSize2, FilterParameter::Category::Parameter, FindRelativeMotionBetweenSlices));
  parameters.push_back(SIMPL_NEW_INTEGER_FP("Search Distance 1", SSize1, FilterParameter::Category::Parameter, FindRelativeMotionBetweenSlices));
  parameters.push_back(SIMPL_NEW_INTEGER_FP("Search Distance 2", SSize2, FilterParameter::Category::Parameter, FindRelativeMotionBetweenSlices));
  parameters.push_back(SIMPL_NEW_INTEGER_FP("Slice Step", SliceStep, FilterParameter::Category::Parameter, FindRelativeMotionBetweenSlices));
  parameters.push_back(SeparatorFilterParameter::Create("Cell Data", FilterParameter::Category::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req =
        DataArraySelectionFilterParameter::CreateRequirement(SIMPL::Defaults::AnyPrimitive, 1, AttributeMatrix::Type::Cell, IGeometry::Type::Image);
    std::vector<QString> daTypes;
    daTypes.push_back(SIMPL::TypeNames::Int8);
    daTypes.push_back(SIMPL::TypeNames::Int16);
    daTypes.push_back(SIMPL::TypeNames::Int32);
    daTypes.push_back(SIMPL::TypeNames::Int64);
    daTypes.push_back(SIMPL::TypeNames::UInt8);
    daTypes.push_back(SIMPL::TypeNames::UInt16);
    daTypes.push_back(SIMPL::TypeNames::UInt32);
    daTypes.push_back(SIMPL::TypeNames::UInt64);
    daTypes.push_back(SIMPL::TypeNames::Float);
    daTypes.push_back(SIMPL::TypeNames::Double);
    req.daTypes = daTypes;
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Attribute Array to Track Motion", SelectedArrayPath, FilterParameter::Category::RequiredArray, FindRelativeMotionBetweenSlices, req));
  }
  parameters.push_back(SeparatorFilterParameter::Create("Cell Data", FilterParameter::Category::CreatedArray));
  parameters.push_back(
      SIMPL_NEW_DA_WITH_LINKED_AM_FP("Motion Direction", MotionDirectionArrayName, SelectedArrayPath, SelectedArrayPath, FilterParameter::Category::CreatedArray, FindRelativeMotionBetweenSlices));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
void FindRelativeMotionBetweenSlices::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setMotionDirectionArrayName(reader->readString("MotionDirectionArrayName", getMotionDirectionArrayName()));
  setSelectedArrayPath(reader->readDataArrayPath("SelectedArrayPath", getSelectedArrayPath()));
  setPlane(reader->readValue("Plane", getPlane()));
  setPSize1(reader->readValue("PSize1", getPSize1()));
  setPSize2(reader->readValue("PSize2", getPSize2()));
  setSSize1(reader->readValue("SSize1", getSSize1()));
  setSSize2(reader->readValue("SSize2", getSSize2()));
  setSliceStep(reader->readValue("SliceStep", getSliceStep()));
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindRelativeMotionBetweenSlices::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindRelativeMotionBetweenSlices::dataCheck()
{
  clearErrorCode();
  clearWarningCode();

  DataArrayPath tempPath;
  QString ss;

  m_InDataPtr = getDataContainerArray()->getPrereqIDataArrayFromPath(this, getSelectedArrayPath());
  if(nullptr != m_InDataPtr.lock())
  {
    if(TemplateHelpers::CanDynamicCast<BoolArrayType>()(m_InDataPtr.lock()))
    {
      QString ss = QObject::tr("Selected array cannot be of type bool.  The path is %1").arg(getSelectedArrayPath().serialize());
      setErrorCondition(-11001, ss);
    }
  }

  std::vector<size_t> cDims(1, 3);
  tempPath.update(m_SelectedArrayPath.getDataContainerName(), m_SelectedArrayPath.getAttributeMatrixName(), getMotionDirectionArrayName());
  m_MotionDirectionPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<float>>(this, tempPath, 0, cDims);
  if(nullptr != m_MotionDirectionPtr.lock())
  {
    m_MotionDirection = m_MotionDirectionPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */

  ImageGeom::Pointer image = getDataContainerArray()->getPrereqGeometryFromDataContainer<ImageGeom>(this, getSelectedArrayPath().getDataContainerName());
  if(getErrorCode() < 0)
  {
    return;
  }

  if(image->getXPoints() <= 1 || image->getYPoints() <= 1 || image->getZPoints() <= 1)
  {
    ss = QObject::tr("The Image Geometry is not 3D and cannot be run through this filter. The dimensions are (%1,%2,%3)").arg(image->getXPoints()).arg(image->getYPoints()).arg(image->getZPoints());
    setErrorCondition(-3000, ss);
  }

  if(getPSize1() <= 0 || getPSize2() <= 0)
  {
    ss = QObject::tr("The patch dimensions (%1, %2) must both be positive numbers").arg(getPSize1()).arg(getPSize2());
    setErrorCondition(-3001, ss);
  }

  if(getSSize1() <= 0 || getSSize2() <= 0)
  {
    ss = QObject::tr("The search dimensions (%1, %2) must both be positive numbers").arg(getSSize1()).arg(getSSize2());
    setErrorCondition(-3002, ss);
  }

  if(getPlane() == 0)
  {
    if(getSliceStep() >= static_cast<int64_t>(image->getZPoints()))
    {
      ss = QObject::tr("The Image Geometry extent (%1) is smaller than the supplied slice step (%2)").arg(image->getZPoints()).arg(getSliceStep());
      setErrorCondition(-3003, ss);
    }
  }

  if(getPlane() == 1)
  {
    if(getSliceStep() >= static_cast<int64_t>(image->getYPoints()))
    {
      ss = QObject::tr("The Image Geometry Y extent (%1) is smaller than the supplied slice step (%2)").arg(image->getYPoints()).arg(getSliceStep());
      setErrorCondition(-3004, ss);
    }
  }

  if(getPlane() == 2)
  {
    if(getSliceStep() >= static_cast<int64_t>(image->getXPoints()))
    {
      ss = QObject::tr("The Image Geometry X extent (%1) is smaller than the supplied slice step (%2)").arg(image->getXPoints()).arg(getSliceStep());
      setErrorCondition(-3005, ss);
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindRelativeMotionBetweenSlices::execute()
{
  dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(m_SelectedArrayPath.getDataContainerName());
  ImageGeom::Pointer image = m->getGeometryAs<ImageGeom>();

  int64_t xP = static_cast<int64_t>(image->getXPoints());
  int64_t yP = static_cast<int64_t>(image->getYPoints());
  int64_t zP = static_cast<int64_t>(image->getZPoints());
  size_t totalPoints = xP * yP * zP;

  int32_t buffer1 = (m_PSize1 / 2) + (m_SSize1 / 2);
  int32_t buffer2 = (m_PSize2 / 2) + (m_SSize1 / 2);

  std::vector<size_t> cDims(1, 4);
  Int32ArrayType::Pointer patchPointsPtr = Int32ArrayType::CreateArray((m_PSize1 * m_PSize2), std::string("_INTERNAL_USE_ONLY_patchPoints"), true);
  Int32ArrayType::Pointer searchPointsPtr = Int32ArrayType::CreateArray((m_SSize1 * m_SSize2), cDims, "_INTERNAL_USE_ONLY_searchPoints", true);
  BoolArrayType::Pointer validPointsPtr = BoolArrayType::CreateArray(totalPoints, std::string("_INTERNAL_USE_ONLY_validPoints"), true);
  validPointsPtr->initializeWithValue(false);
  int32_t* patchPoints = patchPointsPtr->getPointer(0);
  int32_t* searchPoints = searchPointsPtr->getPointer(0);
  bool* validPoints = validPointsPtr->getPointer(0);
  int64_t yStride = 0, zStride = 0;
  size_t count = 0;
  size_t numPatchPoints = 0, numSearchPoints = 0;

  if(m_Plane == 0)
  {
    for(int32_t j = -(m_PSize2 / 2); j < (m_PSize2 / 2); j++)
    {
      yStride = j * xP;
      for(int32_t i = -(m_PSize1 / 2); i < (m_PSize1 / 2); i++)
      {
        patchPoints[count] = yStride + i;
        count++;
      }
    }
    numPatchPoints = count;
    count = 0;
    for(int32_t j = -(m_SSize2 / 2); j <= (m_SSize2 / 2); j++)
    {
      yStride = j * xP;
      for(int32_t i = -(m_SSize1 / 2); i <= (m_SSize1 / 2); i++)
      {
        searchPoints[4 * count] = (m_SliceStep * xP * yP) + yStride + i;
        searchPoints[4 * count + 1] = i;
        searchPoints[4 * count + 2] = j;
        searchPoints[4 * count + 3] = m_SliceStep;
        count++;
      }
    }
    numSearchPoints = count;
    for(int64_t k = 0; k < zP - m_SliceStep; k++)
    {
      zStride = k * xP * yP;
      for(int64_t j = buffer2; j < (yP - buffer2); j++)
      {
        yStride = j * xP;
        for(int64_t i = buffer1; i < (xP - buffer1); i++)
        {
          validPoints[zStride + yStride + i] = true;
        }
      }
    }
  }

  if(m_Plane == 1)
  {
    for(int32_t j = -(m_PSize2 / 2); j < (m_PSize2 / 2); j++)
    {
      yStride = (j * xP * yP);
      for(int32_t i = -(m_PSize1 / 2); i < (m_PSize1 / 2); i++)
      {
        patchPoints[count] = yStride + i;
        count++;
      }
    }
    numPatchPoints = count;
    count = 0;
    for(int32_t j = -(m_SSize2 / 2); j <= (m_SSize2 / 2); j++)
    {
      yStride = (j * xP * yP);
      for(int32_t i = -(m_SSize1 / 2); i <= (m_SSize1 / 2); i++)
      {
        searchPoints[count] = (m_SliceStep * xP) + yStride + i;
        searchPoints[4 * count + 1] = i;
        searchPoints[4 * count + 2] = m_SliceStep;
        searchPoints[4 * count + 3] = j;
        count++;
      }
    }
    numSearchPoints = count;
    for(int64_t k = buffer2; k < (zP - buffer2); k++)
    {
      zStride = k * xP * yP;
      for(int64_t j = 0; j < yP - m_SliceStep; j++)
      {
        yStride = j * xP;
        for(int64_t i = buffer1; i < (xP - buffer1); i++)
        {
          validPoints[zStride + yStride + i] = true;
        }
      }
    }
  }

  if(m_Plane == 2)
  {
    for(int32_t j = -(m_PSize2 / 2); j < (m_PSize2 / 2); j++)
    {
      yStride = (j * xP * yP);
      for(int32_t i = -(m_PSize1 / 2); i < (m_PSize1 / 2); i++)
      {
        patchPoints[count] = yStride + (i * xP);
        count++;
      }
    }
    numPatchPoints = count;
    count = 0;
    for(int32_t j = -(m_SSize2 / 2); j <= (m_SSize2 / 2); j++)
    {
      yStride = (j * xP * yP);
      for(int32_t i = -(m_SSize1 / 2); i <= (m_SSize1 / 2); i++)
      {
        searchPoints[count] = (m_SliceStep) + yStride + (i * xP);
        searchPoints[4 * count + 1] = m_SliceStep;
        searchPoints[4 * count + 2] = i;
        searchPoints[4 * count + 3] = j;
        count++;
      }
    }
    numSearchPoints = count;
    for(int64_t k = buffer2; k < (zP - buffer2); k++)
    {
      zStride = k * xP * yP;
      for(int64_t j = buffer1; j < (yP - buffer1); j++)
      {
        yStride = j * xP;
        for(int64_t i = 0; i < xP - m_SliceStep; i++)
        {
          validPoints[zStride + yStride + i] = true;
        }
      }
    }
  }

  if(nullptr == patchPoints || nullptr == searchPoints || nullptr == validPoints)
  {
    QString ss = QObject::tr("Unable to establish search space for supplied parameters");
    setErrorCondition(-11001, ss);
    return;
  }

  if(TemplateHelpers::CanDynamicCast<Int8ArrayType>()(m_InDataPtr.lock()))
  {
    Int8ArrayType::Pointer cellArray = std::dynamic_pointer_cast<Int8ArrayType>(m_InDataPtr.lock());
    int8_t* cPtr = cellArray->getPointer(0);
#ifdef SIMPL_USE_PARALLEL_ALGORITHMS
    tbb::parallel_for(tbb::blocked_range<size_t>(0, totalPoints), CalcRelativeMotion<int8_t>(cPtr, m_MotionDirection, patchPoints, searchPoints, validPoints, numPatchPoints, numSearchPoints),
                      tbb::auto_partitioner());
#else
    CalcRelativeMotion<int8_t> serial(cPtr, m_MotionDirection, patchPoints, searchPoints, validPoints, numPatchPoints, numSearchPoints);
    serial.convert(0, totalPoints);
#endif
  }
  else if(TemplateHelpers::CanDynamicCast<UInt8ArrayType>()(m_InDataPtr.lock()))
  {
    UInt8ArrayType::Pointer cellArray = std::dynamic_pointer_cast<UInt8ArrayType>(m_InDataPtr.lock());
    uint8_t* cPtr = cellArray->getPointer(0);
#ifdef SIMPL_USE_PARALLEL_ALGORITHMS
    tbb::parallel_for(tbb::blocked_range<size_t>(0, totalPoints), CalcRelativeMotion<uint8_t>(cPtr, m_MotionDirection, patchPoints, searchPoints, validPoints, numPatchPoints, numSearchPoints),
                      tbb::auto_partitioner());
#else
    CalcRelativeMotion<uint8_t> serial(cPtr, m_MotionDirection, patchPoints, searchPoints, validPoints, numPatchPoints, numSearchPoints);
    serial.convert(0, totalPoints);
#endif
  }
  else if(TemplateHelpers::CanDynamicCast<Int16ArrayType>()(m_InDataPtr.lock()))
  {
    Int16ArrayType::Pointer cellArray = std::dynamic_pointer_cast<Int16ArrayType>(m_InDataPtr.lock());
    int16_t* cPtr = cellArray->getPointer(0);
#ifdef SIMPL_USE_PARALLEL_ALGORITHMS
    tbb::parallel_for(tbb::blocked_range<size_t>(0, totalPoints), CalcRelativeMotion<int16_t>(cPtr, m_MotionDirection, patchPoints, searchPoints, validPoints, numPatchPoints, numSearchPoints),
                      tbb::auto_partitioner());
#else
    CalcRelativeMotion<int16_t> serial(cPtr, m_MotionDirection, patchPoints, searchPoints, validPoints, numPatchPoints, numSearchPoints);
    serial.convert(0, totalPoints);
#endif
  }
  else if(TemplateHelpers::CanDynamicCast<UInt16ArrayType>()(m_InDataPtr.lock()))
  {
    UInt16ArrayType::Pointer cellArray = std::dynamic_pointer_cast<UInt16ArrayType>(m_InDataPtr.lock());
    uint16_t* cPtr = cellArray->getPointer(0);
#ifdef SIMPL_USE_PARALLEL_ALGORITHMS
    tbb::parallel_for(tbb::blocked_range<size_t>(0, totalPoints), CalcRelativeMotion<uint16_t>(cPtr, m_MotionDirection, patchPoints, searchPoints, validPoints, numPatchPoints, numSearchPoints),
                      tbb::auto_partitioner());
#else
    CalcRelativeMotion<uint16_t> serial(cPtr, m_MotionDirection, patchPoints, searchPoints, validPoints, numPatchPoints, numSearchPoints);
    serial.convert(0, totalPoints);
#endif
  }
  else if(TemplateHelpers::CanDynamicCast<Int32ArrayType>()(m_InDataPtr.lock()))
  {
    Int32ArrayType::Pointer cellArray = std::dynamic_pointer_cast<Int32ArrayType>(m_InDataPtr.lock());
    int32_t* cPtr = cellArray->getPointer(0);
#ifdef SIMPL_USE_PARALLEL_ALGORITHMS
    tbb::parallel_for(tbb::blocked_range<size_t>(0, totalPoints), CalcRelativeMotion<int32_t>(cPtr, m_MotionDirection, patchPoints, searchPoints, validPoints, numPatchPoints, numSearchPoints),
                      tbb::auto_partitioner());
#else
    CalcRelativeMotion<int32_t> serial(cPtr, m_MotionDirection, patchPoints, searchPoints, validPoints, numPatchPoints, numSearchPoints);
    serial.convert(0, totalPoints);
#endif
  }
  else if(TemplateHelpers::CanDynamicCast<UInt32ArrayType>()(m_InDataPtr.lock()))
  {
    UInt32ArrayType::Pointer cellArray = std::dynamic_pointer_cast<UInt32ArrayType>(m_InDataPtr.lock());
    uint32_t* cPtr = cellArray->getPointer(0);
#ifdef SIMPL_USE_PARALLEL_ALGORITHMS
    tbb::parallel_for(tbb::blocked_range<size_t>(0, totalPoints), CalcRelativeMotion<uint32_t>(cPtr, m_MotionDirection, patchPoints, searchPoints, validPoints, numPatchPoints, numSearchPoints),
                      tbb::auto_partitioner());
#else
    CalcRelativeMotion<uint32_t> serial(cPtr, m_MotionDirection, patchPoints, searchPoints, validPoints, numPatchPoints, numSearchPoints);
    serial.convert(0, totalPoints);
#endif
  }
  else if(TemplateHelpers::CanDynamicCast<Int64ArrayType>()(m_InDataPtr.lock()))
  {
    Int64ArrayType::Pointer cellArray = std::dynamic_pointer_cast<Int64ArrayType>(m_InDataPtr.lock());
    int64_t* cPtr = cellArray->getPointer(0);
#ifdef SIMPL_USE_PARALLEL_ALGORITHMS
    tbb::parallel_for(tbb::blocked_range<size_t>(0, totalPoints), CalcRelativeMotion<int64_t>(cPtr, m_MotionDirection, patchPoints, searchPoints, validPoints, numPatchPoints, numSearchPoints),
                      tbb::auto_partitioner());
#else
    CalcRelativeMotion<int64_t> serial(cPtr, m_MotionDirection, patchPoints, searchPoints, validPoints, numPatchPoints, numSearchPoints);
    serial.convert(0, totalPoints);
#endif
  }
  else if(TemplateHelpers::CanDynamicCast<UInt64ArrayType>()(m_InDataPtr.lock()))
  {
    UInt64ArrayType::Pointer cellArray = std::dynamic_pointer_cast<UInt64ArrayType>(m_InDataPtr.lock());
    uint64_t* cPtr = cellArray->getPointer(0);
#ifdef SIMPL_USE_PARALLEL_ALGORITHMS
    tbb::parallel_for(tbb::blocked_range<size_t>(0, totalPoints), CalcRelativeMotion<uint64_t>(cPtr, m_MotionDirection, patchPoints, searchPoints, validPoints, numPatchPoints, numSearchPoints),
                      tbb::auto_partitioner());
#else
    CalcRelativeMotion<uint64_t> serial(cPtr, m_MotionDirection, patchPoints, searchPoints, validPoints, numPatchPoints, numSearchPoints);
    serial.convert(0, totalPoints);
#endif
  }
  else if(TemplateHelpers::CanDynamicCast<FloatArrayType>()(m_InDataPtr.lock()))
  {
    FloatArrayType::Pointer cellArray = std::dynamic_pointer_cast<FloatArrayType>(m_InDataPtr.lock());
    float* cPtr = cellArray->getPointer(0);
#ifdef SIMPL_USE_PARALLEL_ALGORITHMS
    tbb::parallel_for(tbb::blocked_range<size_t>(0, totalPoints), CalcRelativeMotion<float>(cPtr, m_MotionDirection, patchPoints, searchPoints, validPoints, numPatchPoints, numSearchPoints),
                      tbb::auto_partitioner());
#else
    CalcRelativeMotion<float> serial(cPtr, m_MotionDirection, patchPoints, searchPoints, validPoints, numPatchPoints, numSearchPoints);
    serial.convert(0, totalPoints);
#endif
  }
  else if(TemplateHelpers::CanDynamicCast<DoubleArrayType>()(m_InDataPtr.lock()))
  {
    DoubleArrayType::Pointer cellArray = std::dynamic_pointer_cast<DoubleArrayType>(m_InDataPtr.lock());
    double* cPtr = cellArray->getPointer(0);
#ifdef SIMPL_USE_PARALLEL_ALGORITHMS
    tbb::parallel_for(tbb::blocked_range<size_t>(0, totalPoints), CalcRelativeMotion<double>(cPtr, m_MotionDirection, patchPoints, searchPoints, validPoints, numPatchPoints, numSearchPoints),
                      tbb::auto_partitioner());
#else
    CalcRelativeMotion<double> serial(cPtr, m_MotionDirection, patchPoints, searchPoints, validPoints, numPatchPoints, numSearchPoints);
    serial.convert(0, totalPoints);
#endif
  }
  else
  {
    QString ss = QObject::tr("Selected array is of unsupported type. The type is %1").arg(m_InDataPtr.lock()->getTypeAsString());
    setErrorCondition(-3007, ss);
    return;
  }

  float v[3];
  FloatVec3Type spacing = m->getGeometryAs<ImageGeom>()->getSpacing();

  for(size_t i = 0; i < totalPoints; i++)
  {
    v[0] = m_MotionDirection[3 * i + 0] * spacing[0];
    v[1] = m_MotionDirection[3 * i + 1] * spacing[1];
    v[2] = m_MotionDirection[3 * i + 2] * spacing[2];
    MatrixMath::Normalize3x1(v);
    m_MotionDirection[3 * i + 0] = v[0];
    m_MotionDirection[3 * i + 1] = v[1];
    m_MotionDirection[3 * i + 2] = v[2];
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer FindRelativeMotionBetweenSlices::newFilterInstance(bool copyFilterParameters) const
{
  FindRelativeMotionBetweenSlices::Pointer filter = FindRelativeMotionBetweenSlices::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindRelativeMotionBetweenSlices::getCompiledLibraryName() const
{
  return ProcessingConstants::ProcessingBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindRelativeMotionBetweenSlices::getBrandingString() const
{
  return "Processing";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindRelativeMotionBetweenSlices::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream << Processing::Version::Major() << "." << Processing::Version::Minor() << "." << Processing::Version::Patch();
  return version;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindRelativeMotionBetweenSlices::getGroupName() const
{
  return SIMPL::FilterGroups::ProcessingFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid FindRelativeMotionBetweenSlices::getUuid() const
{
  return QUuid("{801008ce-1dcb-5604-8f16-a86526e28cf9}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindRelativeMotionBetweenSlices::getSubGroupName() const
{
  return SIMPL::FilterSubGroups::ImageFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindRelativeMotionBetweenSlices::getHumanLabel() const
{
  return "Find Relative Motion Between Slices";
}

// -----------------------------------------------------------------------------
FindRelativeMotionBetweenSlices::Pointer FindRelativeMotionBetweenSlices::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<FindRelativeMotionBetweenSlices> FindRelativeMotionBetweenSlices::New()
{
  struct make_shared_enabler : public FindRelativeMotionBetweenSlices
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString FindRelativeMotionBetweenSlices::getNameOfClass() const
{
  return QString("FindRelativeMotionBetweenSlices");
}

// -----------------------------------------------------------------------------
QString FindRelativeMotionBetweenSlices::ClassName()
{
  return QString("FindRelativeMotionBetweenSlices");
}

// -----------------------------------------------------------------------------
void FindRelativeMotionBetweenSlices::setSelectedArrayPath(const DataArrayPath& value)
{
  m_SelectedArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FindRelativeMotionBetweenSlices::getSelectedArrayPath() const
{
  return m_SelectedArrayPath;
}

// -----------------------------------------------------------------------------
void FindRelativeMotionBetweenSlices::setPlane(unsigned int value)
{
  m_Plane = value;
}

// -----------------------------------------------------------------------------
unsigned int FindRelativeMotionBetweenSlices::getPlane() const
{
  return m_Plane;
}

// -----------------------------------------------------------------------------
void FindRelativeMotionBetweenSlices::setPSize1(int value)
{
  m_PSize1 = value;
}

// -----------------------------------------------------------------------------
int FindRelativeMotionBetweenSlices::getPSize1() const
{
  return m_PSize1;
}

// -----------------------------------------------------------------------------
void FindRelativeMotionBetweenSlices::setPSize2(int value)
{
  m_PSize2 = value;
}

// -----------------------------------------------------------------------------
int FindRelativeMotionBetweenSlices::getPSize2() const
{
  return m_PSize2;
}

// -----------------------------------------------------------------------------
void FindRelativeMotionBetweenSlices::setSSize1(int value)
{
  m_SSize1 = value;
}

// -----------------------------------------------------------------------------
int FindRelativeMotionBetweenSlices::getSSize1() const
{
  return m_SSize1;
}

// -----------------------------------------------------------------------------
void FindRelativeMotionBetweenSlices::setSSize2(int value)
{
  m_SSize2 = value;
}

// -----------------------------------------------------------------------------
int FindRelativeMotionBetweenSlices::getSSize2() const
{
  return m_SSize2;
}

// -----------------------------------------------------------------------------
void FindRelativeMotionBetweenSlices::setSliceStep(int value)
{
  m_SliceStep = value;
}

// -----------------------------------------------------------------------------
int FindRelativeMotionBetweenSlices::getSliceStep() const
{
  return m_SliceStep;
}

// -----------------------------------------------------------------------------
void FindRelativeMotionBetweenSlices::setMotionDirectionArrayName(const QString& value)
{
  m_MotionDirectionArrayName = value;
}

// -----------------------------------------------------------------------------
QString FindRelativeMotionBetweenSlices::getMotionDirectionArrayName() const
{
  return m_MotionDirectionArrayName;
}
