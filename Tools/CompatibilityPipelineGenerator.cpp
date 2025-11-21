
#include <stdlib.h>

#include "SIMPLib/Filtering/ComparisonInputsAdvanced.h"
#include "SIMPLib/Filtering/QMetaObjectUtilities.h"
#include "SIMPLib/FilterParameters/MontageFileListInfo.h"
#include "SIMPLib/FilterParameters/IntVec2FilterParameter.h"
#include "SIMPLib/FilterParameters/FloatVec2FilterParameter.h"
#include "SIMPLib/Common/EnsembleInfo.h"
#include "SIMPLib/Utilities/MontageSelection.h"
#include "SIMPLib/Common/ShapeType.h"

#include <iostream>
#include <set>
#include <string>

using namespace std::string_literals;

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QMetaProperty>
#include <QtCore/QString>
#include <SIMPLib/FilterParameters/JsonFilterParametersWriter.h>
#include <SIMPLib/Filtering/FilterPipeline.h>

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/FilterParameters/PreflightUpdatedValueFilterParameter.h"
#include "SIMPLib/Filtering/AbstractFilter.h"
#include "SIMPLib/Filtering/FilterFactory.hpp"
#include "SIMPLib/Filtering/FilterManager.h"
#include "SIMPLib/Plugin/ISIMPLibPlugin.h"
#include "SIMPLib/Plugin/PluginManager.h"
#include "SIMPLib/Plugin/SIMPLibPluginLoader.h"

#include "DREAM3DToolsConfiguration.h"

#if 0
#if 0
const QString k_OutputDir("/tmp");
const QString k_PluginsOutputDir("/Users/mjackson/Workspace1/complex_plugins");
#else
const QString k_OutputDir("/tmp/");
const QString k_PluginsOutputDir("/tmp/complex_plugins");
#endif
#endif

static int32_t s_TotalGoodFilters = 0;
static int32_t s_TotalBadFilters = 0;

const QString k_PLUGIN_NAME("@PLUGIN_NAME@");
const QString k_PLUGIN_DESCRIPTION("@PLUGIN_DESCRIPTION@");
const QString k_PLUGIN_VENDOR("@PLUGIN_VENDOR@");
const QString k_FILTER_NAME("@FILTER_NAME@");
const QString k_ALGORITHM_NAME("@ALGORITHM_NAME@");
const QString k_UUID("@UUID@");
const QString k_OLD_UUID("@OLD_UUID@");
const QString k_PARAMETER_KEYS("@PARAMETER_KEYS@");
const QString k_PARAMETER_DEFS("@PARAMETER_DEFS@");
const QString k_FILTER_HUMAN_NAME("@FILTER_HUMAN_NAME@");
const QString k_PARAMETER_INCLUDES("@PARAMETER_INCLUDES@");
const QString k_PREFLIGHT_DEFS("@PREFLIGHT_DEFS@");
const QString k_INPUT_VALUES_DEF("@INPUT_VALUES_DEF@");
const QString k_DEFAULT_TAGS("@DEFAULT_TAGS@");
const QString k_PREFLIGHT_UPDATED_VALUES("@PREFLIGHT_UPDATED_VALUES@");
const QString k_PREFLIGHT_UPDATED_DEFS("@PREFLIGHT_UPDATED_DEFS@");
const QString k_PROPOSED_ACTIONS("@PROPOSED_ACTIONS@");

static QMap<QString, QVariant> s_ParameterTypeMapping;

