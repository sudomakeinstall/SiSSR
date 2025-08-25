#ifndef sissr_Algorithm_h
#define sissr_Algorithm_h

// ITK
#include <itkMesh.h>
#include <itkQuadEdgeMesh.h>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkMeshFileReader.h>
#include <itkMeshFileWriter.h>
#include <itkVertexCell.h>
#include <itkPointsLocator.h>

// Custom
#include <itkLoopSubdivisionSurfaceMesh.h>

// SiSSR
#include <sissrDirectoryStructure.h>
#include <sissrParameters.h>

namespace sissr {

class Algorithm
{
public:

  // Constructor/Destructor
  Algorithm(const std::string& candidateDir, const std::string& initialModel, const std::string& outputDir);
  ~Algorithm();

  // Core algorithm functions
  void Register();

  // Parameters access
  Parameters& GetParameters() { return parameters; }
  const Parameters& GetParameters() const { return parameters; }

  // Directory access
  const DirectoryStructure& GetDirectoryStructure() const { return dirStructure; }

private:

  using TIntegral = unsigned char;
  using TReal = float;
  static constexpr unsigned int Dimension = 3;

  // Image typedefs
  using TImage = itk::Image<TIntegral,Dimension>;
  using TImageReader = itk::ImageFileReader<TImage>;
  using TImageWriter = itk::ImageFileWriter<TImage>;

  using TMeshTraits = itk::DefaultStaticMeshTraits<
    TReal,     // Pixel Type
    Dimension, // Point Dimension
    Dimension, // Max Topological Dimension
    TReal,     // Coordinate Representation
    TReal,     // Interpolation Weight
    TReal >;   // Cell Pixel Type
  using TQEMeshTraits = itk::QuadEdgeMeshTraits<
    TReal,     // Pixel Type
    Dimension, // Point Dimension
    TReal,     // PData
    TReal,     // DData
    TReal,     // Coordinate Representation
    TReal >;   // Interpolation Weight

  using TMesh = itk::Mesh<TReal, 3, TMeshTraits>;
  using TQEMesh = itk::QuadEdgeMesh<TReal, 3, TQEMeshTraits>;
  using TLoopMesh = itk::LoopSubdivisionSurfaceMesh<TReal, 3, TQEMeshTraits>;

  using TMeshReader = itk::MeshFileReader<TMesh>;
  using TQEMeshReader = itk::MeshFileReader<TQEMesh>;
  using TLoopMeshReader = itk::MeshFileReader<TLoopMesh>;
  using TQEMeshWriter = itk::MeshFileWriter<TQEMesh>;
  using TMeshWriter = itk::MeshFileWriter<TMesh>;
  using TLocator = itk::PointsLocator< TMesh::PointsContainer >;

  // Helper functions (none currently)

  // Data members
  DirectoryStructure dirStructure;
  Parameters parameters;

}; // End class

} // namespace sissr

#endif