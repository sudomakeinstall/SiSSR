#ifndef sissr_CalculateResidualMesh_h
#define sissr_CalculateResidualMesh_h

// STD
#include <map>

// ITK
#include <itkPointsLocator.h>
#include <itkLineCell.h>

// SiSSR
#include <sissrLabeledMeshToKdMap.h>

namespace sissr {

template<typename TLabeledMesh, typename TLoop, typename TMesh>
class CalculateResidualMesh
{

  using TKd = itk::PointsLocator< typename TLabeledMesh::PointsContainer >;
  using TKdMap = std::map<size_t, typename TKd::Pointer>;
  using TLabeledMeshToKdMap = LabeledMeshToKdMap<TLabeledMesh>;
  using CellAutoPointer = typename TMesh::CellType::CellAutoPointer;
  using LineType = itk::LineCell<typename TMesh::CellType>;

  TKdMap kd_map;
  typename TLoop::Pointer loop;

public:

  CalculateResidualMesh(typename TMesh::Pointer _mesh,
                        typename TLoop::Pointer _loop) :
    loop(_loop)
    {
      TLabeledMeshToKdMap mesh_to_map;
      this->kd_map = mesh_to_map.Calculate(_mesh);
    }

  typename TMesh::Pointer
  Calculate()
    {

    const auto r = TMesh::New();

    for (unsigned int u = 0;
         u < loop->GetSurfaceParameterList().size();
         ++u)
      {
      const auto [cell_id, params] = loop->GetSurfaceParameterList().at(u);
      const auto b = loop->GetPointOnSurface(cell_id, params);
      const auto l = loop->GetCellData()->ElementAt(cell_id);
      const auto e_index = kd_map[l]->FindClosestPoint(b);
      const auto e = kd_map[l]->GetPoints()->ElementAt(e_index);

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
