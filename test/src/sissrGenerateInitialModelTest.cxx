// ITK
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkQuadEdgeMesh.h>

// Custom
#include <itkGenerateInitialModelImageToMeshFilter.h>
#include <dvITKMeshToVTKPolyData.h>
#include <dvQuickViewMultiplePolyData.h>

int main(int argc, char** argv) {

  const std::string test_dir(argv[1]);
  std::cout << test_dir << std::endl;

  const std::string input_file_name = test_dir + "0.nii.gz";

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
  const auto model = TModel::New();
  model->SetInput(reader->GetOutput());
  model->SetNumberOfCellsInDecimatedMesh(count);
  model->SetMeshNoiseSigma(sigma);
  model->SetLVClosingRadius(lv_radius);
  model->SetGeneralClosingRadius(gn_radius);
  model->Update();

  const auto m = dv::ITKMeshToVTKPolyData< TMesh >( model->GetOutput() );
  std::vector<vtkPolyData*> poly_data_vector;
  poly_data_vector.emplace_back(m);

  dv::QuickViewMultiplePolyData( poly_data_vector, true );

  return EXIT_SUCCESS;

}
