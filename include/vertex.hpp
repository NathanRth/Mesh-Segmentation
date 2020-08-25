#ifndef VERTEX_HPP
#define VERTEX_HPP

#include "utils.hpp"
#include <glm/vec3.hpp>
#include <vector>
#include <ostream>

class Edge;

class Vertex
{
public:
    Vertex();
    Vertex(const glm::vec3 &v);

    typedef enum
    {
        TO_VISIT,
        VISITED,
        SEEN
    } VertexState;

    void addEdge(Edge *e);
    void removeEdge(Edge *e);

    // setter
    void position(const glm::vec3 &v);
    void locked(bool b);
    void exportId(ID id);
    void state(VertexState state);

    // getter
    const glm::vec3 &position() const;
    ID id() const;
    std::vector<Edge *> &edges();
    bool locked() const;
    ID exportId() const;
    Vertex::VertexState state() const;

    friend std::ostream& operator<<(std::ostream &o, Vertex &v);

private:
    void init();

    glm::vec3 m_pos;
    ID m_id;
    ID m_exportId;
    std::vector<Edge *> m_edges;
    bool m_locked;
    static ID m_gid;
    Vertex::VertexState m_state;
};

#endif
