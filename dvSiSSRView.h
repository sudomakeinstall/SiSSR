#ifndef dvSiSSRView_h
#define dvSiSSRView_h

// STD
#include <array>

// Qt
#include <ui_QtVTKRenderWindows.h>
#include <QMainWindow>

// ITK
#include <itkMesh.h>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkNumericTraits.h>
#include <itkImageToVTKImageFilter.h>

// VTK
#include <vtkNIFTIImageReader.h>
#include <vtkSmartPointer.h>
#include <vtkResliceImageViewer.h>
#include <vtkImagePlaneWidget.h>
#include <vtkDistanceWidget.h>
#include <vtkResliceImageViewerMeasurements.h>
#include <vtkColorTransferFunction.h>
#include <vtkRenderer.h>
#include <vtkCellPicker.h>
#include <vtkProperty.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkActor.h>
#include <vtkPolyDataReader.h>
#include <vtkExtractEdges.h>
#include <vtkTubeFilter.h>
#include <vtkLoopSubdivisionFilter.h>
#include <vtkAssembly.h>
#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkLookupTable.h>
#include <vtkScalarBarActor.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkGlyph3D.h>
#include <vtkSphereSource.h>
#include <vtkPlaneSource.h>
#include <vtkTransform.h>
#include <vtkImageData.h>
#include <vtkPlaneWidget.h>

// Custom
#include <dvDirectoryStructure.h>
#include <dvStateMachine.h>
#include <vtkColorbarState.h>
#include <dvPlaneWidgetCallback.h>

namespace dv
{
class SiSSRView
{
public:

  // Constructor/Destructor
  SiSSRView();
  ~SiSSRView(){};

  //
  // Typedefs
  //

  using TVTKMeshReader      = vtkSmartPointer<vtkPolyDataReader>;
  using TVTKResidualsReader = vtkSmartPointer<vtkPolyDataReader>;
  using TImage              = itk::Image<short,3>;
  using TImageReader        = itk::ImageFileReader<TImage>;
  using TITK2VTK            = itk::ImageToVTKImageFilter<TImage>;

  //
  // Setup - May only be called once.
  //

  void SetupImageData(const std::string&);
  void SetupImageInformation();
  void SetupImagePlane(vtkRenderWindowInteractor*);
  void SetupCandidates(std::string);
  void SetupModel(const std::string);
  void SetupResiduals(const std::string fileName);

  bool ImageDataHasBeenSetup        = false;
  bool ImageInformationHasBeenSetup = false;
  bool ImagePlaneHasBeenSetup       = false;
  bool CandidatesHaveBeenSetup      = false;
  bool ModelHasBeenSetup            = false;
  bool ResidualsHaveBeenSetup       = false;

  //
  // Visibility
  //

  void SetImagePlanesVisible(const bool &);
  void SetCandidatesVisible(const bool &);
  void SetModelVisible(const bool &);
  void SetModelWiresVisible(const bool &);
  void SetModelSurfaceVisible(const bool &);
  void SetColorbarVisible(const bool &);
  void SetResidualsVisible(const bool &);

  //
  // Update
  //

  void UpdateLookupTable();
  void UpdatePlanesSource(const std::string &);
  void UpdateCandidatesSource(const std::string &);
  void UpdateModelSource(const std::string &);
  void UpdateResidualsSource(const std::string &);

  unsigned int PlaneSourceResolution = 256;

  //
  // Utilities
  //

  vtkSmartPointer<vtkColorTransferFunction>
    CalculateImageTransferFunction(const double &,
                                   const double &,
                                   const double &,
                                   const double &);
  vtkSmartPointer<vtkColorTransferFunction>
    CalculateMeshTransferFunction();

  void SetPhaseAnnotation(const std::string &annotation)
    {
    if (nullptr == this->phaseAnnotation) return;
    this->phaseAnnotation->SetInput( annotation.c_str() );
    }

  TImage::PointType origin, point1, point2;

  vtkSmartPointer<vtkRenderer> renderer = nullptr;

//private:


  vtkSmartPointer< PlaneWidgetCallback > callback;

