#ifndef dvSiSSRController_h
#define dvSiSSRController_h

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
#include <dvDirectoryStructure.h>
#include <dvStateMachine.h>
#include <dvSiSSRView.h>
#include <itkLoopSubdivisionSurfaceMesh.h>

namespace dv
{
class SiSSRController
: public QMainWindow
{
  Q_OBJECT
public:

  // Constructor/Destructor
  SiSSRController(int,char**);
  ~SiSSRController();

protected slots:

  void FrameValueChanged(int);
  void EDButtonPressed();
  void JumpToEDButtonPressed();
  void CalculateBoundaryCandidates();

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
  void Register();
  void CalculateResidualsForPass(const unsigned int pass);
  void WriteScreenshots();
  void UpdateCellData();

private:

  using TVTKMeshReader = vtkSmartPointer<vtkPolyDataReader>;

  void UpdateAnnotations();
  typedef double TReal;

  // ITK typedefs
  typedef itk::Image<short,3>                                              TImage;
  typedef itk::Image<TReal,3>                                              TWorkImage;
  typedef itk::ImageFileReader< TImage >                                   TImageReader;
  typedef itk::ImageToVTKImageFilter< TImage >                             TITK2VTK;
  typedef itk::DefaultStaticMeshTraits< TReal,  // Pixel Type
                                        3,      // Point Dimension
                                        3,      // Max Topological Dimension
                                        TReal,  // Coordinate Representation
                                        TReal,  // Interpolation Weight
                                        TReal > // Cell Pixel Type
                                                TMeshTraits;
  typedef itk::QuadEdgeMeshTraits< TReal,  // Pixel Type
                                   3,      // Point Dimension
                                   TReal,  // PData
                                   TReal,  // DData
                                   TReal,  // Coordinate Representation
                                   TReal > // Interpolation Weight
                                           TQEMeshTraits;

  typedef itk::Mesh< TReal, 3, TMeshTraits >                         TMesh;
  typedef itk::QuadEdgeMesh< TReal, 3, TQEMeshTraits >               TQEMesh;
  typedef itk::LoopSubdivisionSurfaceMesh< TReal, 3, TQEMeshTraits > TLoopMesh;

  typedef typename TMesh::CellType                     TCell;
  typedef itk::VertexCell< TCell >                     TVertex;
  typedef itk::ImageFileWriter< TImage >               TImageWriter;
  typedef itk::MeshFileWriter< TMesh >                 TMeshWriter;
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

  void DetermineState();
  void Render();
  void UpdateCurrentIndex();
  void Serialize();
  void Deserialize();
  void GenerateInitialModel();
  unsigned int GetCurrentFrame();
  void UpdateModelTransform();
  void CalculateSurfaceAreas();

  // Other properties
  DirectoryStructure DirectoryStructure;

  StateMachine State;
  SiSSRView window;

};
} // End namespace

#endif // SiSSRController_h

