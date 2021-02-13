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

  using TLabels = std::set<typename TInputImage::PixelType>;
  using TLabelsPair = std::pair<TLabels,TLabels>;
  using TLabelsPairVector = std::vector<TLabelsPair>;

  TLabelsPairVector labels_pair_vector;
  labels_pair_vector.push_back(std::make_pair<TLabels,TLabels>(TLabels({2}),TLabels({3})));

  std::vector<typename TEnforce::Pointer> enforce;

  for (size_t i = 0; i < labels_pair_vector.size(); ++i) {
    enforce.push_back(TEnforce::New());
    enforce[i]->SetLabels1( labels_pair_vector[i].first );
    enforce[i]->SetLabels2( labels_pair_vector[i].second );
    if (enforce.size() > 1) {
      enforce[i]->SetInput( enforce[i - 1]->GetOutput() );
    } else {
      enforce[i]->SetInput( this->GetInput() );
    }
  }

  const auto connected = TConnected::New();
  if (enforce.size() > 0) {
    connected->SetInput( enforce.back()->GetOutput() );
  } else {
    connected->SetInput( this->GetInput() );
  }

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
