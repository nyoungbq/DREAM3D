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

#include <QtCore/QObject>

#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/FilterParameter.h"

#include "SVWidgetsLib/FilterParameterWidgets/FilterParameterWidget.h"
#include "SVWidgetsLib/SVWidgetsLib.h"

#include "SyntheticBuilding/ui_InitializeSyntheticVolumeWidget.h"

class QComboBox;
class InitializeSyntheticVolume;
class DataContainer;
using DataContainerShPtrType = std::shared_ptr<DataContainer>;

/**
 * @brief The InitializeSyntheticVolumeWidget class
 */
class InitializeSyntheticVolumeWidget : public FilterParameterWidget, private Ui::InitializeSyntheticVolumeWidget
{
  Q_OBJECT

public:
  InitializeSyntheticVolumeWidget(FilterParameter* parameter, AbstractFilter* filter = nullptr, QWidget* parent = nullptr);

  ~InitializeSyntheticVolumeWidget() override;

  /**
   * @brief Initializes some of the GUI elements with selections or other GUI related items
   */
  void setupGui() override;

  void setFilter(AbstractFilter* value) override;
  AbstractFilter* getFilter() const override;

public Q_SLOTS:
  // void widgetChanged(const QString& msg);
  void beforePreflight();
  void afterPreflight();
  void filterNeedsInputParameters(AbstractFilter* filter);
  void displayErrorMessage(const AbstractMessage::Pointer& msg);

protected Q_SLOTS:
  // Auto Hookup Slots
  void on_m_InputFileBtn_clicked();
  void on_m_InputFile_textChanged(const QString& text);
  void on_m_XPoints_valueChanged(int v);
  void on_m_YPoints_valueChanged(int v);
  void on_m_ZPoints_valueChanged(int v);
  void on_m_XResolution_valueChanged(double v);
  void on_m_YResolution_valueChanged(double v);
  void on_m_ZResolution_valueChanged(double v);

protected:
  void setInputFilePath(QString val);
  QString getInputFilePath();

  /**
   * @brief setInputFile
   * @param v
   */
  void setInputFile(const QString& v);

  /**
   * @brief setWidgetListEnabled
   */
  void setWidgetListEnabled(bool v);

  /**
   * @brief estimate_numFeatures
   * @param xpoints
   * @param ypoints
   * @param zpoints
   * @param xres
   * @param yres
   * @param zres
   * @return
   */
  int estimate_numFeatures(int xpoints, int ypoints, int zpoints, float xres, float yres, float zres);

  /**
   * @brief estimateNumFeaturesSetup
   */
  void estimateNumFeaturesSetup();

#if 0

    virtual AbstractFilter::Pointer getFilter(bool defaultValues);

    virtual void writeOptions(QSettings& prefs);
    virtual void readOptions(QSettings& prefs);



    QFilterWidget* createDeepCopy();

    void setShapeTypes(DataArray<unsigned int>::Pointer array);

    virtual QString getFilterGroup();

    virtual void openHtmlHelpFile();
    virtual void getGuiParametersFromFilter(AbstractFilter* filt);
#endif

private:
  InitializeSyntheticVolume* m_Filter;
  QList<QWidget*> m_WidgetList;

  bool m_Version4Warning;
  bool m_DidCausePreflight;
  bool m_NewFileLoaded;

  DataContainerShPtrType m_DataContainer;
  QList<QLabel*> m_ShapeTypeLabels;
  QList<QComboBox*> m_ShapeTypeCombos;

  DataArraySelectionFilterParameter::Pointer m_StatsArrayPath;
  DataArraySelectionFilterParameter::Pointer m_PhaseTypesPath;
  DataArraySelectionFilterParameter::Pointer m_CrystalStructuresPath;

public:
  InitializeSyntheticVolumeWidget(const InitializeSyntheticVolumeWidget&) = delete;            // Copy Constructor Not Implemented
  InitializeSyntheticVolumeWidget(InitializeSyntheticVolumeWidget&&) = delete;                 // Move Constructor Not Implemented
  InitializeSyntheticVolumeWidget& operator=(const InitializeSyntheticVolumeWidget&) = delete; // Copy Assignment Not Implemented
  InitializeSyntheticVolumeWidget& operator=(InitializeSyntheticVolumeWidget&&) = delete;      // Move Assignment Not Implemented
};
