#include "edge.hpp"
#include "vertex.hpp"
#include "utils.hpp"

#include <sstream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

ID Edge::m_gid = 0;

Edge::Edge()
{
    m_cost = 0;
    m_v1 = nullptr;
    m_v2 = nullptr;
    init();
}

Edge::Edge(Vertex *v1, Vertex *v2)
{
    m_v1 = v1;
    m_v2 = v2;
    computeCost();
    init();
}

void Edge::init()
{
    m_id = m_gid++;
    m_type = MESH;
    m_state = TO_VISIT;
}

std::vector<Edge *> Edge::getConnectedEdges()
{
    std::vector<Edge*> edges;

    for(auto e: m_v1->edges())
    {
        if(e != this)
        {
            edges.push_back(e);
        }
    }
    for(auto e: m_v2->edges())
    {
        if(e != this)
        {
            edges.push_back(e);
        }
    }

    return edges;
}

Vertex *Edge::getMeanPosition()
{
    return new Vertex(glm::vec3((m_v1->position().x + m_v2->position().x) / 2,
                                (m_v1->position().y + m_v2->position().y) / 2,
                                (m_v1->position().z + m_v2->position().z) / 2));
}

void Edge::addFace(ID f)
{
    m_faces.push_back(f);
}

void Edge::removeFace(ID f)
{
    erase(m_faces, f);
}

void Edge::addFaceATL(std::size_t id)
{
    auto it = find(m_ATL,id);
    if(it == m_ATL.end())
    {
        m_ATL.push_back(id);
    }
}

void Edge::v1(Vertex *v1)
{
    if(m_v2 == nullptr) {
        m_v1 = v1;
    }
    else if(v1->id() <= m_v2->id())
    {
        // we unsuscribe this edge from the old vertex
        if(m_v1 != nullptr)
        {
            m_v1->removeEdge(this);
        }
        m_v1 = v1;
    }
    else
    {
        if(m_v1 != nullptr)
        {
            m_v1->removeEdge(this);
        }
        m_v1 = m_v2;
        m_v2 = v1;
    }
    computeCost();
}

void Edge::v2(Vertex *v2)
{
    if(m_v1 == nullptr) {
        m_v2 = v2;
    }
    else if(v2->id() >= m_v1->id())
    {
        if(m_v2 != nullptr)
        {
            m_v2->removeEdge(this);
        }
        m_v2 = v2;
    }
    else
    {
        if(m_v2 != nullptr)
        {
            m_v2->removeEdge(this);
        }
        m_v2 = m_v1;
        m_v1 = v2;
    }
    computeCost();
}

void Edge::type(Edge::EdgeType type)
{
    m_type = type;
}

void Edge::state(Edge::EdgeState state)
{
    m_state = state;
}

Vertex *Edge::v1() const { return m_v1; }
Vertex *Edge::v2() const { return m_v2; }
std::vector<ID> &Edge::faces() { return  m_faces; }
std::vector<ID> &Edge::ATL() { return  m_ATL; }
Edge::EdgeType Edge::type() const { return m_type; }
Edge::EdgeState Edge::state() const { return m_state; }
float Edge::cost() const { return m_cost; }
ID Edge::id() const { return m_id; }

bool Edge::compEdgeId(Edge *a, Edge *b) {return a->id() < b->id();}
bool Edge::compEdgeCost(Edge *a, Edge *b) {return a->m_cost < b->m_cost;}

void Edge::computeCost()
{
    if(m_v1 != nullptr && m_v2 != nullptr)
    {
        m_cost = glm::length2(m_v1->position() - m_v2->position());
    }
    else
    {
        m_cost = 0;
    }
}

std::ostream &operator<<(std::ostream &o, Edge &e)
{
    std::stringstream ss;
    std::stringstream ss2;
    for(auto id : e.ATL())
    {
        ss << " f"<< id;
    }
    for(auto id : e.faces())
    {
        ss2 <<" f"<< id;
    }
    return o << "e" << e.id() << ":[v" << e.v1()->id() << ",v" << e.v2()->id() << "]("<<e.faces().size()<<")("<<ss.str()<<")("<<(e.type() == Edge::MESH ? "M)" : (e.type()==Edge::VIRTUAL ? "V)":"B)"))<<ss2.str();
}
