// ITK
#include <itkImage.h>
#include <itkPointSet.h>
#include <itkPointsLocator.h>
#include <itkQuadEdgeMesh.h>
#include <itkImageFileReader.h>
#include <itkCuberilleImageToMeshFilter.h>

// Custom
#include <itkCleanSegmentationImageFilter.h>
#include <sissrLabeledMeshToKdMap.h>

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
  using TImage = itk::Image<TPixel, Dimension>;
  using TMesh = itk::QuadEdgeMesh<TCoordinate, Dimension>;
  using TMeshToKdMap = sissr::LabeledMeshToKdMap<TCoordinate, Dimension>;

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
  cuberille->SavePixelAsCellDataOn();
  cuberille->Update();

  //
  //
  //

  TMeshToKdMap mesh_to_kd_map;
  const auto kd_map = mesh_to_kd_map.Calculate(cuberille->GetOutput());
  for (auto &[label, kd] : kd_map) {
    std::cout << label << ':' << kd->GetPoints()->Size() << std::endl;
  }

  //
  //
  //

  return EXIT_SUCCESS;

}
