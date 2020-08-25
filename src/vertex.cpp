#include "vertex.hpp"
#include "edge.hpp"
#include "utils.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <sstream>

ID Vertex::m_gid = 0;

Vertex::Vertex()
{
    init();
}

Vertex::Vertex(const glm::vec3 &v)
{
    m_pos = v;
    init();
}

void Vertex::init()
{
    m_locked = false;

    m_id = m_gid++;
}

void Vertex::addEdge(Edge *e)
{
    m_edges.push_back(e);
}

void Vertex::removeEdge(Edge *e)
{
    erase(m_edges, e);
}

void Vertex::position(const glm::vec3 &v)
{
    m_pos = v;
}

void Vertex::locked(bool b)
{
    m_locked = b;
}

void Vertex::exportId(ID id)
{
    m_exportId = id;
}

void Vertex::state(VertexState state) {
    m_state = state;
}

Vertex::VertexState Vertex::state() const { return m_state; }
const glm::vec3 &Vertex::position() const { return m_pos; }
ID Vertex::id() const { return m_id; }
std::vector<Edge *> &Vertex::edges() { return m_edges; }
bool Vertex::locked() const { return m_locked; }

ID Vertex::exportId() const
{
    return m_exportId;
}

std::ostream& operator<<(std::ostream &o, Vertex &v)
{
    std::stringstream ss;
    ss << "v" << v.id() << ": " << glm::to_string(v.position());
    ss << "{ ";
    for(auto e: v.edges())
    {
        ss << "e" << (*e).id() << " ";
    }
    ss << "}";
    return o << ss.str() ;
}
