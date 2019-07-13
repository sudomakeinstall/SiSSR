#ifndef dv_GetPointsFromITKImage_h
#define dv_GetPointsFromITKImage_h

namespace dv
{

/*
 Get a sensible origin, point1, and point2 (in world coordinates) to initialize
 the vtkPlaneWidget, which is used to control which cross section of the data is viewed.
 */
template<typename TImage>
void
GetPointsFromITKImage(const typename TImage::Pointer image,
                      typename TImage::PointType &o,
                      typename TImage::PointType &p1,
                      typename TImage::PointType &p2)
{
  const typename TImage::SizeType size = image->GetLargestPossibleRegion().GetSize();
  const typename TImage::IndexType index = image->GetLargestPossibleRegion().GetIndex();

  auto o_index = index;
  o_index[2] += size[2] / 2;

  auto p1_index = index;
  p1_index[2] += size[2] / 2;
  p1_index[0] += size[0];

  auto p2_index = index;
  p2_index[1] += size[1];
  p2_index[2] += size[2] / 2;

  image->TransformIndexToPhysicalPoint(o_index, o);
  image->TransformIndexToPhysicalPoint(p1_index, p1);
  image->TransformIndexToPhysicalPoint(p2_index, p2);
}

}

#endif
