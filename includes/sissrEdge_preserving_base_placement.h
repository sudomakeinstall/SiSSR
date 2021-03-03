#ifndef dv_Edge_preserving_base_placement_h
#define dv_Edge_preserving_base_placement_h

#include <CGAL/license/Surface_mesh_simplification.h>

#include <CGAL/Surface_mesh_simplification/internal/Common.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Edge_profile.h>

namespace CGAL {

namespace Surface_mesh_simplification {

template<class TM_, typename TData>
class Edge_preserving_base_placement
{
public:
  typedef TM_ TM;

  const std::string data_tag = "f:data";

  template <typename Profile>
  bool labels_uniform_around_vertex(const Profile& profile, const typename Profile::halfedge_descriptor &edge) const {

    const auto property = std::get<0>(profile.surface_mesh().template property_map<typename Profile::Triangle_mesh::Face_index,TData>(this->data_tag));

    bool uniform_around_vertex = true;

    const auto f = profile.surface_mesh().face(edge);
    const auto d = property[f];

    for (const auto& i : halfedges_around_source(edge, profile.surface_mesh())) {
      const auto fn = profile.surface_mesh().face(i);
      const auto dn = property[fn];
      if (d != dn) {
        uniform_around_vertex = false;
        break;
      }
    }

    return uniform_around_vertex;

  }

  template <typename Profile>
  bool edge_lies_on_boundary(const Profile& profile) const {

    const auto property = std::get<0>(profile.surface_mesh().template property_map<typename Profile::Triangle_mesh::Face_index,TData>(this->data_tag));
    const auto data_l = property[profile.surface_mesh().face(profile.v0_v1())];
    const auto data_r = property[profile.surface_mesh().face(profile.v1_v0())];
    return (data_l != data_r);

  }

};

}

}

#endif
