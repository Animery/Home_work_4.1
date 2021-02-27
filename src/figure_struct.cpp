// #include "../include/engine.hpp"
#include "../include/figure_struct.hpp"

#include <istream>

namespace my_engine
{

std::istream& operator>>(std::istream& is, vertex& vertex)
{
    is >> vertex.x;
    is >> vertex.y;
    is >> vertex.z;
    
    is >> vertex.r;
    is >> vertex.g;
    is >> vertex.b;

    return is;
}

std::istream& operator>>(std::istream& is, triangle& triang)
{
    is >> triang.v[0];
    is >> triang.v[1];
    is >> triang.v[2];
    return is;
}

std::istream& operator>>(std::istream& is, quad& quadr)
{
    is >> quadr.v[0];
    is >> quadr.v[1];
    is >> quadr.v[2];
    is >> quadr.v[3];
    return is;
}

} // namespace my_engine