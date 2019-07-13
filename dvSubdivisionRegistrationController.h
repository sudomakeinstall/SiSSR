
#ifndef dvSubdivisionRegistrationController_h
#define dvSubdivisionRegistrationController_h

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
//#include <itkNumericTraits.h>
#include <itkVertexCell.h>
//#include <itkImageRegionIterator.h>
#include <itkBinaryMask3DMeshSource.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkOBJReader.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>

// RapidJSON
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

// Custom
#include <dvDirectoryStructure.h>
#include <dvStateMachine.h>
#include <dvSubdivisionRegistrationWindow.h>
#include <itkCalculateBoundaryCandidates2.h>
#include <itkLoopSubdivisionSurfaceMesh.h>

namespace dv
{
class SubdivisionRegistrationController
: public QMainWindow
{
  Q_OBJECT
public:

  // Constructor/Destructor
  SubdivisionRegistrationController(int,char**);
  ~SubdivisionRegistrationController();

  void UpdateReslicePlanesPlacement();

protected slots:

  void PlanesDistanceValueChanged(int);
  void UpperLVValueChanged(int);
  void LowerLVValueChanged(int);
  void FrameValueChanged(int);
  void UpperWindowValueChanged(int);
  void LowerWindowValueChanged(int);
  void EDButtonPressed();
  void JumpToEDButtonPressed();
  void CalculateBoundaryCandidates();

  void ToggleImageVolume();
  void ToggleImagePlanes();
  void ToggleReslicePlanes();
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

  std::vector<unsigned short>
  CalculateSASegmentIDsForCellData(const std::vector<std::array<double, 3>> &);

  std::vector<unsigned short>
  CalculateLASegmentIDsForCellData(const std::vector<std::array<double, 3>> &);

  vtkSmartPointer<vtkFloatArray>
  CalculateSegmentIDsForCellData(const std::vector<std::array<double, 3>> &);

  void CalculateSegmentIDs();

private:

  using TVTKMeshReader = vtkSmartPointer<vtkOBJReader>;

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

  typedef itk::CalculateBoundaryCandidates2< TImage >  TCandidates;
  typedef itk::BinaryMask3DMeshSource< TImage, TMesh > TMarchingCubes;

  // Qt Properties
  Ui_QtVTKRenderWindows *ui;

  // Setup
  void SetupImage();
  void SetupImageVolume();
  void SetupImagePlanes();
  void SetupReslicePlanes();
  void SetupCandidates();
  void SetupValueLabels();
  void SetupSliderRanges();
  void SetupSlots();

  void DetermineState();
  void UpdateTransferFunction();
  void Render();
  void UpdateCurrentIndex();
  void Serialize();
  void Deserialize();
  void WriteInitialModel();
  void WriteInitialRefinedModel();
  void WriteInitialSubdividedModel();
  unsigned int GetCurrentFrame();
  void UpdateModelTransform();
  void CalculateSurfaceAreas();

  // Other properties
  DirectoryStructure FileTree;

  StateMachine State;
  SubdivisionRegistrationWindow window;

};
} // End namespace

#endif // SubdivisionRegistrationController_h

