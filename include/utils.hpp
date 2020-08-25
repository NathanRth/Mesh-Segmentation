#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <algorithm>
#include <glm/vec3.hpp>
#include <string>
#include <map>

class Edge;
class Vertex;
class Mesh;

using ID = std::size_t;

template <typename T>
typename std::vector<T>::const_iterator find(const std::vector<T> &v, const T& e)
{
    return std::find(v.begin(), v.end(), e);
}

template <typename T>
void erase(std::vector<T> &v, T &e)
{
    auto it = find(v, e);
    if(it!= v.end())
        v.erase(it);
}

template <typename T>
void bubbleSort(std::vector<T> &v)
{
    for(int i = v.size()-1; i>=1 ; i--)
    {
        for(int j = 0; j < i; j++)
        {
            if(v[j+1] < v[j])
            {
                std::swap(v[j+1], v[j]);
            }
        }
    }
}

bool faceInCommon(Edge *e1, Edge *e2, ID *id);
std::vector<ID> facesInCommon(Edge *e1, Edge *e2);
float computeArea(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c);
std::string getFileExt(const std::string& s);
Edge *findEdge(std::map<std::pair<int, int>, Edge *> &map, Vertex *a, Vertex *b);

#endif // UTILS_HPP
