#ifndef sissr_CGALSurfaceMeshToITKMesh_h
#define sissr_CGALSurfaceMeshToITKMesh_h

#include <itkPolygonCell.h>

namespace sissr {

//
// This templated function takes a CGAL surface_mesh and converts it
// to an itk::QuadEdgeMesh by copying first the points and then the cells.
// If a property named "f:data" of the appropriate type exists in the
// surface_mesh, it is stored in the itk::QuadEdgeMesh as CellData.
// If a property named "v:data" of the appropriate type exists in the
// surface_mesh, it is stored in the itk::QuadEdgeMesh as PointData.
//

template<typename TCGALMesh, typename TITKMesh>
typename TITKMesh::Pointer CGALSurfaceMeshToITKMesh(const TCGALMesh &cgal_mesh) {

  using TCGALVertex = typename TCGALMesh::Vertex_index;
  using TCGALFace = typename TCGALMesh::Face_index;
  using TData = typename TITKMesh::PixelType;
  using TVertexDataMap = typename TCGALMesh::template Property_map<TCGALVertex,TData>;
  using TFaceDataMap = typename TCGALMesh::template Property_map<TCGALFace,TData>;

  const auto itk_mesh = TITKMesh::New();

  // Copy points and any associated point data.

  TVertexDataMap vertex_data;
  bool vertex_data_found;
  boost::tie(vertex_data, vertex_data_found) = cgal_mesh.template property_map<TCGALVertex,TData>("v:data");

  for (const auto i : cgal_mesh.vertices()) {
    typename TITKMesh::PointType p;
    p[0] = cgal_mesh.point(i)[0];
    p[1] = cgal_mesh.point(i)[1];
    p[2] = cgal_mesh.point(i)[2];
    itk_mesh->SetPoint(i, p);

    if (vertex_data_found) {
      itk_mesh->GetPointData()->SetElement(i, vertex_data[i]);
    }
  }

  // Copy cells and any associated cell data.

  bool cell_data_found;
  TFaceDataMap cell_data;
  boost::tie(cell_data, cell_data_found) = cgal_mesh.template property_map<TCGALFace,TData>("f:data");

  for (const auto i : cgal_mesh.faces()) {
    std::vector<size_t> ids;
    for (const auto vd : vertices_around_face(cgal_mesh.halfedge(i), cgal_mesh)) {
      ids.push_back(vd);
    }

    itk_mesh->AddFace( ids );
    if (cell_data_found) {
      itk_mesh->GetCellData()->SetElement(i, cell_data[i] );
    }
  }

  return itk_mesh;

}

}
#endif
