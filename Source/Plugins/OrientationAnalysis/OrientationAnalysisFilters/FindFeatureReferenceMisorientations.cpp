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
#include "FindFeatureReferenceMisorientations.h"

#include <QtCore/QTextStream>

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/DataContainers/DataContainer.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedChoicesFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedPathCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"
#include "SIMPLib/Math/SIMPLibMath.h"

#include "EbsdLib/Core/EbsdLibConstants.h"
#include "EbsdLib/Core/Orientation.hpp"
#include "EbsdLib/Core/OrientationTransformation.hpp"
#include "EbsdLib/Core/Quaternion.hpp"
#include "EbsdLib/LaueOps/LaueOps.h"
#include "OrientationAnalysis/OrientationAnalysisConstants.h"
#include "OrientationAnalysis/OrientationAnalysisVersion.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindFeatureReferenceMisorientations::FindFeatureReferenceMisorientations() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindFeatureReferenceMisorientations::~FindFeatureReferenceMisorientations() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindFeatureReferenceMisorientations::setupFilterParameters()
{
  FilterParameterVectorType parameters;

  {
    LinkedChoicesFilterParameter::Pointer parameter = LinkedChoicesFilterParameter::New();
    parameter->setHumanLabel("Reference Orientation");
    parameter->setPropertyName("ReferenceOrientation");
    parameter->setSetterCallback(SIMPL_BIND_SETTER(FindFeatureReferenceMisorientations, this, ReferenceOrientation));
    parameter->setGetterCallback(SIMPL_BIND_GETTER(FindFeatureReferenceMisorientations, this, ReferenceOrientation));

    std::vector<QString> choices;
    choices.push_back("Average Orientation");
    choices.push_back("Orientation at Feature Centroid");
    parameter->setChoices(choices);
    std::vector<QString> linkedProps;
    linkedProps.push_back("GBEuclideanDistancesArrayPath");
    linkedProps.push_back("AvgQuatsArrayPath");
    parameter->setLinkedProperties(linkedProps);
    parameter->setEditable(false);
    parameter->setCategory(FilterParameter::Category::Parameter);
    parameters.push_back(parameter);
  }
  parameters.push_back(SeparatorFilterParameter::Create("Cell Data", FilterParameter::Category::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::Int32, 1, AttributeMatrix::Type::Cell, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Feature Ids", FeatureIdsArrayPath, FilterParameter::Category::RequiredArray, FindFeatureReferenceMisorientations, req));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::Int32, 1, AttributeMatrix::Type::Cell, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Phases", CellPhasesArrayPath, FilterParameter::Category::RequiredArray, FindFeatureReferenceMisorientations, req));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::Float, 4, AttributeMatrix::Type::Cell, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Quaternions", QuatsArrayPath, FilterParameter::Category::RequiredArray, FindFeatureReferenceMisorientations, req));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::Float, 1, AttributeMatrix::Type::Cell, IGeometry::Type::Image);
    parameters.push_back(
        SIMPL_NEW_DA_SELECTION_FP("Boundary Euclidean Distances", GBEuclideanDistancesArrayPath, FilterParameter::Category::RequiredArray, FindFeatureReferenceMisorientations, req, {1}));
  }
  parameters.push_back(SeparatorFilterParameter::Create("Cell Feature Data", FilterParameter::Category::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req =
        DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::Float, 4, AttributeMatrix::Type::CellFeature, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Average Quaternions", AvgQuatsArrayPath, FilterParameter::Category::RequiredArray, FindFeatureReferenceMisorientations, req, {0}));
  }
  parameters.push_back(SeparatorFilterParameter::Create("Cell Ensemble Data", FilterParameter::Category::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req =
        DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::UInt32, 1, AttributeMatrix::Type::CellEnsemble, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Crystal Structures", CrystalStructuresArrayPath, FilterParameter::Category::RequiredArray, FindFeatureReferenceMisorientations, req));
  }
  parameters.push_back(SeparatorFilterParameter::Create("Cell Data", FilterParameter::Category::CreatedArray));
  parameters.push_back(SIMPL_NEW_DA_WITH_LINKED_AM_FP("Feature Reference Misorientations", FeatureReferenceMisorientationsArrayName, FeatureIdsArrayPath, FeatureIdsArrayPath,
                                                      FilterParameter::Category::CreatedArray, FindFeatureReferenceMisorientations));
  parameters.push_back(SeparatorFilterParameter::Create("Cell Feature Data", FilterParameter::Category::CreatedArray));
  parameters.push_back(SIMPL_NEW_DA_WITH_LINKED_AM_FP("Average Misorientations", FeatureAvgMisorientationsArrayName, AvgQuatsArrayPath, AvgQuatsArrayPath, FilterParameter::Category::CreatedArray,
                                                      FindFeatureReferenceMisorientations));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
void FindFeatureReferenceMisorientations::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setFeatureReferenceMisorientationsArrayName(reader->readString("FeatureReferenceMisorientationsArrayName", getFeatureReferenceMisorientationsArrayName()));
  setFeatureAvgMisorientationsArrayName(reader->readString("FeatureAvgMisorientationsArrayName", getFeatureAvgMisorientationsArrayName()));
  setGBEuclideanDistancesArrayPath(reader->readDataArrayPath("GBEuclideanDistancesArrayPath", getGBEuclideanDistancesArrayPath()));
  setAvgQuatsArrayPath(reader->readDataArrayPath("AvgQuatsArrayPath", getAvgQuatsArrayPath()));
  setQuatsArrayPath(reader->readDataArrayPath("QuatsArrayPath", getQuatsArrayPath()));
  setCrystalStructuresArrayPath(reader->readDataArrayPath("CrystalStructuresArrayPath", getCrystalStructuresArrayPath()));
  setCellPhasesArrayPath(reader->readDataArrayPath("CellPhasesArrayPath", getCellPhasesArrayPath()));
  setFeatureIdsArrayPath(reader->readDataArrayPath("FeatureIdsArrayPath", getFeatureIdsArrayPath()));
  setReferenceOrientation(reader->readValue("ReferenceOrientation", getReferenceOrientation()));
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindFeatureReferenceMisorientations::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindFeatureReferenceMisorientations::dataCheck()
{
  clearErrorCode();
  clearWarningCode();
  DataArrayPath tempPath;

  getDataContainerArray()->getPrereqGeometryFromDataContainer<ImageGeom>(this, getFeatureIdsArrayPath().getDataContainerName());

  QVector<DataArrayPath> dataArrayPaths;

  std::vector<size_t> cDims(1, 1);
  m_FeatureIdsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<int32_t>>(this, getFeatureIdsArrayPath(), cDims);
  if(nullptr != m_FeatureIdsPtr.lock())
  {
    m_FeatureIds = m_FeatureIdsPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCode() >= 0)
  {
    dataArrayPaths.push_back(getFeatureIdsArrayPath());
  }

  m_CellPhasesPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<int32_t>>(this, getCellPhasesArrayPath(), cDims);
  if(nullptr != m_CellPhasesPtr.lock())
  {
    m_CellPhases = m_CellPhasesPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCode() >= 0)
  {
    dataArrayPaths.push_back(getCellPhasesArrayPath());
  }

  tempPath.update(m_AvgQuatsArrayPath.getDataContainerName(), m_AvgQuatsArrayPath.getAttributeMatrixName(), getFeatureAvgMisorientationsArrayName());
  m_FeatureAvgMisorientationsPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<float>>(this, tempPath, 0, cDims);
  if(nullptr != m_FeatureAvgMisorientationsPtr.lock())
  {
    m_FeatureAvgMisorientations = m_FeatureAvgMisorientationsPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */

  tempPath.update(m_FeatureIdsArrayPath.getDataContainerName(), m_FeatureIdsArrayPath.getAttributeMatrixName(), getFeatureReferenceMisorientationsArrayName());
  m_FeatureReferenceMisorientationsPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<float>>(this, tempPath, 0, cDims);
  if(nullptr != m_FeatureReferenceMisorientationsPtr.lock())
  {
    m_FeatureReferenceMisorientations = m_FeatureReferenceMisorientationsPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */

  m_CrystalStructuresPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<uint32_t>>(this, getCrystalStructuresArrayPath(), cDims);
  if(nullptr != m_CrystalStructuresPtr.lock())
  {
    m_CrystalStructures = m_CrystalStructuresPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */

  cDims[0] = 4;
  m_QuatsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<float>>(this, getQuatsArrayPath(), cDims);
  if(nullptr != m_QuatsPtr.lock())
  {
    m_Quats = m_QuatsPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCode() >= 0)
  {
    dataArrayPaths.push_back(getQuatsArrayPath());
  }

  if(m_ReferenceOrientation == 0)
  {
    m_AvgQuatsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<float>>(this, getAvgQuatsArrayPath(), cDims);
    if(nullptr != m_AvgQuatsPtr.lock())
    {
      m_AvgQuats = m_AvgQuatsPtr.lock()->getPointer(0);
    } /* Now assign the raw pointer to data from the DataArray<T> object */
  }
  else if(m_ReferenceOrientation == 1)
  {
    cDims[0] = 1;
    m_GBEuclideanDistancesPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<float>>(this, getGBEuclideanDistancesArrayPath(), cDims);
    if(nullptr != m_GBEuclideanDistancesPtr.lock())
    {
      m_GBEuclideanDistances = m_GBEuclideanDistancesPtr.lock()->getPointer(0);
    } /* Now assign the raw pointer to data from the DataArray<T> object */
    if(getErrorCode() >= 0)
    {
      dataArrayPaths.push_back(getGBEuclideanDistancesArrayPath());
    }
  }

  getDataContainerArray()->validateNumberOfTuples(this, dataArrayPaths);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindFeatureReferenceMisorientations::execute()
{
  dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  std::vector<LaueOps::Pointer> m_OrientationOps = LaueOps::GetAllOrientationOps();

  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(m_FeatureIdsArrayPath.getDataContainerName());
  size_t totalPoints = m_FeatureIdsPtr.lock()->getNumberOfTuples();
  size_t totalFeatures = m_AvgQuatsPtr.lock()->getNumberOfTuples();

  FloatArrayType::Pointer quatsPtr = m_QuatsPtr.lock();
  FloatArrayType::Pointer avgQuatsPtr = m_AvgQuatsPtr.lock();

  // float n1 = 0.0f, n2 = 0.0f, n3 = 0.0f;
  uint32_t phase1 = EbsdLib::CrystalStructure::UnknownCrystalStructure;
  uint32_t phase2 = EbsdLib::CrystalStructure::UnknownCrystalStructure;
  SizeVec3Type udims = m->getGeometryAs<ImageGeom>()->getDimensions();

  uint32_t maxUInt32 = std::numeric_limits<uint32_t>::max();
  // We have more points than can be allocated on a 32 bit machine. Assert Now.
  if(totalPoints > maxUInt32)
  {
    QString ss = QObject::tr("The volume is too large for a 32 bit machine. Try reducing the input volume size. Total Voxels: %1").arg(totalPoints);
    notifyStatusMessage(ss);
    return;
  }

  int32_t gnum = 0;
  float dist = 0.0f;
  std::vector<size_t> m_Centers(totalFeatures, 0);
  std::vector<float> m_CenterDists(totalFeatures, 0.0f);
  if(m_ReferenceOrientation == 1)
  {
    for(size_t i = 0; i < totalPoints; i++)
    {
      gnum = m_FeatureIds[i];
      dist = m_GBEuclideanDistances[i];
      if(dist >= m_CenterDists[gnum])
      {
        m_CenterDists[gnum] = dist;
        m_Centers[gnum] = i;
      }
    }
  }

  FloatArrayType::Pointer avgMisoPtr = FloatArrayType::CreateArray(totalFeatures * 2, std::string("_INTERNAL_USE_ONLY_AVERAGE_MISORIENTATION"), true);
  avgMisoPtr->initializeWithZeros();
  float* avgMiso = avgMisoPtr->getPointer(0);

  int64_t xPoints = static_cast<int64_t>(udims[0]);
  int64_t yPoints = static_cast<int64_t>(udims[1]);
  int64_t zPoints = static_cast<int64_t>(udims[2]);
  int64_t point = 0;
  int32_t idx = 0;
  float* currentQuatPtr = nullptr;

  for(int64_t col = 0; col < xPoints; col++)
  {
    for(int64_t row = 0; row < yPoints; row++)
    {
      for(int64_t plane = 0; plane < zPoints; plane++)
      {
        point = (plane * xPoints * yPoints) + (row * xPoints) + col;
        if(m_FeatureIds[point] > 0 && m_CellPhases[point] > 0)
        {
          currentQuatPtr = quatsPtr->getTuplePointer(point);

          QuatF q1(currentQuatPtr[0], currentQuatPtr[1], currentQuatPtr[2], currentQuatPtr[3]);
          QuatF q2;
          phase1 = m_CrystalStructures[m_CellPhases[point]];
          if(m_ReferenceOrientation == 0)
          {
            currentQuatPtr = avgQuatsPtr->getTuplePointer(m_FeatureIds[point]);
            q2 = QuatF(currentQuatPtr[0], currentQuatPtr[1], currentQuatPtr[2], currentQuatPtr[3]);
          }
          else if(m_ReferenceOrientation == 1)
          {
            gnum = m_FeatureIds[point];
            currentQuatPtr = avgQuatsPtr->getTuplePointer(m_Centers[gnum]);
            q2 = QuatF(currentQuatPtr[0], currentQuatPtr[1], currentQuatPtr[2], currentQuatPtr[3]);
            phase2 = m_CrystalStructures[m_CellPhases[m_Centers[gnum]]];
          }

          OrientationD axisAngle = m_OrientationOps[phase1]->calculateMisorientation(q1, q2);

          m_FeatureReferenceMisorientations[point] = SIMPLib::Constants::k_180OverPiD * axisAngle[3]; // convert to degrees
          idx = m_FeatureIds[point] * 2;
          avgMiso[idx + 0]++;
          avgMiso[idx + 1] = avgMiso[idx + 1] + m_FeatureReferenceMisorientations[point];
        }
        if(m_FeatureIds[point] == 0 || m_CellPhases[point] == 0)
        {
          m_FeatureReferenceMisorientations[point] = 0.0f;
        }
      }
    }
  }

  for(size_t i = 1; i < totalFeatures; i++)
  {
    idx = i * 2;
    m_FeatureAvgMisorientations[i] = avgMiso[idx + 1] / avgMiso[idx];
    if(avgMiso[idx] == 0.0f)
    {
      m_FeatureAvgMisorientations[i] = 0.0f;
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer FindFeatureReferenceMisorientations::newFilterInstance(bool copyFilterParameters) const
{
  FindFeatureReferenceMisorientations::Pointer filter = FindFeatureReferenceMisorientations::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindFeatureReferenceMisorientations::getCompiledLibraryName() const
{
  return OrientationAnalysisConstants::OrientationAnalysisBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindFeatureReferenceMisorientations::getBrandingString() const
{
  return "OrientationAnalysis";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindFeatureReferenceMisorientations::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream << OrientationAnalysis::Version::Major() << "." << OrientationAnalysis::Version::Minor() << "." << OrientationAnalysis::Version::Patch();
  return version;
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindFeatureReferenceMisorientations::getGroupName() const
{
  return SIMPL::FilterGroups::StatisticsFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid FindFeatureReferenceMisorientations::getUuid() const
{
  return QUuid("{428e1f5b-e6d8-5e8b-ad68-56ff14ee0e8c}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindFeatureReferenceMisorientations::getSubGroupName() const
{
  return SIMPL::FilterSubGroups::CrystallographyFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindFeatureReferenceMisorientations::getHumanLabel() const
{
  return "Find Feature Reference Misorientations";
}

// -----------------------------------------------------------------------------
FindFeatureReferenceMisorientations::Pointer FindFeatureReferenceMisorientations::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<FindFeatureReferenceMisorientations> FindFeatureReferenceMisorientations::New()
{
  struct make_shared_enabler : public FindFeatureReferenceMisorientations
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString FindFeatureReferenceMisorientations::getNameOfClass() const
{
  return QString("FindFeatureReferenceMisorientations");
}

// -----------------------------------------------------------------------------
QString FindFeatureReferenceMisorientations::ClassName()
{
  return QString("FindFeatureReferenceMisorientations");
}

// -----------------------------------------------------------------------------
void FindFeatureReferenceMisorientations::setFeatureIdsArrayPath(const DataArrayPath& value)
{
  m_FeatureIdsArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FindFeatureReferenceMisorientations::getFeatureIdsArrayPath() const
{
  return m_FeatureIdsArrayPath;
}

// -----------------------------------------------------------------------------
void FindFeatureReferenceMisorientations::setCellPhasesArrayPath(const DataArrayPath& value)
{
  m_CellPhasesArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FindFeatureReferenceMisorientations::getCellPhasesArrayPath() const
{
  return m_CellPhasesArrayPath;
}

// -----------------------------------------------------------------------------
void FindFeatureReferenceMisorientations::setCrystalStructuresArrayPath(const DataArrayPath& value)
{
  m_CrystalStructuresArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FindFeatureReferenceMisorientations::getCrystalStructuresArrayPath() const
{
  return m_CrystalStructuresArrayPath;
}

// -----------------------------------------------------------------------------
void FindFeatureReferenceMisorientations::setQuatsArrayPath(const DataArrayPath& value)
{
  m_QuatsArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FindFeatureReferenceMisorientations::getQuatsArrayPath() const
{
  return m_QuatsArrayPath;
}

// -----------------------------------------------------------------------------
void FindFeatureReferenceMisorientations::setAvgQuatsArrayPath(const DataArrayPath& value)
{
  m_AvgQuatsArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FindFeatureReferenceMisorientations::getAvgQuatsArrayPath() const
{
  return m_AvgQuatsArrayPath;
}

// -----------------------------------------------------------------------------
void FindFeatureReferenceMisorientations::setGBEuclideanDistancesArrayPath(const DataArrayPath& value)
{
  m_GBEuclideanDistancesArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FindFeatureReferenceMisorientations::getGBEuclideanDistancesArrayPath() const
{
  return m_GBEuclideanDistancesArrayPath;
}

// -----------------------------------------------------------------------------
void FindFeatureReferenceMisorientations::setFeatureAvgMisorientationsArrayName(const QString& value)
{
  m_FeatureAvgMisorientationsArrayName = value;
}

// -----------------------------------------------------------------------------
QString FindFeatureReferenceMisorientations::getFeatureAvgMisorientationsArrayName() const
{
  return m_FeatureAvgMisorientationsArrayName;
}

// -----------------------------------------------------------------------------
void FindFeatureReferenceMisorientations::setFeatureReferenceMisorientationsArrayName(const QString& value)
{
  m_FeatureReferenceMisorientationsArrayName = value;
}

// -----------------------------------------------------------------------------
QString FindFeatureReferenceMisorientations::getFeatureReferenceMisorientationsArrayName() const
{
  return m_FeatureReferenceMisorientationsArrayName;
}

// -----------------------------------------------------------------------------
void FindFeatureReferenceMisorientations::setReferenceOrientation(int value)
{
  m_ReferenceOrientation = value;
}

// -----------------------------------------------------------------------------
int FindFeatureReferenceMisorientations::getReferenceOrientation() const
{
  return m_ReferenceOrientation;
}
