#ifndef MESH_HPP
#define MESH_HPP

#include <vector>
#include <glm/glm.hpp>
#include <string>
#include "utils.hpp"

class Edge;
class Vertex;
class Face;

class Mesh
{
public:
    Mesh();
    Mesh(std::string filename);

    int importOBJ(std::string filename);
    int importOFF(std::string filename);
    int exportOBJ(std::string filename);
    void exportMesh(std::string filename);
    int simpleExport(std::string filename);

    void skeletonization();
    void segmentation(std::string filename);
    void dissolveEdge(Edge *e);

    void addFace(Face *face);
    void addVertex(Vertex *v);
    void removeEdge(Edge *e);
    void removeVertex(Vertex *v);

    // getter
    const std::vector<Edge *> &edges() { return m_edges; }
    std::vector<Face*> faces();
    
    void debug();
    int allFacesInATL();

private:

    typedef struct EdgeCouple
    {
        EdgeCouple(Vertex *a, Vertex *b)
        {
            v1 = a;
            v2 = b;
        }
        Vertex *v1;
        Vertex *v2;
    } EdgeCouple;

    typedef struct
    {
        Edge *e1;
        Edge *e2;
        bool fic;
        ID f_id;
    } FIC;

    typedef struct
    {
        glm::vec3 pos;
        ID id;
    } SimpleVertex;

    std::vector<Edge*> m_edges;
    std::vector<Vertex*> m_vertices;

    std::vector<SimpleVertex> m_raw_vertices;
    std::vector<Face*> m_faces;

    std::vector<std::vector<Face*>> m_components;
};

#endif // MESH_HPP
