#ifndef sissr_ITKMeshToCGALSurfaceMesh_h
#define sissr_ITKMeshToCGALSurfaceMesh_h

namespace sissr {

//
// This templated function takes an itk::QuadEdgeMesh and converts it to
// a CGAL surface_mesh by copying first the points and then the cells.
// If CellData exists, it is stored as a property with tag "f:data".
// If PointData exists, it is stored as a property with tag "v:data".
//

template<typename TITKMesh, typename TCGALMesh>
TCGALMesh ITKMeshToCGALSurfaceMesh(typename TITKMesh::Pointer itk_mesh) {

  using TCGALVertex = typename TCGALMesh::Vertex_index;
  using TCGALFace = typename TCGALMesh::Face_index;
  using TData = typename TITKMesh::PixelType;
  using TVertexDataMap = typename TCGALMesh::template Property_map<TCGALVertex,TData>;
  using TFaceDataMap = typename TCGALMesh::template Property_map<TCGALFace,TData>;

  TCGALMesh cgal_mesh;

  // Copy points and any associated point data.

  bool vertex_data_exists = (itk_mesh->GetNumberOfPoints() == itk_mesh->GetPointData()->Size());

  TVertexDataMap vertex_data;
  bool vertex_data_created;
  boost::tie(vertex_data, vertex_data_created) = cgal_mesh.template add_property_map<TCGALVertex,TData>("v:data",-1);

  for (auto it = itk_mesh->GetPoints()->Begin(); it != itk_mesh->GetPoints()->End(); ++it) {
    typename TCGALMesh::Point p(it.Value()[0], it.Value()[1], it.Value()[2]);
    const auto vertex_id = cgal_mesh.add_vertex(p);

    if (vertex_data_exists && vertex_data_created) {
      vertex_data[vertex_id] = itk_mesh->GetPointData()->ElementAt(it.Index());
    }
  }

  // Copy cells and any associated cell data.

  bool cell_data_exists = (itk_mesh->GetNumberOfCells() == itk_mesh->GetCellData()->Size());
  TFaceDataMap face_data;
  bool face_data_created;
  boost::tie(face_data, face_data_created) = cgal_mesh.template add_property_map<TCGALFace,TData>("f:data",-1);

  for (auto it = itk_mesh->GetCells()->Begin(); it != itk_mesh->GetCells()->End(); ++it) {
    std::vector<CGAL::SM_Vertex_index> point_ids;
    for (auto point_id = it.Value()->PointIdsBegin(); point_id != it.Value()->PointIdsEnd(); ++point_id) {
      point_ids.push_back(static_cast<CGAL::SM_Vertex_index>(*point_id));
    }
    const auto face_id = cgal_mesh.add_face(point_ids);

    if (cell_data_exists && face_data_created) {
      face_data[face_id] = itk_mesh->GetCellData()->ElementAt(it.Index());
    }
  }

  return cgal_mesh;

};

}

#endif
