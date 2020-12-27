#ifndef itkGenerateInitialModelImageToMeshFilter_h
#define itkGenerateInitialModelImageToMeshFilter_h

#include "itkImageToMeshFilter.h"

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

  itkSetMacro(GeneralClosingRadius, unsigned int);
  itkGetMacro(GeneralClosingRadius, unsigned int);

  itkSetMacro(LVClosingRadius, unsigned int);
  itkGetMacro(LVClosingRadius, unsigned int);

  itkSetMacro(MeshNoiseSigma, float);
  itkGetMacro(MeshNoiseSigma, float);

  itkSetMacro(NumberOfCellsInDecimatedMesh, unsigned int);
  itkGetMacro(NumberOfCellsInDecimatedMesh, unsigned int);

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

};
} // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkGenerateInitialModelImageToMeshFilter.hxx"
#endif

#endif // itkGenerateInitialModelImageToMeshFilter
