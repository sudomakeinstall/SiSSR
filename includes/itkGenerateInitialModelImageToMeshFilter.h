#ifndef itkGenerateInitialModelImageToMeshFilter_h
#define itkGenerateInitialModelImageToMeshFilter_h

// ITK
#include <itkImageToMeshFilter.h> 
// SiSSR
#include <sissrCGALDecimationTechnique.h>

namespace itk
{

/** \class GenerateInitialModelImageToMeshFilter
 *
 * \brief Filters a image by iterating over its pixels.
 *
 * Filters a image by iterating over its pixels in a multi-threaded way
 * and {to be completed by the developer}.
 *
 * \ingroup DVUtils
 *
 */
template <typename TInputImage, typename TOutputMesh>
class GenerateInitialModelImageToMeshFilter : public ImageToMeshFilter<TInputImage, TOutputMesh>
{
public:
  ITK_DISALLOW_COPY_AND_ASSIGN(GenerateInitialModelImageToMeshFilter);

  static constexpr unsigned int InputImageDimension = TInputImage::ImageDimension;
  using InputImageType = TInputImage;
  using OutputMeshType = TOutputMesh;
  using InputPixelType = typename InputImageType::PixelType;

  /** Standard class typedefs. */
  using Self = GenerateInitialModelImageToMeshFilter<InputImageType, OutputMeshType>;
  using Superclass = ImageToMeshFilter<InputImageType, OutputMeshType>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** Run-time type information. */
  itkTypeMacro(GenerateInitialModelImageToMeshFilter, ImageToMeshFilter);

  /** Standard New macro. */
  itkNewMacro(Self);

  using Superclass::SetInput;
  virtual void
  SetInput(const InputImageType * inputImage);

  using CGALDecimationTechnique = sissr::CGALDecimationTechnique;

  itkSetEnumMacro(DecimationTechnique, CGALDecimationTechnique);
  itkGetEnumMacro(DecimationTechnique, CGALDecimationTechnique);

  itkSetMacro(GeneralClosingRadius, unsigned int);
  itkGetConstMacro(GeneralClosingRadius, unsigned int);

  itkSetMacro(LVClosingRadius, unsigned int);
  itkGetConstMacro(LVClosingRadius, unsigned int);

  itkSetMacro(MeshNoiseSigma, float);
  itkGetConstMacro(MeshNoiseSigma, float);

  itkSetMacro(NumberOfCellsInDecimatedMesh, unsigned int);
  itkGetConstMacro(NumberOfCellsInDecimatedMesh, unsigned int);

  itkSetMacro(PreserveEdges, bool);
  itkGetConstMacro(PreserveEdges, bool);
  itkBooleanMacro(PreserveEdges);

protected:
  GenerateInitialModelImageToMeshFilter();
  ~GenerateInitialModelImageToMeshFilter() override = default;

  void PrintSelf(std::ostream & os, Indent indent) const override;

  using OutputRegionType = typename OutputMeshType::RegionType;

  void GenerateData() override;

private:
#ifdef ITK_USE_CONCEPT_CHECKING
  // Add concept checking such as
  // itkConceptMacro( FloatingPointPixel, ( itk::Concept::IsFloatingPoint< typename InputImageType::PixelType > ) );
#endif

  unsigned int m_GeneralClosingRadius;
  unsigned int m_LVClosingRadius;
  float        m_MeshNoiseSigma;
  unsigned int m_NumberOfCellsInDecimatedMesh;
  bool         m_PreserveEdges;
  CGALDecimationTechnique m_DecimationTechnique;

};
} // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkGenerateInitialModelImageToMeshFilter.hxx"
#endif

#endif // itkGenerateInitialModelImageToMeshFilter