void InitParameterTypeMapping()
{
  s_ParameterTypeMapping["BooleanFilterParameter"] = QVariant(true);
  s_ParameterTypeMapping["DataArrayCreationFilterParameter"] = QVariant();
  s_ParameterTypeMapping["DataArrayCreationFilterParameter"].setValue(DataArrayPath(QString("DC-A"), QString("AM-A"), QString("DA-A")));
  s_ParameterTypeMapping["DataArraySelectionFilterParameter"] = QVariant();
  s_ParameterTypeMapping["DataArraySelectionFilterParameter"].setValue(DataArrayPath(QString("DC-B"), QString("AM-B"), QString("DA-B")));
  s_ParameterTypeMapping["ChoiceFilterParameter"] = QVariant(42);
  s_ParameterTypeMapping["AttributeMatrixCreationFilterParameter"] = QVariant();
  s_ParameterTypeMapping["AttributeMatrixCreationFilterParameter"].setValue(DataArrayPath(QString("DC-C|AM-C")));
  s_ParameterTypeMapping["AttributeMatrixSelectionFilterParameter"] = QVariant();
  s_ParameterTypeMapping["AttributeMatrixSelectionFilterParameter"].setValue(DataArrayPath(QString("DC-D|AM-D")));
  s_ParameterTypeMapping["DataContainerCreationFilterParameter"] = QVariant();
  s_ParameterTypeMapping["DataContainerCreationFilterParameter"].setValue(DataArrayPath(QString("DC-E")));
  s_ParameterTypeMapping["DataContainerSelectionFilterParameter"] = QVariant();
  s_ParameterTypeMapping["DataContainerSelectionFilterParameter"].setValue(DataArrayPath(QString("DC-F")));
  s_ParameterTypeMapping["InputFileFilterParameter"] = QVariant(QString("/Input/File/Filter/Parameter.txt"));
  s_ParameterTypeMapping["InputPathFilterParameter"] = QVariant(QString("/Input/Path/Filter/Parameter"));
  s_ParameterTypeMapping["OutputFileFilterParameter"] = QVariant(QString("/Output/File/Filter/Parameter.txt"));
  s_ParameterTypeMapping["OutputPathFilterParameter"] = QVariant(QString("/Output/File/Filter/Parameter"));
  s_ParameterTypeMapping["FileListInfoFilterParameter"] = QVariant();
  s_ParameterTypeMapping["FileListInfoFilterParameter"].setValue(StackFileListInfo(2, 1, 12, 167, 3, QString("/File/List/Info/Filter/Parameter"), QString("prefix-"), QString("-suffix"), QString(".txt")));
  s_ParameterTypeMapping["FloatFilterParameter"] = QVariant(92.27f);
  s_ParameterTypeMapping["IntFilterParameter"] = QVariant(-132);
  s_ParameterTypeMapping["DoubleFilterParameter"] = QVariant(184.54);
  s_ParameterTypeMapping["UInt64FilterParameter"] = QVariant(132ULL);
  s_ParameterTypeMapping["AxisAngleFilterParameter"] = QVariant();
  s_ParameterTypeMapping["AxisAngleFilterParameter"].setValue(AxisAngleInput(23.7f, -62.9f, 36.4f, 90.01f));
  s_ParameterTypeMapping["IntVec2FilterParameter"] = QVariant();
  s_ParameterTypeMapping["IntVec2FilterParameter"].setValue(IntVec2Type(-23, 61));
  s_ParameterTypeMapping["IntVec3FilterParameter"] = QVariant();
  s_ParameterTypeMapping["IntVec3FilterParameter"].setValue(IntVec3Type(35, -56, 92));
  s_ParameterTypeMapping["FloatVec2FilterParameter"] = QVariant();
  s_ParameterTypeMapping["FloatVec2FilterParameter"].setValue(FloatVec2Type(-71.63f, 26.81f));
  s_ParameterTypeMapping["FloatVec3FilterParameter"] = QVariant();
  s_ParameterTypeMapping["FloatVec3FilterParameter"].setValue(FloatVec3Type(782.62f, -15.48f, 49.11f));
  s_ParameterTypeMapping["NumericTypeFilterParameter"] = QVariant();
  s_ParameterTypeMapping["NumericTypeFilterParameter"].setValue(SIMPL::NumericTypes::Type::UInt16);
  s_ParameterTypeMapping["StringFilterParameter"] = QVariant(QString("StringFilterParameter"));
  s_ParameterTypeMapping["SeparatorFilterParameter"] = QVariant(QString(""));
  s_ParameterTypeMapping["LinkedDataContainerSelectionFilterParameter"] = QVariant();
  s_ParameterTypeMapping["LinkedDataContainerSelectionFilterParameter"].setValue(DataArrayPath(QString("DC-G")));
  s_ParameterTypeMapping["LinkedPathCreationFilterParameter"] = QVariant(QString("LinkedPathCreationFilterParameter"));
  s_ParameterTypeMapping["MultiDataArraySelectionFilterParameter"] = QVariant();
  s_ParameterTypeMapping["MultiDataArraySelectionFilterParameter"].setValue(std::vector<DataArrayPath>{DataArrayPath("DC-H|AM-E|DA-C"), DataArrayPath("DC-I|AM-F|DA-D")});
  s_ParameterTypeMapping["LinkedBooleanFilterParameter"] = QVariant(true);
  s_ParameterTypeMapping["LinkedChoicesFilterParameter"] = QVariant(1);
  s_ParameterTypeMapping["PreflightUpdatedValueFilterParameter"] = QVariant(QString("PreflightUpdatedValueFilterParameter"));
  s_ParameterTypeMapping["CalculatorFilterParameter"] = QVariant(QString("57+92"));
  s_ParameterTypeMapping["ComparisonSelectionAdvancedFilterParameter"] = QVariant();
  ComparisonInputsAdvanced compInputAdv = {};
  compInputAdv.addInput(1, QString("DC-J|AM-G|DA-E"), 1, 3.76f);
  s_ParameterTypeMapping["ComparisonSelectionAdvancedFilterParameter"].setValue(compInputAdv);
  s_ParameterTypeMapping["ComparisonSelectionFilterParameter"] = QVariant();
  ComparisonInputs compInput = {};
  compInput.addInput(QString("DC-K"), QString("AM-H"), QString("DA-F"), 1, 84.301f);
  s_ParameterTypeMapping["ComparisonSelectionFilterParameter"].setValue(compInput);
  s_ParameterTypeMapping["ConvertHexGridToSquareGridFilterParameter"] = "<<<NOT_IMPLEMENTED>>>";

  {
    DataContainerArrayProxy dcArrayProxy = {};
    {
      DataArrayProxy daProxy("DC-L|AM-I", "DA-G");

      AttributeMatrixProxy amProxy = AttributeMatrixProxy("AM-I");
      amProxy.insertDataArray("DA-G", daProxy);
      amProxy.setAMType(AttributeMatrix::Type::Face);
      amProxy.setFlag(3);
      amProxy.setName("AM-I");

      DataContainerProxy dcProxy = DataContainerProxy("DC-L");
      dcProxy.insertAttributeMatrix("AM-I", amProxy);
      dcProxy.setDCType(1);
      dcProxy.setFlag(4);
      dcProxy.setName("DC-L");

      dcArrayProxy.insertDataContainer("DC-L", dcProxy);
    }
    {
      AttributeMatrixProxy amProxy = AttributeMatrixProxy("AM-J");
      amProxy.setAMType(AttributeMatrix::Type::Face);
      amProxy.setFlag(3);
      amProxy.setName("AM-J");

      DataContainerProxy dcProxy = DataContainerProxy("DC-M");
      dcProxy.insertAttributeMatrix("AM-J", amProxy);
      dcProxy.setDCType(1);
      dcProxy.setFlag(4);
      dcProxy.setName("DC-M");

      dcArrayProxy.insertDataContainer("DC-M", dcProxy);
    }
    {
      DataContainerProxy dcProxy = DataContainerProxy("DC-N");
      dcProxy.setDCType(1);
      dcProxy.setFlag(4);
      dcProxy.setName("DC-N");

      dcArrayProxy.insertDataContainer("DC-N", dcProxy);
    }

    s_ParameterTypeMapping["DataContainerArrayProxyFilterParameter"] = QVariant();
    s_ParameterTypeMapping["DataContainerArrayProxyFilterParameter"].setValue(dcArrayProxy);
  }
  s_ParameterTypeMapping["DataContainerReaderFilterParameter"] = QVariant(QString("/DataContainer/Reader/Filter/Parameter.dream3d"));
  s_ParameterTypeMapping["DynamicChoiceFilterParameter"] = QVariant(QString("DynamicChoiceFilterParameter"));
  s_ParameterTypeMapping["DynamicTableFilterParameter"] = QVariant();
  s_ParameterTypeMapping["DynamicTableFilterParameter"].setValue(DynamicTableData(std::vector<std::vector<double>>{{1.1, 1.2, 1.3}, {2.1, 2.2, 2.3}, {3.1, 3.2, 3.3}}, QStringList{"Row1","Row2","Row3"},QStringList{"Head1","Head2","Head3"}));
  s_ParameterTypeMapping["EMMPMFilterParameter"] = QVariant(QString("EMMPMFilterParameter"));
  s_ParameterTypeMapping["EbsdMontageImportFilterParameter"] = QVariant();
  s_ParameterTypeMapping["EbsdMontageImportFilterParameter"].setValue(MontageFileListInfo(1,2,3,7));
  s_ParameterTypeMapping["EbsdToH5EbsdFilterParameter"] = QVariant(QString("/Ebsd/To/H5Ebsd/Filter/Parameter.h5ebsd"));
  s_ParameterTypeMapping["EbsdWarpPolynomialFilterParameter"] = "<<<NOT_IMPLEMENTED>>>";
  s_ParameterTypeMapping["EnsembleInfoFilterParameter"] = QVariant();
  EnsembleInfo ensInfo = {};
  ensInfo.addValues(EnsembleInfo::CrystalStructure::Cubic_High, PhaseType::Type::Precipitate, QString("EnsembleInfo"));
  ensInfo.addValues(EnsembleInfo::CrystalStructure::Cubic_Low, PhaseType::Type::Transformation, QString("FilterParameter"));
  s_ParameterTypeMapping["EnsembleInfoFilterParameter"].setValue(ensInfo);
  s_ParameterTypeMapping["SecondOrderPolynomialFilterParameter"] = QVariant();
  s_ParameterTypeMapping["SecondOrderPolynomialFilterParameter"].setValue(Float2ndOrderPolynomial(0.0f, 0.1f, 1.0f, 1.1f, 0.2f, 2.2f));
  s_ParameterTypeMapping["ThirdOrderPolynomialFilterParameter"] = QVariant();
  s_ParameterTypeMapping["ThirdOrderPolynomialFilterParameter"].setValue(Float3rdOrderPoly_t{.c30=3.0f,.c03=0.3f,.c21=2.1f,.c12=1.2f,.c20=2.0f,.c02=0.2f,.c11=1.1f,.c10=1.0f,.c01=0.1f,.c00=0.0f});
  s_ParameterTypeMapping["FourthOrderPolynomialFilterParameter"] = QVariant();
  s_ParameterTypeMapping["FourthOrderPolynomialFilterParameter"].setValue(Float4thOrderPolynomial(0.0f,0.1f,1.0f,1.1f,0.2f,2.0f,1.2f,2.1f,0.3f,3.0f,2.2f,1.3f,3.1f,0.4f,4.0f));
  s_ParameterTypeMapping["GenerateColorTableFilterParameter"] = QVariant(QString("GenerateColorTableFilterParameter"));
  s_ParameterTypeMapping["ImportHDF5DatasetFilterParameter"] = QVariant(QString("/Import/HDF5/Dataset/Filter/Parameter.h5"));
  s_ParameterTypeMapping["ImportVectorImageStackFilterParameter"] = QVariant();
  s_ParameterTypeMapping["KbrRecisConfigFilterParameter"] = "<<<NOT_IMPLEMENTED>>>";
  s_ParameterTypeMapping["MontageSelectionFilterParameter"] = QVariant();
  s_ParameterTypeMapping["MontageSelectionFilterParameter"].setValue(MontageSelection(QString("prefix-"), QString("-suffix"), 2, 5, 7, 4, 9));
  s_ParameterTypeMapping["MontageStructureSelectionFilterParameter"] = QVariant(QString("MontageStructureSelectionFilterParameter"));
  s_ParameterTypeMapping["MultiAttributeMatrixSelectionFilterParameter"] = QVariant();
  s_ParameterTypeMapping["MultiAttributeMatrixSelectionFilterParameter"].setValue(std::vector<DataArrayPath>{DataArrayPath("DC-O|AM-K"),DataArrayPath("DC-P|AM-L"), DataArrayPath("DC-Q|AM-M")});
  s_ParameterTypeMapping["MultiDataContainerSelectionFilterParameter"] = QVariant();
  s_ParameterTypeMapping["MultiDataContainerSelectionFilterParameter"].setValue(std::vector<QString>{QString("DC-R"), QString("DC-S")});
  s_ParameterTypeMapping["MultiInputFileFilterParameter"] = "<<<NOT_IMPLEMENTED>>>";
  s_ParameterTypeMapping["OEMEbsdScanSelectionFilterParameter"] = QVariant(QStringList{QString("Scan A"),QString("Scan B"),QString("Scan C")});
  s_ParameterTypeMapping["OrientationUtilityFilterParameter"] = QVariant(QString(""));
  s_ParameterTypeMapping["ParagraphFilterParameter"] = QVariant(QString("ParagraphFilterParameter"));
  s_ParameterTypeMapping["PhaseTypeSelectionFilterParameter"] = "<<<NOT_IMPLEMENTED>>>";
  s_ParameterTypeMapping["RangeFilterParameter"] = QVariant();
  s_ParameterTypeMapping["RangeFilterParameter"].setValue(std::pair<double, double>{-2.8, 77.36});
  s_ParameterTypeMapping["ReadASCIIDataFilterParameter"] = QVariant(QString("/Read/ASCII/Data/Filter/Parameter.csv"));
  s_ParameterTypeMapping["ReadH5EbsdFilterParameter"] = QVariant(QString("/Read/H5Ebsd/Filter/Parameter.h5ebsd"));
  s_ParameterTypeMapping["ScalarTypeFilterParameter"] = QVariant();
  s_ParameterTypeMapping["ScalarTypeFilterParameter"].setValue(SIMPL::ScalarTypes::Type::Int16);
  s_ParameterTypeMapping["ShapeTypeSelectionFilterParameter"] = QVariant();
  s_ParameterTypeMapping["ShapeTypeSelectionFilterParameter"].setValue(ShapeType::Types{ShapeType::Type::CubeOctahedron, ShapeType::Type::SuperEllipsoid});
  s_ParameterTypeMapping["StatsGeneratorFilterParameter"] = QVariant(QString("StatsGeneratorFilterParameter"));
  s_ParameterTypeMapping["Symmetric6x6FilterParameter"] = "<<<NOT_IMPLEMENTED>>>";
}

