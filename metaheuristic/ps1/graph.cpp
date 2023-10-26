#include <graph.hpp>

Edge calculate_edge_between(Vertex const& src, Vertex const& dst)
{
  i64 const dx = dst.x - src.x;
  i64 const dy = dst.y - src.y;
  i64 const weight = anton::math::round(anton::math::sqrt(dx * dx + dy * dy));
  return Edge{.src = src.index, .dst = dst.index, .weight = weight};
}
