#pragma once

#include <iosfwd>

namespace my_engine
{

struct vertex
{
    float x = 0.f;
    float y = 0.f;
    float z = 0.f;
    
    float r = 0.f;
    float g = 0.f;
    float b = 0.f;
};

struct triangle
{
    triangle()
    {
        v[0] = vertex();
        v[1] = vertex();
        v[2] = vertex();
    }
    vertex v[3];
};

struct quad
{
    quad()
    {
        v[0] = vertex();
        v[1] = vertex();
        v[2] = vertex();
        v[3] = vertex();
    }
    vertex v[4];
};

std::istream& operator>>(std::istream& is, vertex&);
std::istream& operator>>(std::istream& is, triangle&);
std::istream& operator>>(std::istream& is, quad&);

} // namespace my_engine