// -----------------------------------------------------------------------------
void GeneratePipeline(const QString& filePath)
{
  FilterPipeline::Pointer pipeline = FilterPipeline::New();
  FilterManager* fm = FilterManager::Instance();
  QSet<QString> pluginNames = fm->getPluginNames();
  for(const auto& pluginName : pluginNames)
  {
    FilterManager::Collection factories = fm->getFactoriesForPluginName(pluginName);
    FilterManager::Collection::const_iterator factoryMapIter = factories.constBegin();
    while(factoryMapIter != factories.constEnd())
    {
      IFilterFactory::Pointer factory = fm->getFactoryFromClassName(factoryMapIter.key());
      if(factory.get() != nullptr)
      {
        AbstractFilter::Pointer filter = factory->create();
        FilterParameterVectorType params = filter->getFilterParameters();
        for(const auto& param : params)
        {
//          param->setDefaultValue(s_ParameterTypeMapping[param->getNameOfClass()]);
          filter->setProperty(param->getPropertyName().toStdString().c_str(), s_ParameterTypeMapping[param->getNameOfClass()]);
        }
        pipeline->pushBack(filter);
      }
      factoryMapIter++;
    }
  }

  JsonFilterParametersWriter::Pointer jsonWriter = JsonFilterParametersWriter::New();
  jsonWriter->writePipelineToFile(pipeline, filePath, "CompatibilityTestPipeline", true);
}

