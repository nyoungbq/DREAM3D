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
#include "FitFeatureData.h"

#include <QtCore/QTextStream>

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/Common/TemplateHelpers.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/ChoiceFilterParameter.h"
#include "SIMPLib/FilterParameters/DataArrayCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedBooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"

#include "StatsToolbox/DistributionAnalysisOps/BetaOps.h"
#include "StatsToolbox/DistributionAnalysisOps/LogNormalOps.h"
#include "StatsToolbox/DistributionAnalysisOps/PowerLawOps.h"
#include "StatsToolbox/StatsToolboxConstants.h"
#include "StatsToolbox/StatsToolboxVersion.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FitFeatureData::FitFeatureData() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FitFeatureData::~FitFeatureData() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FitFeatureData::setupFilterParameters()
{
  FilterParameterVectorType parameters;
  {
    ChoiceFilterParameter::Pointer parameter = ChoiceFilterParameter::New();
    parameter->setHumanLabel("Distribution Type");
    parameter->setPropertyName("DistributionType");
    parameter->setSetterCallback(SIMPL_BIND_SETTER(FitFeatureData, this, DistributionType));
    parameter->setGetterCallback(SIMPL_BIND_GETTER(FitFeatureData, this, DistributionType));

    std::vector<QString> choices;
    choices.push_back("Beta");
    choices.push_back("Log-Normal");
    choices.push_back("Power Law");
    parameter->setChoices(choices);
    parameter->setCategory(FilterParameter::Category::Parameter);
    parameters.push_back(parameter);
  }
  std::vector<QString> linkedProps = {"BiasedFeaturesArrayPath"};
  parameters.push_back(SIMPL_NEW_LINKED_BOOL_FP("Remove Biased Features", RemoveBiasedFeatures, FilterParameter::Category::Parameter, FitFeatureData, linkedProps));
  parameters.push_back(SeparatorFilterParameter::Create("Feature Data", FilterParameter::Category::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::Defaults::AnyPrimitive, 1, AttributeMatrix::Category::Feature);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Feature Array to Fit", SelectedFeatureArrayPath, FilterParameter::Category::RequiredArray, FitFeatureData, req));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::Int32, 1, AttributeMatrix::Category::Feature);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Phases", FeaturePhasesArrayPath, FilterParameter::Category::RequiredArray, FitFeatureData, req));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::Bool, 1, AttributeMatrix::Category::Feature);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Biased Features", BiasedFeaturesArrayPath, FilterParameter::Category::RequiredArray, FitFeatureData, req));
  }
  parameters.push_back(SeparatorFilterParameter::Create("Ensemble Data", FilterParameter::Category::CreatedArray));
  {
    DataArrayCreationFilterParameter::RequirementType req = DataArrayCreationFilterParameter::CreateRequirement(AttributeMatrix::Category::Ensemble);
    parameters.push_back(SIMPL_NEW_DA_CREATION_FP("Fit Parameters", NewEnsembleArrayArray, FilterParameter::Category::CreatedArray, FitFeatureData, req));
  }
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FitFeatureData::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setNewEnsembleArrayArray(reader->readDataArrayPath("NewEnsembleArrayArray", getNewEnsembleArrayArray()));
  setBiasedFeaturesArrayPath(reader->readDataArrayPath("BiasedFeaturesArrayPath", getBiasedFeaturesArrayPath()));
  setFeaturePhasesArrayPath(reader->readDataArrayPath("FeaturePhasesArrayPath", getFeaturePhasesArrayPath()));
  setSelectedFeatureArrayPath(reader->readDataArrayPath("SelectedFeatureArrayPath", getSelectedFeatureArrayPath()));
  setDistributionType(reader->readValue("DistributionType", getDistributionType()));
  setRemoveBiasedFeatures(reader->readValue("RemoveBiasedFeatures", getRemoveBiasedFeatures()));
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FitFeatureData::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FitFeatureData::dataCheck()
{
  clearErrorCode();
  clearWarningCode();

  std::vector<size_t> cDims(1, 1);
  m_FeaturePhasesPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<int32_t>>(this, getFeaturePhasesArrayPath(), cDims);
  if(nullptr != m_FeaturePhasesPtr.lock())
  {
    m_FeaturePhases = m_FeaturePhasesPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */

  m_InDataArrayPtr = getDataContainerArray()->getPrereqIDataArrayFromPath(this, getSelectedFeatureArrayPath());

  int32_t numComp = 0;
  QString distType("UNKNOWN");
  // Determining number of components and name given distribution type
  if(m_DistributionType == SIMPL::DistributionType::Beta)
  {
    distType = "Beta", numComp = SIMPL::DistributionType::BetaColumnCount;
  }
  else if(m_DistributionType == SIMPL::DistributionType::LogNormal)
  {
    distType = "LogNormal", numComp = SIMPL::DistributionType::LogNormalColumnCount;
  }
  else if(m_DistributionType == SIMPL::DistributionType::Power)
  {
    distType = "PowerLaw", numComp = SIMPL::DistributionType::PowerLawColumnCount;
  }

  getNewEnsembleArrayArray().setDataArrayName(m_SelectedFeatureArrayPath.getDataArrayName() + distType + QString("Fit"));
  cDims[0] = numComp;
  m_NewEnsembleArrayPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<float>>(
      this, getNewEnsembleArrayArray(), 0, cDims); /* Assigns the shared_ptr<>(this, tempPath, 0, dims); Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if(nullptr != m_NewEnsembleArrayPtr.lock())
  {
    m_NewEnsembleArray = m_NewEnsembleArrayPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */

  if(m_RemoveBiasedFeatures)
  {
    cDims[0] = 1;
    m_BiasedFeaturesPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<bool>>(this, getBiasedFeaturesArrayPath(), cDims);
    if(nullptr != m_BiasedFeaturesPtr.lock())
    {
      m_BiasedFeatures = m_BiasedFeaturesPtr.lock()->getPointer(0);
    } /* Now assign the raw pointer to data from the DataArray<T> object */
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
template <typename T>
void fitData(IDataArray::Pointer inDataPtr, float* ensembleArray, int32_t* eIds, size_t numEnsembles, uint32_t dType, bool removeBiasedFeatures, bool* biasedFeatures)
{
  typename DataArray<T>::Pointer inputDataPtr = std::dynamic_pointer_cast<DataArray<T>>(inDataPtr);

  StatsData::Pointer sData = StatsData::New();

  std::vector<DistributionAnalysisOps::Pointer> distributionAnalysis;
  distributionAnalysis.push_back(BetaOps::New());
  distributionAnalysis.push_back(LogNormalOps::New());
  distributionAnalysis.push_back(PowerLawOps::New());

  QString distType;
  int32_t numComp = 1;

  // Determining number of components and name given distribution type
  if(dType == SIMPL::DistributionType::Beta)
  {
    distType = "Beta", numComp = SIMPL::DistributionType::BetaColumnCount;
  }
  else if(dType == SIMPL::DistributionType::LogNormal)
  {
    distType = "LogNormal", numComp = SIMPL::DistributionType::LogNormalColumnCount;
  }
  else if(dType == SIMPL::DistributionType::Power)
  {
    distType = "PowerLaw", numComp = SIMPL::DistributionType::PowerLawColumnCount;
  }

  T* fPtr = inputDataPtr->getPointer(0);

  std::vector<FloatArrayType::Pointer> dist;
  std::vector<std::vector<float>> values;

  size_t numfeatures = inputDataPtr->getNumberOfTuples();

  dist.resize(numEnsembles);
  values.resize(numEnsembles);

  for(size_t i = 1; i < numEnsembles; i++)
  {
    dist[i] = sData->CreateDistributionArrays(dType);
  }

  int32_t ensemble = 0;
  for(size_t i = 1; i < numfeatures; i++)
  {
    if(!removeBiasedFeatures || !biasedFeatures[i])
    {
      ensemble = eIds[i];
      values[ensemble].push_back(static_cast<float>(fPtr[i]));
    }
  }
  for(size_t i = 1; i < numEnsembles; i++)
  {
    distributionAnalysis[dType]->calculateParameters(values[i], dist[i]);
    for(int32_t j = 0; j < numComp; j++)
    {
      FloatArrayType::Pointer data = dist[i];
      ensembleArray[numComp * i + j] = data->getValue(j);
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FitFeatureData::execute()
{
  dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  size_t numEnsembles = m_NewEnsembleArrayPtr.lock()->getNumberOfTuples();

  EXECUTE_FUNCTION_TEMPLATE(this, fitData, m_InDataArrayPtr.lock(), m_InDataArrayPtr.lock(), m_NewEnsembleArray, m_FeaturePhases, numEnsembles, m_DistributionType, m_RemoveBiasedFeatures,
                            m_BiasedFeatures)
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer FitFeatureData::newFilterInstance(bool copyFilterParameters) const
{
  FitFeatureData::Pointer filter = FitFeatureData::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FitFeatureData::getCompiledLibraryName() const
{
  return StatsToolboxConstants::StatsToolboxBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FitFeatureData::getBrandingString() const
{
  return "Statistics";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FitFeatureData::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream << StatsToolbox::Version::Major() << "." << StatsToolbox::Version::Minor() << "." << StatsToolbox::Version::Patch();
  return version;
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FitFeatureData::getGroupName() const
{
  return SIMPL::FilterGroups::StatisticsFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid FitFeatureData::getUuid() const
{
  return QUuid("{6c255fc4-1692-57cf-be55-71dc4e05ec83}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FitFeatureData::getSubGroupName() const
{
  return SIMPL::FilterSubGroups::EnsembleStatsFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FitFeatureData::getHumanLabel() const
{
  return "Fit Distribution to Feature Data";
}

// -----------------------------------------------------------------------------
FitFeatureData::Pointer FitFeatureData::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<FitFeatureData> FitFeatureData::New()
{
  struct make_shared_enabler : public FitFeatureData
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString FitFeatureData::getNameOfClass() const
{
  return QString("FitFeatureData");
}

// -----------------------------------------------------------------------------
QString FitFeatureData::ClassName()
{
  return QString("FitFeatureData");
}

// -----------------------------------------------------------------------------
void FitFeatureData::setSelectedFeatureArrayPath(const DataArrayPath& value)
{
  m_SelectedFeatureArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FitFeatureData::getSelectedFeatureArrayPath() const
{
  return m_SelectedFeatureArrayPath;
}

// -----------------------------------------------------------------------------
void FitFeatureData::setDistributionType(unsigned int value)
{
  m_DistributionType = value;
}

// -----------------------------------------------------------------------------
unsigned int FitFeatureData::getDistributionType() const
{
  return m_DistributionType;
}

// -----------------------------------------------------------------------------
void FitFeatureData::setRemoveBiasedFeatures(bool value)
{
  m_RemoveBiasedFeatures = value;
}

// -----------------------------------------------------------------------------
bool FitFeatureData::getRemoveBiasedFeatures() const
{
  return m_RemoveBiasedFeatures;
}

// -----------------------------------------------------------------------------
void FitFeatureData::setFeaturePhasesArrayPath(const DataArrayPath& value)
{
  m_FeaturePhasesArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FitFeatureData::getFeaturePhasesArrayPath() const
{
  return m_FeaturePhasesArrayPath;
}

// -----------------------------------------------------------------------------
void FitFeatureData::setBiasedFeaturesArrayPath(const DataArrayPath& value)
{
  m_BiasedFeaturesArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FitFeatureData::getBiasedFeaturesArrayPath() const
{
  return m_BiasedFeaturesArrayPath;
}

// -----------------------------------------------------------------------------
void FitFeatureData::setNewEnsembleArrayArray(const DataArrayPath& value)
{
  m_NewEnsembleArrayArray = value;
}

// -----------------------------------------------------------------------------
DataArrayPath FitFeatureData::getNewEnsembleArrayArray() const
{
  return m_NewEnsembleArrayArray;
}
