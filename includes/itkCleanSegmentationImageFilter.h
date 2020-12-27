#ifndef itkCleanSegmentationImageFilter_h
#define itkCleanSegmentationImageFilter_h

#include "itkImageToImageFilter.h"

namespace itk
{

/** \class CleanSegmentationImageFilter
 *
 * \brief Basic cleanup of a segmentation.
 *
 */
template <typename TInputImage, typename TOutputImage = TInputImage>
class CleanSegmentationImageFilter : public ImageToImageFilter<TInputImage, TOutputImage>
{
public:
  ITK_DISALLOW_COPY_AND_ASSIGN(CleanSegmentationImageFilter);

  static constexpr unsigned int InputImageDimension = TInputImage::ImageDimension;
  static constexpr unsigned int OutputImageDimension = TOutputImage::ImageDimension;

  using InputImageType = TInputImage;
  using OutputImageType = TOutputImage;
  using InputPixelType = typename InputImageType::PixelType;
  using OutputPixelType = typename OutputImageType::PixelType;
  using LabelSetType = std::set<InputPixelType>;

  /** Standard class typedefs. */
  using Self = CleanSegmentationImageFilter<InputImageType, OutputImageType>;
  using Superclass = ImageToImageFilter<InputImageType, OutputImageType>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** Run-time type information. */
  itkTypeMacro(CleanSegmentationImageFilter, ImageToImageFilter);

  /** Standard New macro. */
  itkNewMacro(Self);

protected:
  CleanSegmentationImageFilter();
  ~CleanSegmentationImageFilter() override = default;

  void PrintSelf(std::ostream & os, Indent indent) const override;

  using OutputRegionType = typename OutputImageType::RegionType;

  void GenerateData() override;

private:
#ifdef ITK_USE_CONCEPT_CHECKING
  // Add concept checking such as
  // itkConceptMacro( FloatingPointPixel, ( itk::Concept::IsFloatingPoint< typename InputImageType::PixelType > ) );
#endif

};
} // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkCleanSegmentationImageFilter.hxx"
#endif

#endif // itkCleanSegmentationImageFilter
