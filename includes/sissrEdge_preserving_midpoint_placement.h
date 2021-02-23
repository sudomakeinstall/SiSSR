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

    typename Profile::halfedge_descriptor i = edge;

    const auto f = profile.surface_mesh().face(edge);
    const auto d = property[f];

    do {
      i = profile.surface_mesh().next_around_source(i);
      const auto fn = profile.surface_mesh().face(i);
      const auto dn = property[fn];
      if (d != dn) {
        uniform_around_vertex = false;
        break;
      }
    } while (i != edge);

    return uniform_around_vertex;

  }

  template <typename Profile>
  boost::optional<typename Profile::Point> operator()(const Profile& profile) const
  {
    const auto property = std::get<0>(profile.surface_mesh().template property_map<typename Profile::Triangle_mesh::Face_index,TData>("f:data"));

    bool uniform_around_source = this->labels_uniform_around_vertex<Profile>(profile, profile.v0_v1());
    bool uniform_around_target = this->labels_uniform_around_vertex<Profile>(profile, profile.v1_v0());

    typedef boost::optional<typename Profile::Point>              result_type;

    if (uniform_around_source && uniform_around_target) {
      return result_type(profile.geom_traits().construct_midpoint_3_object()(profile.p0(), profile.p1()));
    } else if (uniform_around_source) {
      return result_type(profile.p1());
    } else if (uniform_around_target) {
      return result_type(profile.p0());
    } else {
      return result_type();
    }

  }

};

}

}

#endif