  // Planes
  typename TImageReader::Pointer       itkReader = nullptr;
  typename TITK2VTK::Pointer           itk2vtk = nullptr;

  vtkSmartPointer<vtkPlaneWidget> planeWidget = nullptr;
  vtkSmartPointer<vtkActor> imageActor = nullptr;

  // Candidates
// TODO
//  TVTKMeshReader      candidateReader = nullptr;
  vtkSmartPointer<vtkPolyDataMapper> candidateMapper = nullptr;
  vtkSmartPointer<vtkActor>          candidateActor = nullptr;

  // Model
  vtkSmartPointer<vtkLookupTable>           modelLUT = nullptr;
  TVTKMeshReader             modelReader = nullptr;
  vtkSmartPointer<vtkExtractEdges>          modelEdges = nullptr;
  vtkSmartPointer<vtkTubeFilter>            modelTubes = nullptr;
  vtkSmartPointer<vtkSphereSource>          modelVertexGlyph = nullptr;
  vtkSmartPointer<vtkGlyph3D>               modelVertices = nullptr;
  vtkSmartPointer<vtkLoopSubdivisionFilter> modelLoop = nullptr;
  vtkSmartPointer<vtkPolyDataMapper>        modelLoopMapper = nullptr;
  vtkSmartPointer<vtkPolyDataMapper>        modelWireMapper = nullptr;
  vtkSmartPointer<vtkPolyDataMapper>        modelVerticesMapper = nullptr;
  vtkSmartPointer<vtkPolyDataMapper2D>      modelColorbarMapper = nullptr;
  vtkSmartPointer<vtkActor>                 modelLoopActor = nullptr;
  vtkSmartPointer<vtkActor>                 modelWireActor = nullptr;
  vtkSmartPointer<vtkActor>                 modelVerticesActor = nullptr;
  vtkSmartPointer<vtkScalarBarActor>        modelColorbarActor = nullptr;
  vtkSmartPointer<vtkAssembly>              modelAssembly = nullptr;
  vtkSmartPointer<vtkFloatArray>            modelCellData = nullptr;

  TVTKResidualsReader                       modelResidualsReader = nullptr;
  vtkSmartPointer<vtkPolyDataMapper>        modelResidualsMapper = nullptr;
  vtkSmartPointer<vtkActor>                 modelResidualsActor = nullptr;

  vtkSmartPointer<vtkTubeFilter>            modelResidualsTubes = nullptr;
  vtkSmartPointer<vtkSphereSource>          modelResidualsVertexGlyph = nullptr;
  vtkSmartPointer<vtkGlyph3D>               modelResidualsVertices = nullptr;
  vtkSmartPointer<vtkPolyDataMapper>        modelResidualsVerticesMapper = nullptr;
  vtkSmartPointer<vtkActor>                 modelResidualsVerticesActor = nullptr;

  // Annotations
  vtkSmartPointer<vtkTextActor> phaseAnnotation = nullptr;

  unsigned int NumberOfSubdivisions = 3;

  // Color Mapping
  vtk::ColorbarState CB_State = {"", 0.0, 3.0};

  unsigned int NumberOfColorbarLabels = 8;

  double HueRangeMinimum = 0.00;
  double HueRangeMaximum = 1.00;

  double SaturationRangeMinimum = 0.9;
  double SaturationRangeMaximum = 0.9;

  double ValueRangeMinimum = 0.8;
  double ValueRangeMaximum = 0.8;

  double WireframeColor[3] = {1.0, 0.5, 0.5};
  double WireframeEdgeRadius = 0.25;
  double WireframeVertexRadius = 0.5;
  int WireframeNumberOfSides = 5;

  double ResidualsColor[3] = {0.0, 1.0, 1.0};
  double ResidualsEdgeRadius = 0.10;
  double ResidualsVertexRadius = 0.15;
  int ResidualsNumberOfSides = 10;

  vtkSmartPointer<vtkTransform> fTrans = nullptr;

  void PlaneWidgetDidMove();

  vtkSmartPointer<vtkPlaneSource> planeSource = nullptr;
}; // End class

} // End namespace

#endif // SiSSRView_h

