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

#pragma once

#include <memory>

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/DataArrays/DataArray.hpp"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/Filtering/AbstractFilter.h"

#include "OrientationAnalysis/OrientationAnalysisDLLExport.h"

/**
 * @brief The FindGBCD class. See [Filter documentation](@ref findgbcd) for details.
 */
class OrientationAnalysis_EXPORT FindGBCD : public AbstractFilter
{
  Q_OBJECT

  // Start Python bindings declarations
  PYB11_BEGIN_BINDINGS(FindGBCD SUPERCLASS AbstractFilter)
  PYB11_FILTER()
  PYB11_SHARED_POINTERS(FindGBCD)
  PYB11_FILTER_NEW_MACRO(FindGBCD)
  PYB11_PROPERTY(QString FaceEnsembleAttributeMatrixName READ getFaceEnsembleAttributeMatrixName WRITE setFaceEnsembleAttributeMatrixName)
  PYB11_PROPERTY(float GBCDRes READ getGBCDRes WRITE setGBCDRes)
  PYB11_PROPERTY(DataArrayPath SurfaceMeshFaceLabelsArrayPath READ getSurfaceMeshFaceLabelsArrayPath WRITE setSurfaceMeshFaceLabelsArrayPath)
  PYB11_PROPERTY(DataArrayPath SurfaceMeshFaceNormalsArrayPath READ getSurfaceMeshFaceNormalsArrayPath WRITE setSurfaceMeshFaceNormalsArrayPath)
  PYB11_PROPERTY(DataArrayPath SurfaceMeshFaceAreasArrayPath READ getSurfaceMeshFaceAreasArrayPath WRITE setSurfaceMeshFaceAreasArrayPath)
  PYB11_PROPERTY(DataArrayPath FeatureEulerAnglesArrayPath READ getFeatureEulerAnglesArrayPath WRITE setFeatureEulerAnglesArrayPath)
  PYB11_PROPERTY(DataArrayPath FeaturePhasesArrayPath READ getFeaturePhasesArrayPath WRITE setFeaturePhasesArrayPath)
  PYB11_PROPERTY(DataArrayPath CrystalStructuresArrayPath READ getCrystalStructuresArrayPath WRITE setCrystalStructuresArrayPath)
  PYB11_PROPERTY(QString GBCDArrayName READ getGBCDArrayName WRITE setGBCDArrayName)
  PYB11_END_BINDINGS()
  // End Python bindings declarations

public:
  using Self = FindGBCD;
  using Pointer = std::shared_ptr<Self>;
  using ConstPointer = std::shared_ptr<const Self>;
  using WeakPointer = std::weak_ptr<Self>;
  using ConstWeakPointer = std::weak_ptr<const Self>;

  /**
   * @brief Returns a NullPointer wrapped by a shared_ptr<>
   * @return
   */
  static Pointer NullPointer();

  /**
   * @brief Creates a new object wrapped in a shared_ptr<>
   * @return
   */
  static Pointer New();

  /**
   * @brief Returns the name of the class for FindGBCD
   */
  QString getNameOfClass() const override;
  /**
   * @brief Returns the name of the class for FindGBCD
   */
  static QString ClassName();

  ~FindGBCD() override;

  /**
   * @brief Setter property for FaceEnsembleAttributeMatrixName
   */
  void setFaceEnsembleAttributeMatrixName(const QString& value);
  /**
   * @brief Getter property for FaceEnsembleAttributeMatrixName
   * @return Value of FaceEnsembleAttributeMatrixName
   */
  QString getFaceEnsembleAttributeMatrixName() const;
  Q_PROPERTY(QString FaceEnsembleAttributeMatrixName READ getFaceEnsembleAttributeMatrixName WRITE setFaceEnsembleAttributeMatrixName)

  /**
   * @brief Setter property for GBCDRes
   */
  void setGBCDRes(float value);
  /**
   * @brief Getter property for GBCDRes
   * @return Value of GBCDRes
   */
  float getGBCDRes() const;
  Q_PROPERTY(float GBCDRes READ getGBCDRes WRITE setGBCDRes)

  /**
   * @brief Setter property for SurfaceMeshFaceLabelsArrayPath
   */
  void setSurfaceMeshFaceLabelsArrayPath(const DataArrayPath& value);
  /**
   * @brief Getter property for SurfaceMeshFaceLabelsArrayPath
   * @return Value of SurfaceMeshFaceLabelsArrayPath
   */
  DataArrayPath getSurfaceMeshFaceLabelsArrayPath() const;
  Q_PROPERTY(DataArrayPath SurfaceMeshFaceLabelsArrayPath READ getSurfaceMeshFaceLabelsArrayPath WRITE setSurfaceMeshFaceLabelsArrayPath)

