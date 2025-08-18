#ifndef sissr_Controller_h
#define sissr_Controller_h

// Qt
#include <ui_QtVTKRenderWindows.h>
#include <QMainWindow>

// ITK
#include <itkMesh.h>
#include <itkQuadEdgeMesh.h>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkImageToVTKImageFilter.h>
#include <itkMeshFileReader.h>
#include <itkMeshFileWriter.h>
#include <itkVertexCell.h>
#include <itkPointsLocator.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkPolyDataReader.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>

// RapidJSON
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

// Custom
#include <itkLoopSubdivisionSurfaceMesh.h>

// SiSSR
#include <sissrDirectoryStructure.h>
#include <sissrStateMachine.h>
#include <sissrView.h>

namespace sissr {

class Controller
: public QMainWindow
{
  Q_OBJECT
public:

  // Constructor/Destructor
  Controller(int argc, char** argv);
  ~Controller();

  void ResetCamera();
  void CalculateResiduals();
  StateMachine State;

public slots:

  void CalculateBoundaryCandidates();
  void GenerateInitialModel();
  void Register();

  void FrameValueChanged(int);
  void EDButtonPressed();
  void JumpToEDButtonPressed();

  void ToggleImagePlanes();
  void ToggleCandidates();
  void ToggleModel();
  void ToggleModelWires();
  void ToggleModelSurface();
  void ToggleColorbar();
  void ToggleResiduals();

  void CurrentPageChanged(int);
  void IncrementFrame();
  void DecrementFrame();
  void SetupModel();
  void CalculateResidualsForPass(const unsigned int pass);
  void WriteScreenshots();
  void UpdateCellData();

private:

  using TVTKMeshReader = vtkSmartPointer<vtkPolyDataReader>;

  void UpdateAnnotations();

  using TIntegral = unsigned char;
  using TReal = float;
  static constexpr unsigned int Dimension = 3;

  // Image typedefs
  using TImage = itk::Image<TIntegral,Dimension>;
  using TImageReader = itk::ImageFileReader<TImage>;
  using TImageWriter = itk::ImageFileWriter<TImage>;
  using TITK2VTK = itk::ImageToVTKImageFilter<TImage>;

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

  // Qt Properties
  Ui_QtVTKRenderWindows *ui;

  // Setup
  void SetupImage();
  void SetupImagePlanes();
  void SetupCandidates();
  void SetupValueLabels();
  void SetupSliderRanges();
  void SetupSlots();

  void Render();
  void UpdateCurrentIndex();
  void Serialize();
  void Deserialize();
  unsigned int GetCurrentFrame();
  void UpdateModelTransform();
  void CalculateSurfaceAreas();

  // Other properties
  DirectoryStructure DirStructure;

  View window;

};

} // namespace sissr

#endif
