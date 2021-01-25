#ifndef sissr_LabeledMeshToKdTreeMap_h
#define sissr_LabeledMeshToKdTreeMap_h

// STD
#include <map>

// ITK
#include <itkPointSet.h>
#include <itkPointsLocator.h>

namespace sissr {

template<typename TMesh>
class LabeledMeshToKdTreeMap {
  public:

    using TCoordinate = typename TMesh::PixelType;
    static constexpr unsigned int Dimension = TMesh::PointDimension;
    using TPointSet = itk::PointSet<TCoordinate, Dimension>;
    using TLocator = itk::PointsLocator<typename TPointSet::PointsContainer>;
    using TLocatorMap = std::map<size_t, typename TLocator::Pointer>;

    TLocatorMap Calculate(TMesh* mesh) {

      std::map<size_t, typename TPointSet::Pointer> pointset_map;

      for (auto it = mesh->GetCells()->Begin();
           it != mesh->GetCells()->End();
           ++it) {

        const auto cell = it.Value();

        typename TMesh::PointType centroid;
        centroid.SetToMidPoint(
          mesh->GetPoint(cell->GetPointIds()[0]),
          mesh->GetPoint(cell->GetPointIds()[2])
        );

        const auto label = mesh->GetCellData()->ElementAt( it.Index() );

        itkAssertOrThrowMacro(label != 0, "Label == 0");

        if (0 == pointset_map.count(label)) {
          pointset_map[label] = TPointSet::New();
        }

        pointset_map[label]->SetPoint( it.Index(), centroid );

      }

      TLocatorMap locator_map;

      for (const auto& pointset : pointset_map) {

        locator_map[pointset.first] = TLocator::New();
        locator_map[pointset.first]->SetPoints( pointset.second->GetPoints() );
        locator_map[pointset.first]->Initialize();

      }

      return locator_map;

    }

};

}

#endif
