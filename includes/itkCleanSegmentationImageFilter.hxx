#ifndef itkCleanSegmentationImageFilter_hxx
#define itkCleanSegmentationImageFilter_hxx

#include "itkCleanSegmentationImageFilter.h"

#include <itkEnforceBoundaryBetweenLabelsImageFilter.h>
#include <itkExtractConnectedComponentsImageFilter.h>
#include <itkConstantPadImageFilter.h>

namespace itk
{

template <typename TInputImage, typename TOutputImage>
CleanSegmentationImageFilter<TInputImage, TOutputImage>
::CleanSegmentationImageFilter()
{
}


template <typename TInputImage, typename TOutputImage>
void
CleanSegmentationImageFilter<TInputImage, TOutputImage>
::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}


template <typename TInputImage, typename TOutputImage>
void
CleanSegmentationImageFilter<TInputImage, TOutputImage>
::GenerateData()
{

  using TEnforce = itk::EnforceBoundaryBetweenLabelsImageFilter<InputImageType>;
  using TConnected = itk::ExtractConnectedComponentsImageFilter<InputImageType>;
  using TPad = itk::ConstantPadImageFilter<TInputImage, TInputImage>;

  const auto enforce0 = TEnforce::New();
  enforce0->SetInput( this->GetInput() );
  enforce0->SetLabels1({4, 5});
  enforce0->SetLabels2({2, 3, 6, 7, 8, 9});

  const auto enforce1 = TEnforce::New();
  enforce1->SetInput( enforce0->GetOutput() );
  enforce1->SetLabels1({3});
  enforce1->SetLabels2({1, 6, 7, 8, 9});

  const auto connected = TConnected::New();
  connected->SetInput( enforce1->GetOutput() );

  typename TInputImage::SizeType padding;
  padding.Fill(1);

  const auto pad = TPad::New();
  pad->SetInput(connected->GetOutput());
  pad->SetPadUpperBound(padding);
  pad->SetPadLowerBound(padding);
  pad->SetConstant(static_cast<typename TInputImage::PixelType>(0));
  pad->Update();

  this->GetOutput()->Graft( pad->GetOutput() );

}

} // end namespace itk

#endif // itkCleanSegmentationImageFilter_hxx
