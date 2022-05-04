#ifndef _TILEGROUP_H_
#define _TILEGROUP_H_

namespace tilepuzzles {

template <typename T>
struct TileGroup {
  math::float2 anchorPoint;
  std::vector<T> tileGroup;
  bool dragable;
  math::int2 gridCoord;

  TileGroup() {
  }

  TileGroup(math::float2 anchorPoint, std::vector<T> tileGroup, bool dragable, math::int2 gridCoord)
    : anchorPoint(anchorPoint), tileGroup(tileGroup), dragable(dragable), gridCoord(gridCoord) {
  }
};
} // namespace tilepuzzles

#endif