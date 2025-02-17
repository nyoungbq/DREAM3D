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

#include "FindFeatureReferenceCAxisMisorientations.h"

#include <QtCore/QTextStream>

#include <Eigen/Dense>

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/DataContainers/DataContainer.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedPathCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"
#include "SIMPLib/Math/GeometryMath.h"
#include "SIMPLib/Math/MatrixMath.h"
#include "SIMPLib/Math/SIMPLibMath.h"

#include "EbsdLib/Core/Orientation.hpp"
#include "EbsdLib/Core/OrientationTransformation.hpp"
#include "EbsdLib/Core/Quaternion.hpp"

#include "OrientationAnalysis/OrientationAnalysisConstants.h"
#include "OrientationAnalysis/OrientationAnalysisUtilities.h"
#include "OrientationAnalysis/OrientationAnalysisVersion.h"

using QuatF = Quaternion<float>;
using namespace OrientationUtilities;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindFeatureReferenceCAxisMisorientations::FindFeatureReferenceCAxisMisorientations() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindFeatureReferenceCAxisMisorientations::~FindFeatureReferenceCAxisMisorientations() = default;
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindFeatureReferenceCAxisMisorientations::setupFilterParameters()
{
  FilterParameterVectorType parameters;
  parameters.push_back(SeparatorFilterParameter::Create("Cell Data", FilterParameter::Category::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::Int32, 1, AttributeMatrix::Type::Cell, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Feature Ids", FeatureIdsArrayPath, FilterParameter::Category::RequiredArray, FindFeatureReferenceCAxisMisorientations, req));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::Int32, 1, AttributeMatrix::Type::Cell, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Phases", CellPhasesArrayPath, FilterParameter::Category::RequiredArray, FindFeatureReferenceCAxisMisorientations, req));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::Float, 4, AttributeMatrix::Type::Cell, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Quaternions", QuatsArrayPath, FilterParameter::Category::RequiredArray, FindFeatureReferenceCAxisMisorientations, req));
  }
  parameters.push_back(SeparatorFilterParameter::Create("Cell Feature Data", FilterParameter::Category::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req =
        DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::Float, 3, AttributeMatrix::Type::CellFeature, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Average C-Axes", AvgCAxesArrayPath, FilterParameter::Category::RequiredArray, FindFeatureReferenceCAxisMisorientations, req));
  }
  parameters.push_back(SeparatorFilterParameter::Create("Cell Data", FilterParameter::Category::CreatedArray));
  parameters.push_back(SIMPL_NEW_DA_WITH_LINKED_AM_FP("Average C-Axis Misorientations", FeatureAvgCAxisMisorientationsArrayName, AvgCAxesArrayPath, AvgCAxesArrayPath,
                                                      FilterParameter::Category::CreatedArray, FindFeatureReferenceCAxisMisorientations));
  parameters.push_back(SeparatorFilterParameter::Create("Cell Feature Data", FilterParameter::Category::CreatedArray));
  parameters.push_back(SIMPL_NEW_DA_WITH_LINKED_AM_FP("Feature Stdev C-Axis Misorientations", FeatureStdevCAxisMisorientationsArrayName, AvgCAxesArrayPath, AvgCAxesArrayPath,
                                                      FilterParameter::Category::CreatedArray, FindFeatureReferenceCAxisMisorientations));
  parameters.push_back(SIMPL_NEW_DA_WITH_LINKED_AM_FP("Feature Reference C-Axis Misorientations", FeatureReferenceCAxisMisorientationsArrayName, FeatureIdsArrayPath, FeatureIdsArrayPath,
                                                      FilterParameter::Category::CreatedArray, FindFeatureReferenceCAxisMisorientations));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
void FindFeatureReferenceCAxisMisorientations::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setFeatureReferenceCAxisMisorientationsArrayName(reader->readString("FeatureReferenceCAxisMisorientationsArrayName", getFeatureReferenceCAxisMisorientationsArrayName()));
  setFeatureStdevCAxisMisorientationsArrayName(reader->readString("FeatureStdevCAxisMisorientationsArrayName", getFeatureStdevCAxisMisorientationsArrayName()));
  setFeatureAvgCAxisMisorientationsArrayName(reader->readString("FeatureAvgCAxisMisorientationsArrayName", getFeatureAvgCAxisMisorientationsArrayName()));
  setQuatsArrayPath(reader->readDataArrayPath("QuatsArrayPath", getQuatsArrayPath()));
  setAvgCAxesArrayPath(reader->readDataArrayPath("AvgCAxesArrayPath", getAvgCAxesArrayPath()));
  setCellPhasesArrayPath(reader->readDataArrayPath("CellPhasesArrayPath", getCellPhasesArrayPath()));
  setFeatureIdsArrayPath(reader->readDataArrayPath("FeatureIdsArrayPath", getFeatureIdsArrayPath()));
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindFeatureReferenceCAxisMisorientations::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindFeatureReferenceCAxisMisorientations::dataCheck()
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

  tempPath.update(m_AvgCAxesArrayPath.getDataContainerName(), getAvgCAxesArrayPath().getAttributeMatrixName(), getFeatureAvgCAxisMisorientationsArrayName());
  m_FeatureAvgCAxisMisorientationsPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<float>>(this, tempPath, 0, cDims);
  if(nullptr != m_FeatureAvgCAxisMisorientationsPtr.lock())
  {
    m_FeatureAvgCAxisMisorientations = m_FeatureAvgCAxisMisorientationsPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */

  tempPath.update(m_AvgCAxesArrayPath.getDataContainerName(), getAvgCAxesArrayPath().getAttributeMatrixName(), getFeatureStdevCAxisMisorientationsArrayName());
  m_FeatureStdevCAxisMisorientationsPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<float>>(this, tempPath, 0, cDims);
  if(nullptr != m_FeatureStdevCAxisMisorientationsPtr.lock())
  {
    m_FeatureStdevCAxisMisorientations = m_FeatureStdevCAxisMisorientationsPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */

  tempPath.update(m_FeatureIdsArrayPath.getDataContainerName(), getFeatureIdsArrayPath().getAttributeMatrixName(), getFeatureReferenceCAxisMisorientationsArrayName());
  m_FeatureReferenceCAxisMisorientationsPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<float>>(this, tempPath, 0, cDims);
  if(nullptr != m_FeatureReferenceCAxisMisorientationsPtr.lock())
  {
    m_FeatureReferenceCAxisMisorientations = m_FeatureReferenceCAxisMisorientationsPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */

  cDims[0] = 3;
  m_AvgCAxesPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<float>>(this, getAvgCAxesArrayPath(), cDims);
  if(nullptr != m_AvgCAxesPtr.lock())
  {
    m_AvgCAxes = m_AvgCAxesPtr.lock()->getPointer(0);
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

  getDataContainerArray()->validateNumberOfTuples(this, dataArrayPaths);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindFeatureReferenceCAxisMisorientations::execute()
{
  dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(m_FeatureIdsArrayPath.getDataContainerName());
  size_t totalPoints = m_FeatureIdsPtr.lock()->getNumberOfTuples();
  size_t totalFeatures = m_AvgCAxesPtr.lock()->getNumberOfTuples();

  int32_t avgMisoComps = 3;
  std::vector<size_t> dims(1, avgMisoComps);
  FloatArrayType::Pointer avgmisoPtr = FloatArrayType::CreateArray(totalFeatures, dims, "_INTERNAL_USE_ONLY_AvgMiso_Temp", true);
  avgmisoPtr->initializeWithZeros();
  float* avgmiso = avgmisoPtr->getPointer(0);

  FloatArrayType::Pointer quatsPtr = m_QuatsPtr.lock();

  float w = 0.0f;
  SizeVec3Type udims = m->getGeometryAs<ImageGeom>()->getDimensions();

  uint32_t maxUInt32 = std::numeric_limits<uint32_t>::max();
  // We have more points than can be allocated on a 32 bit machine. Assert Now.
  if(totalPoints > maxUInt32)
  {
    QString ss = QObject::tr("The volume is too large for a 32 bit machine. Try reducing the input volume size. Total Voxels: %1").arg(totalPoints);
    notifyStatusMessage(ss);
    return;
  }

  int64_t xPoints = static_cast<int64_t>(udims[0]);
  int64_t yPoints = static_cast<int64_t>(udims[1]);
  int64_t zPoints = static_cast<int64_t>(udims[2]);
  int64_t point = 0;

  Matrix3fR g1;
  g1.fill(0.0f);
  Matrix3fR g1T;
  g1T.fill(0.0f);
  const Eigen::Vector3f cAxis{0.0f, 0.0f, 1.0f};
  Eigen::Vector3f c1{0.0f, 0.0f, 0.0f};
  Eigen::Vector3f AvgCAxis = {0.0f, 0.0f, 0.0f};
  size_t index = 0;
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
          OrientationF oMatrix = OrientationTransformation::qu2om<QuatF, Orientation<float>>({currentQuatPtr[0], currentQuatPtr[1], currentQuatPtr[2], currentQuatPtr[3]});
          // g1 = OrientationMatrixToGMatrix(oMatrix);

          // transpose the g matricies so when caxis is multiplied by it
          // it will give the sample direction that the caxis is along
          g1T = OrientationMatrixToGMatrixTranspose(oMatrix);
          // g1T = g1.transpose();
          c1 = g1T * cAxis;
          // normalize so that the magnitude is 1
          c1.normalize();

          AvgCAxis[0] = m_AvgCAxes[3 * m_FeatureIds[point]];
          AvgCAxis[1] = m_AvgCAxes[3 * m_FeatureIds[point] + 1];
          AvgCAxis[2] = m_AvgCAxes[3 * m_FeatureIds[point] + 2];
          // normalize so that the magnitude is 1
          AvgCAxis.normalize();
          w = GeometryMath::CosThetaBetweenVectors(c1, AvgCAxis);
          w = std::clamp(w, -1.0f, 1.0f);
          w = acosf(w);
          w = w * SIMPLib::Constants::k_180OverPiF;
          if(w > 90.0f)
          {
            w = 180.0f - w;
          }

          m_FeatureReferenceCAxisMisorientations[point] = w;
          index = m_FeatureIds[point] * avgMisoComps;
          avgmiso[index]++;
          avgmiso[index + 1] += w;
        }
        if(m_FeatureIds[point] == 0 || m_CellPhases[point] == 0)
        {
          m_FeatureReferenceCAxisMisorientations[point] = 0;
        }
      }
    }
  }

  for(size_t i = 1; i < totalFeatures; i++)
  {
    if(i % 1000 == 0)
    {
      QString ss = QObject::tr("Working On Feature %1 of %2").arg(i).arg(totalFeatures);
      notifyStatusMessage(ss);
    }
    index = i * avgMisoComps;
    // m_FeatureAvgCAxisMisorientations[i] = avgmiso[index + 1] / avgmiso[index];
    if(avgmiso[index] == 0.0f)
    {
      m_FeatureAvgCAxisMisorientations[i] = 0.0;
    }
    else
    {
      m_FeatureAvgCAxisMisorientations[i] = avgmiso[index + 1] / avgmiso[index];
    }
  }

  int32_t gNum = 0;
  for(size_t j = 0; j < totalPoints; j++)
  {
    gNum = m_FeatureIds[j];
    avgmiso[(gNum * avgMisoComps) + 2] +=
        ((m_FeatureReferenceCAxisMisorientations[j] - m_FeatureAvgCAxisMisorientations[gNum]) * (m_FeatureReferenceCAxisMisorientations[j] - m_FeatureAvgCAxisMisorientations[gNum]));
  }

  for(size_t i = 1; i < totalFeatures; i++)
  {
    index = i * avgMisoComps;
    if(avgmiso[index] == 0.0f)
    {
      m_FeatureStdevCAxisMisorientations[i] = 0.0f;
    }
    else
    {
      m_FeatureStdevCAxisMisorientations[i] = sqrtf((1 / avgmiso[index]) * avgmiso[index + 2]);
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer FindFeatureReferenceCAxisMisorientations::newFilterInstance(bool copyFilterParameters) const
{
  FindFeatureReferenceCAxisMisorientations::Pointer filter = FindFeatureReferenceCAxisMisorientations::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindFeatureReferenceCAxisMisorientations::getCompiledLibraryName() const
{
  return OrientationAnalysisConstants::OrientationAnalysisBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindFeatureReferenceCAxisMisorientations::getBrandingString() const
{
  return "OrientationAnalysis";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindFeatureReferenceCAxisMisorientations::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream << OrientationAnalysis::Version::Major() << "." << OrientationAnalysis::Version::Minor() << "." << OrientationAnalysis::Version::Patch();
  return version;
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindFeatureReferenceCAxisMisorientations::getGroupName() const
{
  return SIMPL::FilterGroups::StatisticsFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid FindFeatureReferenceCAxisMisorientations::getUuid() const
{
  return QUuid("{1a0848da-2edd-52c0-b111-62a4dc6d2886}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindFeatureReferenceCAxisMisorientations::getSubGroupName() const
{
  return SIMPL::FilterSubGroups::CrystallographyFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindFeatureReferenceCAxisMisorientations::getHumanLabel() const
{
  return "Find Feature Reference C-Axis Misalignments";
}

// -----------------------------------------------------------------------------
FindFeatureReferenceCAxisMisorientations::Pointer FindFeatureReferenceCAxisMisorientations::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<FindFeatureReferenceCAxisMisorientations> FindFeatureReferenceCAxisMisorientations::New()
{
  struct make_shared_enabler : public FindFeatureReferenceCAxisMisorientations
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString FindFeatureReferenceCAxisMisorientations::getNameOfClass() const
{
  return QString("FindFeatureReferenceCAxisMisorientations");
}

// -----------------------------------------------------------------------------
QString FindFeatureReferenceCAxisMisorientations::ClassName()
{
  return QString("FindFeatureReferenceCAxisMisorientations");
}

// -----------------------------------------------------------------------------
void FindFeatureReferenceCAxisMisorientations::setFeatureIdsArrayPath(const DataArrayPath& value)
{
  m_FeatureIdsArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FindFeatureReferenceCAxisMisorientations::getFeatureIdsArrayPath() const
{
  return m_FeatureIdsArrayPath;
}

// -----------------------------------------------------------------------------
void FindFeatureReferenceCAxisMisorientations::setCellPhasesArrayPath(const DataArrayPath& value)
{
  m_CellPhasesArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FindFeatureReferenceCAxisMisorientations::getCellPhasesArrayPath() const
{
  return m_CellPhasesArrayPath;
}

// -----------------------------------------------------------------------------
void FindFeatureReferenceCAxisMisorientations::setAvgCAxesArrayPath(const DataArrayPath& value)
{
  m_AvgCAxesArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FindFeatureReferenceCAxisMisorientations::getAvgCAxesArrayPath() const
{
  return m_AvgCAxesArrayPath;
}

// -----------------------------------------------------------------------------
void FindFeatureReferenceCAxisMisorientations::setQuatsArrayPath(const DataArrayPath& value)
{
  m_QuatsArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FindFeatureReferenceCAxisMisorientations::getQuatsArrayPath() const
{
  return m_QuatsArrayPath;
}

// -----------------------------------------------------------------------------
void FindFeatureReferenceCAxisMisorientations::setFeatureAvgCAxisMisorientationsArrayName(const QString& value)
{
  m_FeatureAvgCAxisMisorientationsArrayName = value;
}

// -----------------------------------------------------------------------------
QString FindFeatureReferenceCAxisMisorientations::getFeatureAvgCAxisMisorientationsArrayName() const
{
  return m_FeatureAvgCAxisMisorientationsArrayName;
}

// -----------------------------------------------------------------------------
void FindFeatureReferenceCAxisMisorientations::setFeatureStdevCAxisMisorientationsArrayName(const QString& value)
{
  m_FeatureStdevCAxisMisorientationsArrayName = value;
}

// -----------------------------------------------------------------------------
QString FindFeatureReferenceCAxisMisorientations::getFeatureStdevCAxisMisorientationsArrayName() const
{
  return m_FeatureStdevCAxisMisorientationsArrayName;
}

// -----------------------------------------------------------------------------
void FindFeatureReferenceCAxisMisorientations::setFeatureReferenceCAxisMisorientationsArrayName(const QString& value)
{
  m_FeatureReferenceCAxisMisorientationsArrayName = value;
}

// -----------------------------------------------------------------------------
QString FindFeatureReferenceCAxisMisorientations::getFeatureReferenceCAxisMisorientationsArrayName() const
{
  return m_FeatureReferenceCAxisMisorientationsArrayName;
}
