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

#include "GenerateVectorColors.h"

#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Eigen>

#include <QtCore/QTextStream>

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedBooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedPathCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/Math/SIMPLibMath.h"
#include "SIMPLib/Utilities/ColorTable.h"

#include "Generic/GenericConstants.h"
#include "Generic/GenericVersion.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
GenerateVectorColors::GenerateVectorColors() = default;
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
GenerateVectorColors::~GenerateVectorColors() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void GenerateVectorColors::setupFilterParameters()
{
  FilterParameterVectorType parameters;
  std::vector<QString> linkedProps = {"GoodVoxelsArrayPath"};
  parameters.push_back(SIMPL_NEW_LINKED_BOOL_FP("Apply to Good Voxels Only (Bad Voxels Will Be Black)", UseGoodVoxels, FilterParameter::Category::Parameter, GenerateVectorColors, linkedProps));
  parameters.push_back(SeparatorFilterParameter::Create("Element Data", FilterParameter::Category::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::Float, 3, AttributeMatrix::Category::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Vector Attribute Array", VectorsArrayPath, FilterParameter::Category::RequiredArray, GenerateVectorColors, req));
  }

  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::Bool, 1, AttributeMatrix::Category::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Mask", GoodVoxelsArrayPath, FilterParameter::Category::RequiredArray, GenerateVectorColors, req));
  }
  parameters.push_back(SeparatorFilterParameter::Create("Element Data", FilterParameter::Category::CreatedArray));
  parameters.push_back(SIMPL_NEW_DA_WITH_LINKED_AM_FP("Vector Colors", CellVectorColorsArrayName, VectorsArrayPath, VectorsArrayPath, FilterParameter::Category::CreatedArray, GenerateVectorColors));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void GenerateVectorColors::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setUseGoodVoxels(reader->readValue("UseGoodVoxels", getUseGoodVoxels()));
  setCellVectorColorsArrayName(reader->readString("CellVectorColorsArrayName", getCellVectorColorsArrayName()));
  setGoodVoxelsArrayPath(reader->readDataArrayPath("GoodVoxelsArrayPath", getGoodVoxelsArrayPath()));
  setVectorsArrayPath(reader->readDataArrayPath("VectorsArrayPath", getVectorsArrayPath()));
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void GenerateVectorColors::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void GenerateVectorColors::dataCheck()
{
  clearErrorCode();
  clearWarningCode();

  DataArrayPath tempPath;

  QVector<DataArrayPath> dataArrayPaths;

  std::vector<size_t> cDims(1, 3);
  m_VectorsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<float>>(this, getVectorsArrayPath(), cDims);
  if(nullptr != m_VectorsPtr.lock())
  {
    m_Vectors = m_VectorsPtr.lock()->getPointer(0);
  }
  if(getErrorCode() >= 0)
  {
    dataArrayPaths.push_back(getVectorsArrayPath());
  };

  tempPath.update(getVectorsArrayPath().getDataContainerName(), getVectorsArrayPath().getAttributeMatrixName(), getCellVectorColorsArrayName());
  m_CellVectorColorsPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<uint8_t>>(this, tempPath, 0, cDims);
  if(nullptr != m_CellVectorColorsPtr.lock())
  {
    m_CellVectorColors = m_CellVectorColorsPtr.lock()->getPointer(0);
  }

  // The good voxels array is optional, If it is available we are going to use it, otherwise we are going to ignore it
  if(getUseGoodVoxels())
  {
    // The good voxels array is optional, If it is available we are going to use it, otherwise we are going to create it
    cDims[0] = 1;
    m_GoodVoxelsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<bool>>(this, getGoodVoxelsArrayPath(), cDims);
    if(nullptr != m_GoodVoxelsPtr.lock())
    {
      m_GoodVoxels = m_GoodVoxelsPtr.lock()->getPointer(0);
    } /* Now assign the raw pointer to data from the DataArray<T> object */
    if(getErrorCode() >= 0)
    {
      dataArrayPaths.push_back(getGoodVoxelsArrayPath());
    };
  }
  else
  {
    m_GoodVoxels = nullptr;
  }

  getDataContainerArray()->validateNumberOfTuples(this, dataArrayPaths);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void GenerateVectorColors::execute()
{
  dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  // typedef Eigen::Array<float, 3, 1> ArrayType;
  typedef Eigen::Map<Eigen::Vector3f> VectorMapType;

  size_t totalPoints = m_VectorsPtr.lock()->getNumberOfTuples();

  bool missingGoodVoxels = true;
  if(nullptr != m_GoodVoxels)
  {
    missingGoodVoxels = false;
  }

  size_t index = 0;

  // Write the Vector Coloring Cell Data
  for(size_t i = 0; i < totalPoints; i++)
  {
    index = i * 3;
    m_CellVectorColors[index] = 0;
    m_CellVectorColors[index + 1] = 0;
    m_CellVectorColors[index + 2] = 0;

    float dir[3] = {0.0f, 0.0f, 0.0f};
    float r = 0, g = 0, b = 0;
    SIMPL::Rgb argb;
    if(missingGoodVoxels || m_GoodVoxels[i])
    {
      dir[0] = m_Vectors[index + 0];
      dir[1] = m_Vectors[index + 1];
      dir[2] = m_Vectors[index + 2];

      VectorMapType array(const_cast<float*>(dir));
      array.normalize();

      if(dir[2] < 0)
      {
        array = array * -1.0f;
      }
      float trend = atan2f(dir[1], dir[0]) * (SIMPLib::Constants::k_RadToDegF);
      float plunge = acosf(dir[2]) * (SIMPLib::Constants::k_RadToDegF);
      if(trend < 0.0)
      {
        trend += 360.0;
      }
      if(trend <= 120.0)
      {
        r = 255.0 * ((120.0 - trend) / 120.0);
        g = 255.0 * (trend / 120.0);
        b = 0.0;
      }
      if(trend > 120.0 && trend <= 240.0)
      {
        trend -= 120.0;
        r = 0.0;
        g = 255.0 * ((120.0 - trend) / 120.0);
        b = 255.0 * (trend / 120.0);
      }
      if(trend > 240.0 && trend < 360.0)
      {
        trend -= 240.0;
        r = 255.0 * (trend / 120.0);
        g = 0.0;
        b = 255.0 * ((120.0 - trend) / 120.0);
      }
      float deltaR = 255.0 - r;
      float deltaG = 255.0 - g;
      float deltaB = 255.0 - b;
      r += (deltaR * ((90.0 - plunge) / 90.0));
      g += (deltaG * ((90.0 - plunge) / 90.0));
      b += (deltaB * ((90.0 - plunge) / 90.0));
      if(r > 255.0)
      {
        r = 255.0;
      }
      if(g > 255.0)
      {
        g = 255.0;
      }
      if(b > 255.0)
      {
        b = 255.0;
      }
      argb = RgbColor::dRgb(r, g, b, 255);
      m_CellVectorColors[index] = RgbColor::dRed(argb);
      m_CellVectorColors[index + 1] = RgbColor::dGreen(argb);
      m_CellVectorColors[index + 2] = RgbColor::dBlue(argb);
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer GenerateVectorColors::newFilterInstance(bool copyFilterParameters) const
{
  GenerateVectorColors::Pointer filter = GenerateVectorColors::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString GenerateVectorColors::getCompiledLibraryName() const
{
  return GenericConstants::GenericBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString GenerateVectorColors::getBrandingString() const
{
  return "Generic";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString GenerateVectorColors::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream << Generic::Version::Major() << "." << Generic::Version::Minor() << "." << Generic::Version::Patch();
  return version;
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString GenerateVectorColors::getGroupName() const
{
  return SIMPL::FilterGroups::Generic;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid GenerateVectorColors::getUuid() const
{
  return QUuid("{ef28de7e-5bdd-57c2-9318-60ba0dfaf7bc}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString GenerateVectorColors::getSubGroupName() const
{
  return SIMPL::FilterSubGroups::CrystallographyFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString GenerateVectorColors::getHumanLabel() const
{
  return "Generate Vector Colors";
}

// -----------------------------------------------------------------------------
GenerateVectorColors::Pointer GenerateVectorColors::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<GenerateVectorColors> GenerateVectorColors::New()
{
  struct make_shared_enabler : public GenerateVectorColors
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString GenerateVectorColors::getNameOfClass() const
{
  return QString("GenerateVectorColors");
}

// -----------------------------------------------------------------------------
QString GenerateVectorColors::ClassName()
{
  return QString("GenerateVectorColors");
}

// -----------------------------------------------------------------------------
void GenerateVectorColors::setVectorsArrayPath(const DataArrayPath& value)
{
  m_VectorsArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath GenerateVectorColors::getVectorsArrayPath() const
{
  return m_VectorsArrayPath;
}

// -----------------------------------------------------------------------------
void GenerateVectorColors::setGoodVoxelsArrayPath(const DataArrayPath& value)
{
  m_GoodVoxelsArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath GenerateVectorColors::getGoodVoxelsArrayPath() const
{
  return m_GoodVoxelsArrayPath;
}

// -----------------------------------------------------------------------------
void GenerateVectorColors::setCellVectorColorsArrayName(const QString& value)
{
  m_CellVectorColorsArrayName = value;
}

// -----------------------------------------------------------------------------
QString GenerateVectorColors::getCellVectorColorsArrayName() const
{
  return m_CellVectorColorsArrayName;
}

// -----------------------------------------------------------------------------
void GenerateVectorColors::setUseGoodVoxels(bool value)
{
  m_UseGoodVoxels = value;
}

// -----------------------------------------------------------------------------
bool GenerateVectorColors::getUseGoodVoxels() const
{
  return m_UseGoodVoxels;
}
