#include "itkLoopSubdivisionSurfaceMesh.h"
#include "itkMeshFileReader.h"
#include "itkTestingMacros.h"

int main(int argc, char ** argv)
{

  if (argc != 2)
    {
    std::cerr << "No input mesh supplied." << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << argv[1] << std::endl;

  // Typedefs
  typedef double                                      TReal;
  typedef itk::LoopSubdivisionSurfaceMesh< TReal, 3 > TMesh;
  typedef itk::MeshFileReader< TMesh >                TReader;

  auto reader = TReader::New();
  reader->SetFileName( argv[1] );
  reader->Update();

  TMesh::Pointer mesh = TMesh::New();
  mesh->Graft( reader->GetOutput() );

  std::cout << "Number of points: " << mesh->GetNumberOfPoints() << std::endl;
  std::cout << "Number of cells: "  << mesh->GetNumberOfCells() << std::endl;

  mesh->Setup();

  // Get some test points.
  mesh->GetPointOnSurface(0,std::make_pair(0.1,0.1));


  return EXIT_SUCCESS;

}
