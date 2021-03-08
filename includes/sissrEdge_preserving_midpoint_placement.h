#ifndef dv_Edge_preserving_midpoint_placement_h
#define dv_Edge_preserving_midpoint_placement_h

#include <CGAL/license/Surface_mesh_simplification.h>

#include <CGAL/Surface_mesh_simplification/internal/Common.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Edge_profile.h>

#include <sissrEdge_preserving_base_placement.h>

namespace CGAL {

namespace Surface_mesh_simplification {

template<class TM_, typename TData>
class Edge_preserving_midpoint_placement :
  public Edge_preserving_base_placement<TM_, TData>
{
public:
  using TM = TM_;

  const std::string data_tag = "f:data";

  Edge_preserving_midpoint_placement() {}

  template <typename Profile>
  boost::optional<typename Profile::Point> operator()(const Profile& profile) const
  {

    bool edge_lies_on_boundary = this->template edge_lies_on_boundary<Profile>(profile);
    bool uniform_around_source = this->template labels_uniform_around_vertex<Profile>(profile, profile.v0_v1());
    bool uniform_around_target = this->template labels_uniform_around_vertex<Profile>(profile, profile.v1_v0());
    bool edge_is_internal = (uniform_around_source && uniform_around_target);

    using result_type = boost::optional<typename Profile::Point>;

    if (edge_is_internal || edge_lies_on_boundary) {
      // If the edge is entirely internal, or if the edge
      // lies on a boundary, it may be contracted to the midpoint.
      return result_type(this->contract_edge(profile));
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

  template <typename Profile>
  boost::optional<typename Profile::Point> contract_edge(const Profile& profile) const
  {
    using result_type = boost::optional<typename Profile::Point>;
    return result_type(profile.geom_traits().construct_midpoint_3_object()(profile.p0(), profile.p1()));
  }

};

}

}

#endif
