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
#include "VectorSegmentFeatures.h"

#include <chrono>

#include <QtCore/QTextStream>

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/DataContainers/DataContainer.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/BooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/FloatFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedBooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/Math/GeometryMath.h"
#include "SIMPLib/Math/MatrixMath.h"
#include "SIMPLib/Math/SIMPLibMath.h"

#include "Reconstruction/ReconstructionConstants.h"
#include "Reconstruction/ReconstructionVersion.h"

/* Create Enumerations to allow the created Attribute Arrays to take part in renaming */
enum createdPathID : RenameDataPath::DataID_t
{
  AttributeMatrixID21 = 21,

  DataArrayID30 = 30,
  DataArrayID31 = 31,
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VectorSegmentFeatures::VectorSegmentFeatures() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VectorSegmentFeatures::~VectorSegmentFeatures() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VectorSegmentFeatures::setupFilterParameters()
{
  SegmentFeatures::setupFilterParameters();
  FilterParameterVectorType parameters;

  parameters.push_back(SIMPL_NEW_FLOAT_FP("Angle Tolerance", AngleTolerance, FilterParameter::Category::Parameter, VectorSegmentFeatures));
  std::vector<QString> linkedProps = {"GoodVoxelsArrayPath"};
  parameters.push_back(SIMPL_NEW_LINKED_BOOL_FP("Use Mask Array", UseGoodVoxels, FilterParameter::Category::Parameter, VectorSegmentFeatures, linkedProps));
  parameters.push_back(SIMPL_NEW_BOOL_FP("Randomize Feature Ids", RandomizeFeatureIds, FilterParameter::Category::Parameter, VectorSegmentFeatures));

  {
    DataArraySelectionFilterParameter::RequirementType req;
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Vector Array To Segment", SelectedVectorArrayPath, FilterParameter::Category::RequiredArray, VectorSegmentFeatures, req));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req;
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Mask", GoodVoxelsArrayPath, FilterParameter::Category::RequiredArray, VectorSegmentFeatures, req));
  }

  parameters.push_back(SIMPL_NEW_STRING_FP("Cell Feature Ids", FeatureIdsArrayName, FilterParameter::Category::CreatedArray, VectorSegmentFeatures));
  parameters.push_back(SIMPL_NEW_STRING_FP("Cell Feature Attribute Matrix Name", CellFeatureAttributeMatrixName, FilterParameter::Category::CreatedArray, VectorSegmentFeatures));
  parameters.push_back(SIMPL_NEW_STRING_FP("Active", ActiveArrayName, FilterParameter::Category::CreatedArray, VectorSegmentFeatures));

  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VectorSegmentFeatures::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setActiveArrayName(reader->readString("ActiveArrayName", getActiveArrayName()));
  setCellFeatureAttributeMatrixName(reader->readString("CellFeatureAttributeMatrixName", getCellFeatureAttributeMatrixName()));
  setFeatureIdsArrayName(reader->readString("FeatureIdsArrayName", getFeatureIdsArrayName()));
  setGoodVoxelsArrayPath(reader->readDataArrayPath("GoodVoxelsArrayPath", getGoodVoxelsArrayPath()));
  setUseGoodVoxels(reader->readValue("UseGoodVoxels", getUseGoodVoxels()));
  setSelectedVectorArrayPath(reader->readDataArrayPath("SelectedVectorArrayPath", getSelectedVectorArrayPath()));
  setAngleTolerance(reader->readValue("AngleTolerance", getAngleTolerance()));
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VectorSegmentFeatures::updateFeatureInstancePointers()
{
  clearErrorCode();
  clearWarningCode();

  if(nullptr != m_ActivePtr.lock())
  {
    m_Active = m_ActivePtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VectorSegmentFeatures::initialize()
{
  m_AngleToleranceRad = 0.0f;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VectorSegmentFeatures::dataCheck()
{

  clearErrorCode();
  clearWarningCode();
  initialize();
  DataArrayPath tempPath;

  // Set the DataContainerName for the Parent Class (SegmentFeatures) to Use
  setDataContainerName(m_SelectedVectorArrayPath.getDataContainerName());

  SegmentFeatures::dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  DataContainer::Pointer m = getDataContainerArray()->getPrereqDataContainer(this, getDataContainerName(), false);
  if(getErrorCode() < 0 || nullptr == m.get())
  {
    return;
  }

  std::vector<size_t> tDims(1, 0);
  m->createNonPrereqAttributeMatrix(this, getCellFeatureAttributeMatrixName(), tDims, AttributeMatrix::Type::CellFeature, AttributeMatrixID21);

  QVector<DataArrayPath> dataArrayPaths;

  std::vector<size_t> cDims(1, 3);
  m_VectorsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<float>>(this, getSelectedVectorArrayPath(), cDims);
  if(nullptr != m_VectorsPtr.lock())
  {
    m_Vectors = m_VectorsPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCode() >= 0)
  {
    dataArrayPaths.push_back(getSelectedVectorArrayPath());
  }

  cDims[0] = 1;
  tempPath.update(getDataContainerName(), m_SelectedVectorArrayPath.getAttributeMatrixName(), getFeatureIdsArrayName());
  m_FeatureIdsPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<int32_t>>(this, tempPath, 0, cDims);
  if(nullptr != m_FeatureIdsPtr.lock())
  {
    m_FeatureIds = m_FeatureIdsPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */

  if(m_UseGoodVoxels)
  {
    m_GoodVoxelsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<bool>>(this, getGoodVoxelsArrayPath(), cDims);
    if(nullptr != m_GoodVoxelsPtr.lock())
    {
      m_GoodVoxels = m_GoodVoxelsPtr.lock()->getPointer(0);
    } /* Now assign the raw pointer to data from the DataArray<T> object */
    if(getErrorCode() >= 0)
    {
      dataArrayPaths.push_back(getGoodVoxelsArrayPath());
    }
  }

  tempPath.update(getDataContainerName(), getCellFeatureAttributeMatrixName(), getActiveArrayName());
  m_ActivePtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<bool>>(this, tempPath, true, cDims, "", DataArrayID31);
  if(nullptr != m_ActivePtr.lock())
  {
    m_Active = m_ActivePtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */

  getDataContainerArray()->validateNumberOfTuples(this, dataArrayPaths);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VectorSegmentFeatures::randomizeFeatureIds(int64_t totalPoints, int64_t totalFeatures)
{
  notifyStatusMessage("Randomizing Feature Ids");
  // Generate an even distribution of numbers between the min and max range
  const int64_t rangeMin = 1;
  const int64_t rangeMax = totalFeatures - 1;
  initializeVoxelSeedGenerator(rangeMin, rangeMax);

  DataArray<int64_t>::Pointer rndNumbers = DataArray<int64_t>::CreateArray(totalFeatures, std::string("_INTERNAL_USE_ONLY_NewFeatureIds"), true);

  int64_t* gid = rndNumbers->getPointer(0);
  gid[0] = 0;

  for(int64_t i = 1; i < totalFeatures; ++i)
  {
    gid[i] = i;
  }

  int64_t r = 0;
  int64_t temp = 0;

  //--- Shuffle elements by randomly exchanging each with one other.
  for(int64_t i = 1; i < totalFeatures; i++)
  {
    r = m_Distribution(m_Generator); // Random remaining position.
    if(r >= totalFeatures)
    {
      continue;
    }
    temp = gid[i];
    gid[i] = gid[r];
    gid[r] = temp;
  }

  // Now adjust all the Grain Id values for each Voxel
  for(int64_t i = 0; i < totalPoints; ++i)
  {
    m_FeatureIds[i] = gid[m_FeatureIds[i]];
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int64_t VectorSegmentFeatures::getSeed(int32_t gnum, int64_t nextSeed)
{
  clearErrorCode();
  clearWarningCode();
  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getDataContainerName());

  size_t totalPoints = m_FeatureIdsPtr.lock()->getNumberOfTuples();
  int64_t seed = -1;
  // start with the next voxel after the last seed
  size_t randpoint = static_cast<size_t>(nextSeed);
  while(seed == -1 && randpoint < totalPoints)
  {
    if(m_FeatureIds[randpoint] == 0) // If the GrainId of the voxel is ZERO then we can use this as a seed point
    {
      if(!m_UseGoodVoxels || m_GoodVoxels[randpoint])
      {
        seed = randpoint;
      }
      else
      {
        randpoint += 1;
      }
    }
    else
    {
      randpoint += 1;
    }
  }
  if(seed >= 0)
  {
    m_FeatureIds[seed] = gnum;
    std::vector<size_t> tDims(1, gnum + 1);
    m->getAttributeMatrix(getCellFeatureAttributeMatrixName())->resizeAttributeArrays(tDims);
    updateFeatureInstancePointers();
  }
  return seed;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool VectorSegmentFeatures::determineGrouping(int64_t referencepoint, int64_t neighborpoint, int32_t gnum)
{
  bool group = false;
  float v1[3] = {0.0f, 0.0f, 0.0f};
  float v2[3] = {0.0f, 0.0f, 0.0f};
  if(m_FeatureIds[neighborpoint] == 0 && (!m_UseGoodVoxels || m_GoodVoxels[neighborpoint]))
  {
    v1[0] = m_Vectors[3 * referencepoint + 0];
    v1[1] = m_Vectors[3 * referencepoint + 1];
    v1[2] = m_Vectors[3 * referencepoint + 2];
    v2[0] = m_Vectors[3 * neighborpoint + 0];
    v2[1] = m_Vectors[3 * neighborpoint + 1];
    v2[2] = m_Vectors[3 * neighborpoint + 2];
    if(v1[2] < 0)
    {
      MatrixMath::Multiply3x1withConstant(v1, -1.0f);
    }
    if(v2[2] < 0)
    {
      MatrixMath::Multiply3x1withConstant(v2, -1.0f);
    }
    float w = GeometryMath::CosThetaBetweenVectors(v1, v2);
    w = acosf(w);
    if(w > SIMPLib::Constants::k_PiOver2D)
    {
      w = SIMPLib::Constants::k_PiD - w;
    }
    if(w < m_AngleToleranceRad)
    {
      group = true;
      m_FeatureIds[neighborpoint] = gnum;
    }
  }
  return group;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VectorSegmentFeatures::initializeVoxelSeedGenerator(const int64_t rangeMin, const int64_t rangeMax)
{

  std::mt19937_64::result_type seed = static_cast<std::mt19937_64::result_type>(std::chrono::steady_clock::now().time_since_epoch().count());
  m_Generator.seed(seed);
  m_Distribution = std::uniform_int_distribution<int64_t>(rangeMin, rangeMax);
  m_Distribution = std::uniform_int_distribution<int64_t>(rangeMin, rangeMax);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VectorSegmentFeatures::execute()
{
  dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getDataContainerName());

  std::vector<size_t> tDims(1, 1);
  m->getAttributeMatrix(getCellFeatureAttributeMatrixName())->resizeAttributeArrays(tDims);
  updateFeatureInstancePointers();

  int64_t totalPoints = static_cast<int64_t>(m_FeatureIdsPtr.lock()->getNumberOfTuples());

  // Convert user defined tolerance to radians.
  m_AngleToleranceRad = m_AngleTolerance * SIMPLib::Constants::k_PiD / 180.0f;

  // Generate the random voxel indices that will be used for the seed points to start a new grain growth/agglomeration
  const int64_t rangeMin = 0;
  const int64_t rangeMax = totalPoints - 1;
  initializeVoxelSeedGenerator(rangeMin, rangeMax);

  SegmentFeatures::execute();

  int32_t totalFeatures = static_cast<int32_t>(m->getAttributeMatrix(getCellFeatureAttributeMatrixName())->getNumberOfTuples());
  if(totalFeatures < 2)
  {
    setErrorCondition(-87000, "The number of Features was 0 or 1 which means no Features were detected. A threshold value may be set too high");
    return;
  }

  // By default we randomize grains
  if(m_RandomizeFeatureIds)
  {
    randomizeFeatureIds(totalPoints, totalFeatures);
  }

  // If there is an error set this to something negative and also set a message
  notifyStatusMessage("Completed");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer VectorSegmentFeatures::newFilterInstance(bool copyFilterParameters) const
{
  VectorSegmentFeatures::Pointer filter = VectorSegmentFeatures::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString VectorSegmentFeatures::getCompiledLibraryName() const
{
  return ReconstructionConstants::ReconstructionBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString VectorSegmentFeatures::getBrandingString() const
{
  return "Reconstruction";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString VectorSegmentFeatures::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream << Reconstruction::Version::Major() << "." << Reconstruction::Version::Minor() << "." << Reconstruction::Version::Patch();
  return version;
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString VectorSegmentFeatures::getGroupName() const
{
  return SIMPL::FilterGroups::ReconstructionFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid VectorSegmentFeatures::getUuid() const
{
  return QUuid("{c438af53-631e-581e-8b2d-1a23dcdc6f32}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString VectorSegmentFeatures::getSubGroupName() const
{
  return SIMPL::FilterSubGroups::SegmentationFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString VectorSegmentFeatures::getHumanLabel() const
{
  return "Segment Features (Vector)";
}

// -----------------------------------------------------------------------------
VectorSegmentFeatures::Pointer VectorSegmentFeatures::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<VectorSegmentFeatures> VectorSegmentFeatures::New()
{
  struct make_shared_enabler : public VectorSegmentFeatures
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString VectorSegmentFeatures::getNameOfClass() const
{
  return QString("VectorSegmentFeatures");
}

// -----------------------------------------------------------------------------
QString VectorSegmentFeatures::ClassName()
{
  return QString("VectorSegmentFeatures");
}

// -----------------------------------------------------------------------------
void VectorSegmentFeatures::setCellFeatureAttributeMatrixName(const QString& value)
{
  m_CellFeatureAttributeMatrixName = value;
}

// -----------------------------------------------------------------------------
QString VectorSegmentFeatures::getCellFeatureAttributeMatrixName() const
{
  return m_CellFeatureAttributeMatrixName;
}

// -----------------------------------------------------------------------------
void VectorSegmentFeatures::setSelectedVectorArrayPath(const DataArrayPath& value)
{
  m_SelectedVectorArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath VectorSegmentFeatures::getSelectedVectorArrayPath() const
{
  return m_SelectedVectorArrayPath;
}

// -----------------------------------------------------------------------------
void VectorSegmentFeatures::setAngleTolerance(float value)
{
  m_AngleTolerance = value;
}

// -----------------------------------------------------------------------------
float VectorSegmentFeatures::getAngleTolerance() const
{
  return m_AngleTolerance;
}

// -----------------------------------------------------------------------------
void VectorSegmentFeatures::setRandomizeFeatureIds(bool value)
{
  m_RandomizeFeatureIds = value;
}

// -----------------------------------------------------------------------------
bool VectorSegmentFeatures::getRandomizeFeatureIds() const
{
  return m_RandomizeFeatureIds;
}

// -----------------------------------------------------------------------------
void VectorSegmentFeatures::setUseGoodVoxels(bool value)
{
  m_UseGoodVoxels = value;
}

// -----------------------------------------------------------------------------
bool VectorSegmentFeatures::getUseGoodVoxels() const
{
  return m_UseGoodVoxels;
}

// -----------------------------------------------------------------------------
void VectorSegmentFeatures::setGoodVoxelsArrayPath(const DataArrayPath& value)
{
  m_GoodVoxelsArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath VectorSegmentFeatures::getGoodVoxelsArrayPath() const
{
  return m_GoodVoxelsArrayPath;
}

// -----------------------------------------------------------------------------
void VectorSegmentFeatures::setFeatureIdsArrayName(const QString& value)
{
  m_FeatureIdsArrayName = value;
}

// -----------------------------------------------------------------------------
QString VectorSegmentFeatures::getFeatureIdsArrayName() const
{
  return m_FeatureIdsArrayName;
}

// -----------------------------------------------------------------------------
void VectorSegmentFeatures::setActiveArrayName(const QString& value)
{
  m_ActiveArrayName = value;
}

// -----------------------------------------------------------------------------
QString VectorSegmentFeatures::getActiveArrayName() const
{
  return m_ActiveArrayName;
}