  /**
   * @brief Setter property for SurfaceMeshFaceNormalsArrayPath
   */
  void setSurfaceMeshFaceNormalsArrayPath(const DataArrayPath& value);
  /**
   * @brief Getter property for SurfaceMeshFaceNormalsArrayPath
   * @return Value of SurfaceMeshFaceNormalsArrayPath
   */
  DataArrayPath getSurfaceMeshFaceNormalsArrayPath() const;
  Q_PROPERTY(DataArrayPath SurfaceMeshFaceNormalsArrayPath READ getSurfaceMeshFaceNormalsArrayPath WRITE setSurfaceMeshFaceNormalsArrayPath)

  /**
   * @brief Setter property for SurfaceMeshFaceAreasArrayPath
   */
  void setSurfaceMeshFaceAreasArrayPath(const DataArrayPath& value);
  /**
   * @brief Getter property for SurfaceMeshFaceAreasArrayPath
   * @return Value of SurfaceMeshFaceAreasArrayPath
   */
  DataArrayPath getSurfaceMeshFaceAreasArrayPath() const;
  Q_PROPERTY(DataArrayPath SurfaceMeshFaceAreasArrayPath READ getSurfaceMeshFaceAreasArrayPath WRITE setSurfaceMeshFaceAreasArrayPath)

  /**
   * @brief Setter property for FeatureEulerAnglesArrayPath
   */
  void setFeatureEulerAnglesArrayPath(const DataArrayPath& value);
  /**
   * @brief Getter property for FeatureEulerAnglesArrayPath
   * @return Value of FeatureEulerAnglesArrayPath
   */
  DataArrayPath getFeatureEulerAnglesArrayPath() const;
  Q_PROPERTY(DataArrayPath FeatureEulerAnglesArrayPath READ getFeatureEulerAnglesArrayPath WRITE setFeatureEulerAnglesArrayPath)

  /**
   * @brief Setter property for FeaturePhasesArrayPath
   */
  void setFeaturePhasesArrayPath(const DataArrayPath& value);
  /**
   * @brief Getter property for FeaturePhasesArrayPath
   * @return Value of FeaturePhasesArrayPath
   */
  DataArrayPath getFeaturePhasesArrayPath() const;
  Q_PROPERTY(DataArrayPath FeaturePhasesArrayPath READ getFeaturePhasesArrayPath WRITE setFeaturePhasesArrayPath)

  /**
   * @brief Setter property for CrystalStructuresArrayPath
   */
  void setCrystalStructuresArrayPath(const DataArrayPath& value);
  /**
   * @brief Getter property for CrystalStructuresArrayPath
   * @return Value of CrystalStructuresArrayPath
   */
  DataArrayPath getCrystalStructuresArrayPath() const;
  Q_PROPERTY(DataArrayPath CrystalStructuresArrayPath READ getCrystalStructuresArrayPath WRITE setCrystalStructuresArrayPath)

  /**
   * @brief Setter property for GBCDArrayName
   */
  void setGBCDArrayName(const QString& value);
  /**
   * @brief Getter property for GBCDArrayName
   * @return Value of GBCDArrayName
   */
  QString getGBCDArrayName() const;
  Q_PROPERTY(QString GBCDArrayName READ getGBCDArrayName WRITE setGBCDArrayName)

  /**
   * @brief Setter property for GBCDArrayNames
   */
  void setGBCDArrayNames(const QVector<ComparisonInput_t>& value);
  /**
   * @brief Getter property for GBCDArrayNames
   * @return Value of GBCDArrayNames
   */
  QVector<ComparisonInput_t> getGBCDArrayNames() const;

  /**
   * @brief getCompiledLibraryName Reimplemented from @see AbstractFilter class
   */
  QString getCompiledLibraryName() const override;

  /**
   * @brief getBrandingString Returns the branding string for the filter, which is a tag
   * used to denote the filter's association with specific plugins
   * @return Branding string
   */
  QString getBrandingString() const override;

  /**
   * @brief getFilterVersion Returns a version string for this filter. Default
   * value is an empty string.
   * @return
   */
  QString getFilterVersion() const override;

  /**
   * @brief newFilterInstance Reimplemented from @see AbstractFilter class
   */
  AbstractFilter::Pointer newFilterInstance(bool copyFilterParameters) const override;

  /**
   * @brief getGroupName Reimplemented from @see AbstractFilter class
   */
  QString getGroupName() const override;

  /**
   * @brief getSubGroupName Reimplemented from @see AbstractFilter class
   */
  QString getSubGroupName() const override;

  /**
   * @brief getUuid Return the unique identifier for this filter.
   * @return A QUuid object.
   */
  QUuid getUuid() const override;

  /**
   * @brief getHumanLabel Reimplemented from @see AbstractFilter class
   */
  QString getHumanLabel() const override;

  /**
   * @brief setupFilterParameters Reimplemented from @see AbstractFilter class
   */
  void setupFilterParameters() override;

