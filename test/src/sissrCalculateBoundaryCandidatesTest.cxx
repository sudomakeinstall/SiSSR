// STD
#include <sstream>

// ITK
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkQuadEdgeMesh.h>
#include <itkCuberilleImageToMeshFilter.h>

// Custom
#include <itkCleanSegmentationImageFilter.h>
#include <itkGenerateInitialModelImageToMeshFilter.h>
#include <dvITKMeshToVTKPolyData.h>
#include <dvQuickViewMultiplePolyData.h>

int main(int argc, char** argv) {

  itkAssertOrThrowMacro(argc == 3, "The test expects three arguments.");

  const std::string test_dir(argv[1]);
  const std::string input_file_name = test_dir + "0.nii.gz";
  std::stringstream ss(argv[2]);
  bool show;
  ss >> std::boolalpha >> show;

  using TPixel = unsigned char;
  const unsigned int Dimension = 3;
  using TCoordinate = float;
  using TMesh = itk::QuadEdgeMesh<TCoordinate, Dimension>;

  using TImage = itk::Image<TPixel, Dimension>;
  using TReader = itk::ImageFileReader<TImage>;
  using TClean = itk::CleanSegmentationImageFilter<TImage>;
  using TCuberille = itk::CuberilleImageToMeshFilter<TImage, TMesh>;

  const auto reader = TReader::New();
  reader->SetFileName(input_file_name);

  const auto clean = TClean::New();
  clean->SetInput( reader->GetOutput() );

  const auto cuberille = TCuberille::New();
  cuberille->SetInput(clean->GetOutput());
  cuberille->GenerateTriangleFacesOff();
  cuberille->RemoveProblematicPixelsOn();
//  cuberille->ProjectVerticesToIsoSurfaceOn();
  cuberille->SavePixelAsCellDataOn();
  cuberille->Update();

  if (show) {
    const auto d = dv::ITKMeshToVTKPolyData< TMesh >( cuberille->GetOutput() );

    std::vector<vtkPolyData*> poly_data_vector;
    poly_data_vector.emplace_back(d);
    dv::QuickViewMultiplePolyData( poly_data_vector, true );
  }

  return EXIT_SUCCESS;

}
