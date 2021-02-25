#ifndef dv_Edge_preserving_midpoint_placement_h
#define dv_Edge_preserving_midpoint_placement_h

#include <CGAL/Surface_mesh_simplification/internal/Common.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Edge_profile.h>

namespace CGAL {

namespace Surface_mesh_simplification {

template<class TM_, typename TData>
class Edge_preserving_midpoint_placement
{
public:
  typedef TM_ TM;

  Edge_preserving_midpoint_placement() {}

  template <typename Profile>
  bool labels_uniform_around_vertex(const Profile& profile, const typename Profile::halfedge_descriptor &edge) const {

    const auto property = std::get<0>(profile.surface_mesh().template property_map<typename Profile::Triangle_mesh::Face_index,TData>("f:data"));

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
  boost::optional<typename Profile::Point> operator()(const Profile& profile) const
  {
    const auto property = std::get<0>(profile.surface_mesh().template property_map<typename Profile::Triangle_mesh::Face_index,TData>("f:data"));

    const auto data_l = property[profile.surface_mesh().face(profile.v0_v1())];
    const auto data_r = property[profile.surface_mesh().face(profile.v1_v0())];
    bool edge_lies_on_boundary = (data_l != data_r);
    bool uniform_around_source = this->labels_uniform_around_vertex<Profile>(profile, profile.v0_v1());
    bool uniform_around_target = this->labels_uniform_around_vertex<Profile>(profile, profile.v1_v0());
    bool edge_is_internal = (uniform_around_source && uniform_around_target);

    using result_type = boost::optional<typename Profile::Point>;

    if (edge_is_internal || edge_lies_on_boundary) {
      // If the edge is entirely internal, or if the edge
      // lies on a boundary, it may be contracted to the midpoint.
      return result_type(profile.geom_traits().construct_midpoint_3_object()(profile.p0(), profile.p1()));
    } else if (uniform_around_source) {
      // If only the target lies on the boundary,
      // it should be contracted to the target.
      return result_type(profile.p1());
    } else if (uniform_around_target) {
      // If only the source lies on the boundary,
      // it should be contracted to the source.
      return result_type(profile.p0());
    } else {
      // In this case, the source and target lie on different
      // boundaries (connecting two separated components).
      // In this case, the edge should not be contracted.
      return result_type();
    }

  }

};

}

}

#endif
