#ifndef sissr_CalculateBorderCells_h
#define sissr_CalculateBorderCells_h

// System
#include <set>

// ITK
#include <itkMacro.h>

namespace sissr {

template<typename TMesh>
std::set<unsigned int>
CalculateBorderCells(typename TMesh::Pointer i_mesh) {

  std::set<unsigned int> edge_cells;

  for (auto it = i_mesh->GetPoints()->Begin();
      it != i_mesh->GetPoints()->End(); ++it) {

    const auto edge = i_mesh->FindEdge(it.Index());
    itkAssertOrThrowMacro((nullptr != edge), "Edge not found (origin circle).");

    std::set<unsigned int> ring_id;
    std::set<unsigned int> ring_labels;

    auto temp = edge;
    do {
      temp = temp->GetOnext();
      ring_id.insert(temp->GetLeft());
      ring_labels.insert(i_mesh->GetCellData()->ElementAt(temp->GetLeft()));
    } while (temp != edge);

    if (ring_labels.size() > 1) {
      for (const auto id : ring_id) edge_cells.insert(id);
    }

  }

  return edge_cells;

}

} // namespace sissr

#endif