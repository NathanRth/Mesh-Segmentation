#ifndef FACE_HPP
#define FACE_HPP

#include "utils.hpp"

#include <ostream>
#include <vector>
#include <map>

class Edge;
class Vertex;

class Face
{
public:
    Face(ID a, ID b, ID c, float area);

    ID A() const;
    ID B() const;
    ID C() const;
    float area() const;
    ID id() const;

    friend std::ostream& operator<<(std::ostream &o, const Face &f);

private:
    ID m_A, m_B, m_C;
    ID m_id;
    float m_area;

    static ID m_gid;
};

#endif // FACE_HPP
