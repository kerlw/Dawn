#ifndef A_STAR_HPP
#define A_STAR_HPP

#include <vector>

#include "tileset.hpp" // for struct Point

std::vector<Point> aStar( const Point &start,
                          const Point &end,
                          int width,
                          int height,
                          int granularity = 10,
                          int maxIterations=1000 );

#endif
