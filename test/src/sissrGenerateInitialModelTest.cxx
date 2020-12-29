// STD
#include <filesystem>

// ITK
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkQuadEdgeMesh.h>

// Custom
#include <itkGenerateInitialModelImageToMeshFilter.h>
#include <dvITKMeshToVTKPolyData.h>
#include <dvQuickViewMultiplePolyData.h>

int main(int argc, char** argv) {

  itkAssertOrThrowMacro(argc == 3, "The test expects three arguments.");

  namespace fs = std::filesystem;

  const fs::path test_dir = argv[1];
  const fs::path input_file_name = test_dir / "0.nii.gz";
  std::stringstream ss(argv[2]);
  bool show;
  ss >> std::boolalpha >> show;

  const double sigma = 0.1;
  const unsigned int count = 1024;
  const unsigned int lv_radius = 10;
  const unsigned int gn_radius = 5;

  using TPixel = unsigned char;
  const unsigned int Dimension = 3;
  using TReal = float;

  using TImage = itk::Image<TPixel, Dimension>;
  using TMeshTraits = itk::QuadEdgeMeshTraits<
    TReal,  // Pixel Type
    3,       // Point Dimension
    TReal,   // PData
    TReal,   // DData
    TReal,   // Coordinate Representation
    TReal >; // Interpolation Weight
  using TMesh = itk::QuadEdgeMesh<TReal, Dimension, TMeshTraits>;

  using TReader = itk::ImageFileReader<TImage>;
  const auto reader = TReader::New();
  reader->SetFileName(input_file_name);

  using TModel = itk::GenerateInitialModelImageToMeshFilter<TImage,TMesh>;

  const auto model0 = TModel::New();
  model0->SetInput(reader->GetOutput());
  model0->SetNumberOfCellsInDecimatedMesh(count);
  model0->SetMeshNoiseSigma(sigma);
  model0->SetLVClosingRadius(lv_radius);
  model0->SetGeneralClosingRadius(gn_radius);
  model0->PreserveEdgesOff();
  model0->Update();

  const auto model1 = TModel::New();
  model1->SetInput(reader->GetOutput());
  model1->SetNumberOfCellsInDecimatedMesh(count);
  model1->SetMeshNoiseSigma(sigma);
  model1->SetLVClosingRadius(lv_radius);
  model1->SetGeneralClosingRadius(gn_radius);
  model1->PreserveEdgesOn();
  model1->Update();

  if (show) {

    const auto m0 = dv::ITKMeshToVTKPolyData<TMesh>( model0->GetOutput() );
    const auto m1 = dv::ITKMeshToVTKPolyData<TMesh>( model1->GetOutput() );
    std::vector<vtkPolyData*> poly_data_vector;
    poly_data_vector.emplace_back(m0);
    poly_data_vector.emplace_back(m1);

    dv::QuickViewMultiplePolyData( poly_data_vector, true );

  }

  return EXIT_SUCCESS;

}
