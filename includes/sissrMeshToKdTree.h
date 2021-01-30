#ifndef sissr_MeshToKdTree_h
#define sissr_MeshToKdTree_h

// ITK
#include <itkPointSet.h>
#include <itkPointsLocator.h>

namespace sissr {

template<typename TMesh>
class MeshToKdTree {
  public:

    using TCoordinate = typename TMesh::PixelType;
    static constexpr unsigned int Dimension = TMesh::PointDimension;
    using TPointSet = itk::PointSet<TCoordinate, Dimension>;
    using TLocator = itk::PointsLocator<typename TPointSet::PointsContainer>;

    void Calculate(TMesh* mesh, TLocator* locator) {

      const auto pointset = TPointSet::New();

      for (auto it = mesh->GetCells()->Begin();
           it != mesh->GetCells()->End();
           ++it) {

        const auto cell = it.Value();

        typename TMesh::PointType centroid;
        centroid.SetToMidPoint(
          mesh->GetPoint(cell->GetPointIds()[0]),
          mesh->GetPoint(cell->GetPointIds()[2])
        );

        pointset->SetPoint( it.Index(), centroid );

      }

      locator->SetPoints( pointset->GetPoints() );
      locator->Initialize();

    }

};

}

#endif
