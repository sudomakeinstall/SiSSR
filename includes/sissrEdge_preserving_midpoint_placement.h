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
  typedef TM_                                                     TM;

  Edge_preserving_midpoint_placement() {}

  template <typename Profile>
  boost::optional<typename Profile::Point> operator()(const Profile& profile) const
  {
    const auto property = std::get<0>(profile.surface_mesh().template property_map<typename Profile::Triangle_mesh::Face_index,TData>("f:data"));

    bool uniform_around_source = true;

    {
      // Iterate around the source vertex and determine
      // whether the faces surrounding the source vertex
      // all share the same data value.
      const auto f = profile.surface_mesh().face(profile.v0_v1());
      const auto d = property[f];
      typename Profile::halfedge_descriptor i = profile.v0_v1();
      do {
        i = profile.surface_mesh().next_around_source(i);
        const auto fn = profile.surface_mesh().face(i);
        const auto dn = property[fn];
        if (d != dn) {
          uniform_around_source = false;
          break;
        }
      } while (i != profile.v0_v1());
    }

    bool uniform_around_target = true;

    {
      // Iterate around the source vertex and determine
      // whether the faces surrounding the source vertex
      // all share the same data value.
      const auto f = profile.surface_mesh().face(profile.v1_v0());
      const auto d = property[f];
      typename Profile::halfedge_descriptor i = profile.v1_v0();
      do {
        i = profile.surface_mesh().next_around_source(i);
        const auto fn = profile.surface_mesh().face(i);
        const auto dn = property[fn];
        if (d != dn) {
          uniform_around_target = false;
          break;
        }
      } while (i != profile.v1_v0());
    }

    typedef boost::optional<typename Profile::Point>              result_type;

    if (uniform_around_source == uniform_around_target) {
      return result_type(profile.geom_traits().construct_midpoint_3_object()(profile.p0(), profile.p1()));
    } else if (uniform_around_source) {
      return result_type(profile.p1());
    } else {
      return result_type(profile.p0());
    }

  }
};

}
}

#endif