// -----------------------------------------------------------------------------
//  Use unit test framework
// -----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  const size_t k_OutputDir = 0;
  const size_t k_HelpIndex = 1;

  using ArgEntry = std::vector<std::string>;
  using ArgEntries = std::vector<ArgEntry>;

  ArgEntries args;

  args.push_back({"-o", "--output_file", "The output file to write the pipeline"});

  args.push_back({"-h", "--help", "Show help for this program"});

  QString outputFile;

  for(int32_t i = 0; i < argc; i++)
  {
    if(argv[i] == args[k_OutputDir][0] || argv[i] == args[k_OutputDir][1])
    {
      outputFile = argv[++i];
    }

    if(argv[i] == args[k_HelpIndex][0] || argv[i] == args[k_HelpIndex][1])
    {
      std::cout << "This program has the following arguments:" << std::endl;
      for(const auto& input : args)
      {
        std::cout << input[0] << ": " << input[1] << std::endl;
      }
      return 0;
    }
  }

  // Instantiate the QCoreApplication that we need to get the current path and load plugins.
  QCoreApplication app(argc, argv);
  QCoreApplication::setOrganizationName("BlueQuartz Software");
  QCoreApplication::setOrganizationDomain("bluequartz.net");
  QCoreApplication::setApplicationName("CompatibilityPipelineGenerator");

  // Load all the plugins and
  // Register all the filters including trying to load those from Plugins
  FilterManager* fm = FilterManager::Instance();
  SIMPLibPluginLoader::LoadPluginFilters(fm);
  // THIS IS A VERY IMPORTANT LINE: It will register all the known filters in the dream3d library. This
  // will NOT however get filters from plugins. We are going to have to figure out how to compile filters
  // into their own plugin and load the plugins from a command line.
  fm->RegisterKnownFilters(fm);

  QMetaObjectUtilities::RegisterMetaTypes();

  InitParameterTypeMapping();

  GeneratePipeline(outputFile);

  return EXIT_SUCCESS;
}
