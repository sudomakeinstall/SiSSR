#ifndef dv_CalculateResidualMesh_h
#define dv_CalculateResidualMesh_h

// ITK
#include <itkPointsLocator.h>
#include <itkLineCell.h>

namespace sissr {

template<typename TPointSet, typename TLoop, typename TMesh>
class CalculateResidualMesh
{

  typedef itk::PointsLocator< typename TPointSet::PointsContainer > TLocator;
  typedef typename TMesh::CellType::CellAutoPointer CellAutoPointer;
  typedef itk::LineCell< typename TMesh::CellType > LineType;

  typename TLocator::Pointer locator;
  typename TLoop::Pointer loop;

public:

  CalculateResidualMesh(typename TLocator::Pointer _locator,
                        typename TLoop::Pointer _loop) :
    locator(_locator),
    loop(_loop)
    {}

  typename TMesh::Pointer
  Calculate()
    {

    const auto r = TMesh::New();

    for (unsigned int u = 0;
         u < loop->GetSurfaceParameterList().size();
         ++u)
      {
      const auto param = loop->GetSurfaceParameterList().at(u);
      const auto b = loop->GetPointOnSurface(param.first, param.second);
      const auto e_index = locator->FindClosestPoint(b);
      const auto e = locator->GetPoints()->ElementAt(e_index);

      r->SetPoint(2*u  , b);
      r->SetPoint(2*u+1, e);

      CellAutoPointer line;
      line.TakeOwnership( new LineType );
      line->SetPointId(0, 2*u  );
      line->SetPointId(1, 2*u+1);

      r->SetCell( u, line );
      }

    return r;

    }

};

} // namespace sissr

#endif
