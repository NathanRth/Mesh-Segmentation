#include "face.hpp"

ID Face::m_gid = 0;

Face::Face(ID a, ID b, ID c, float area)
{
    m_A = a;
    m_B = b;
    m_C = c;
    m_area = area;

    m_id = m_gid++;
}

ID Face::A() const{ return m_A; }
ID Face::B() const{ return m_B; }
ID Face::C() const{ return m_C; }
float Face::area() const { return m_area; }
ID Face::id() const { return m_id; }

std::ostream& operator<<(std::ostream &o, const Face &f)
{
    return o << "[v" << f.m_A << " v" << f.m_B << " v" << f.m_C << "]";
}