  /**
   * @brief readFilterParameters Reimplemented from @see AbstractFilter class
   */
  void readFilterParameters(AbstractFilterParametersReader* reader, int index) override;

  /**
   * @brief execute Reimplemented from @see AbstractFilter class
   */
  void execute() override;

protected:
  FindGBCD();

  /**
   * @brief dataCheck Checks for the appropriate parameter values and availability of arrays
   */
  void dataCheck() override;

  /**
   * @brief dataCheckSurfaceMesh Checks for the appropriate parameter values and availability of arrays
   */
  void dataCheckSurfaceMesh();

  /**
   * @brief dataCheckVoxel Checks for the appropriate parameter values and availability of arrays
   */
  void dataCheckVoxel();

  /**
   * @brief Initializes all the private instance variables.
   */
  void initialize();

  /**
   * @brief sizeGBCD Determines the sizing for the GBCD arrays
   * @param faceChunkSize Number of triangles per chunk
   * @param numMisoReps Dimensionality for bins
   */
  void sizeGBCD(size_t faceChunkSize, size_t numMisoReps);

private:
  std::weak_ptr<DataArray<double>> m_SurfaceMeshFaceAreasPtr;
  double* m_SurfaceMeshFaceAreas = nullptr;
  std::weak_ptr<DataArray<int32_t>> m_SurfaceMeshFaceLabelsPtr;
  int32_t* m_SurfaceMeshFaceLabels = nullptr;
  std::weak_ptr<DataArray<double>> m_SurfaceMeshFaceNormalsPtr;
  double* m_SurfaceMeshFaceNormals = nullptr;
  std::weak_ptr<DataArray<float>> m_FeatureEulerAnglesPtr;
  float* m_FeatureEulerAngles = nullptr;
  std::weak_ptr<DataArray<int32_t>> m_FeaturePhasesPtr;
  int32_t* m_FeaturePhases = nullptr;
  std::weak_ptr<DataArray<unsigned int>> m_CrystalStructuresPtr;
  unsigned int* m_CrystalStructures = nullptr;
  std::weak_ptr<DataArray<double>> m_GBCDPtr;
  double* m_GBCD = nullptr;

  QString m_FaceEnsembleAttributeMatrixName = {SIMPL::Defaults::FaceEnsembleAttributeMatrixName};
  float m_GBCDRes = {9.0f};
  DataArrayPath m_SurfaceMeshFaceLabelsArrayPath = {SIMPL::Defaults::TriangleDataContainerName, SIMPL::Defaults::FaceAttributeMatrixName, SIMPL::FaceData::SurfaceMeshFaceLabels};
  DataArrayPath m_SurfaceMeshFaceNormalsArrayPath = {SIMPL::Defaults::TriangleDataContainerName, SIMPL::Defaults::FaceAttributeMatrixName, SIMPL::FaceData::SurfaceMeshFaceNormals};
  DataArrayPath m_SurfaceMeshFaceAreasArrayPath = {SIMPL::Defaults::TriangleDataContainerName, SIMPL::Defaults::FaceAttributeMatrixName, SIMPL::FaceData::SurfaceMeshFaceAreas};
  DataArrayPath m_FeatureEulerAnglesArrayPath = {SIMPL::Defaults::ImageDataContainerName, SIMPL::Defaults::CellFeatureAttributeMatrixName, SIMPL::FeatureData::EulerAngles};
  DataArrayPath m_FeaturePhasesArrayPath = {SIMPL::Defaults::ImageDataContainerName, SIMPL::Defaults::CellFeatureAttributeMatrixName, SIMPL::FeatureData::Phases};
  DataArrayPath m_CrystalStructuresArrayPath = {SIMPL::Defaults::ImageDataContainerName, SIMPL::Defaults::CellEnsembleAttributeMatrixName, SIMPL::EnsembleData::CrystalStructures};
  QString m_GBCDArrayName = {SIMPL::EnsembleData::GBCD};
  QVector<ComparisonInput_t> m_GBCDArrayNames = {};

  FloatArrayType::Pointer m_GbcdDeltasArray;
  Int32ArrayType::Pointer m_GbcdSizesArray;
  FloatArrayType::Pointer m_GbcdLimitsArray;
  Int32ArrayType::Pointer m_GbcdBinsArray;
  BoolArrayType::Pointer m_GbcdHemiCheckArray;

  float* m_GbcdDeltas;
  int32_t* m_GbcdSizes;
  float* m_GbcdLimits;
  int32_t* m_GbcdBins;
  bool* m_HemiCheck;

public:
  FindGBCD(const FindGBCD&) = delete;            // Copy Constructor Not Implemented
  FindGBCD(FindGBCD&&) = delete;                 // Move Constructor Not Implemented
  FindGBCD& operator=(const FindGBCD&) = delete; // Copy Assignment Not Implemented
  FindGBCD& operator=(FindGBCD&&) = delete;      // Move Assignment Not Implemented
};
