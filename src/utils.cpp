#include <glm/glm.hpp>
#include "utils.hpp"
#include "edge.hpp"
#include "vertex.hpp"
#include "face.hpp"
#include "mesh.hpp"

bool faceInCommon(Edge *e1, Edge *e2, ID *id)
{
    for(ID f1: e1->faces())
    {
        for(ID f2: e2->faces())
        {
            if(f1 == f2)
            {
                *id = f1;
                return true;
            }
        }
    }
    return false;
}

float computeArea(const glm::vec3 &A, const glm::vec3 &B, const glm::vec3 &C)
{
    return glm::cross(B-A,C-A).length()/2.0f;
}

std::string getFileExt(const std::string& s) {

    size_t i = s.rfind('.', s.length());
    if (i != std::string::npos) {
        return(s.substr(i+1, s.length() - i));
    }

    return("");
}

Edge *findEdge(std::map<std::pair<int, int>, Edge *> &map, Vertex *a, Vertex *b)
{
    std::map<std::pair<int, int>, Edge *>::iterator it = map.find({a->id(), b->id()});
    if (it == map.end())
        return nullptr;
    else
        return it->second;
}

std::vector<ID> facesInCommon(Edge *e1, Edge *e2)
{
    std::vector<ID> fic;
    for(ID f1: e1->faces())
    {
        for(ID f2: e2->faces())
        {
            if(f1 == f2)
            {
                fic.push_back(f1);
                break;
            }
        }
    }
    return fic;
}