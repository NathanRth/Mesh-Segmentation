#include "mesh.hpp"

#include <map>
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "vertex.hpp"
#include "edge.hpp"
#include "face.hpp"

Mesh::Mesh()
{

}

Mesh::Mesh(std::string filename)
{
    bool fail = false;
    std::string ext = getFileExt(filename);
    if(ext == "obj" || ext == "OBJ")
    {
        if(!importOBJ(filename))
            fail = true;
    }
    else if(ext == "off" || ext == "OFF")
    {
        if(!importOFF(filename))
            fail = true;
    }
    else
    {
        throw "File extension unknown";
    }
    if(fail)
    {
        throw "Failed to initialize mesh";
    }
}

void Mesh::segmentation(std::string filename) {

    for(unsigned int i=0; i<m_vertices.size(); i++) {

        if((m_vertices.at(i)->state() != Vertex::SEEN) && (m_vertices.at(i)->edges().size() != 1)) {
            continue;
        }

        std::vector<Face*> component;

        Vertex* currentV = m_vertices.at(i);
        Edge* currentE = currentV->edges().at(0);

        currentV->state(Vertex::SEEN);
        currentE->state(Edge::SEEN);

        // add ATL to component
        for(unsigned int j=0; j<currentE->ATL().size(); j++) {
            if(currentE->ATL().at(j) < m_faces.size())
                component.push_back( m_faces.at( currentE->ATL().at(j) ) );
        }


        while(currentV->edges().size() <= 2) {

            // get next vertex
            if(currentE->v1()->id() == currentV->id()) {
                currentV = currentE->v2();
            } else {
                currentV = currentE->v1();
            }

            // no need to go further if it has already been seen
            if((currentV->edges().size() == 1) || (currentV->state() == Vertex::SEEN)) {
                currentV->state(Vertex::SEEN);
                break;
            }

            // else, continue to expand the component
            for(unsigned int j=0; j<currentV->edges().size(); j++) {
                if(currentV->edges().at(j)->state() == Edge::TO_VISIT) {
                    currentE = currentV->edges().at(j);
                }
            }

            currentV->state(Vertex::SEEN);
            currentE->state(Edge::SEEN);

            // add ATL to component
            for(unsigned int j=0; j<currentE->ATL().size(); j++) {
                if(currentE->ATL().at(j) < m_faces.size())
                    component.push_back( m_faces.at( currentE->ATL().at(j) ) );
            }   
        }  

        std::cout << "over !" << std::endl;
        m_components.push_back(component);
    }

    /*std::vector<Face*> component;

    for(auto edge: m_edges) {
        for(auto face: edge->ATL()) {
            std::cout << *(m_faces.at(face)) << std::endl;
            component.push_back(m_faces.at(face));
        }
    }

    m_components.push_back(component);*/

    exportMesh(filename);
}

int Mesh::importOBJ(std::string filename)
{
    std::map<std::pair<int, int>, Edge *> edge_map;
    std::ifstream file;   // input file
    std::string line;     // current line

    int numLine = 0;       // number of the current line

    // open file
    file.open(filename);
    if (!file.is_open())
    {
        std::cerr << "Unable to open file: " << filename << std::endl;
        return 0;
    }
    else
    {
        std::cout << "filename: " << filename << std::endl;
    }

    while(std::getline(file, line))
    {
        numLine++;

        std::string type;

        // lecture de la ligne
        std::istringstream iss(line);

        // lecture du flag
        iss >> type;

        if(type == "v") // vertex declaration
        {
            float x, y, z;
            // tries to read at least 3 consecutive numbers as float
            if (iss >> x >> y >> z)
            {
                m_vertices.push_back(new Vertex({x, y, z}));
                m_raw_vertices.push_back({{x,y,z},0});
            }
            else
            {
                std::cout << "Wrong vertex format (at " << numLine << ")" << std::endl;
                return 0;
            }
        }
        else if(type == "f") // face declaration
        {
            std::vector<uint> indices;
            for (int i = 0; i < 3; i++)
            {
                uint t;
                if(iss >> t)
                {
                    indices.push_back(t-1);
                }
                else
                {
                    std::cerr << "error" << std::endl;
                    return 0;
                }
            }
            float area = computeArea(
                        m_vertices[indices[0]]->position(),
                    m_vertices[indices[1]]->position(),
                    m_vertices[indices[2]]->position());
            Face *f = new Face(indices[0], indices[1], indices[2],area);
            m_faces.push_back(f);

            // sort indices list in ascending order
            bubbleSort(indices);

            // gen edge couple [0,1] [1,2] [0,2]
            // we only want edges with v1.id < v2.id
            std::vector<EdgeCouple> edgeCouples;
            edgeCouples.push_back(EdgeCouple(m_vertices[indices[0]], m_vertices[indices[1]]));
            edgeCouples.push_back(EdgeCouple(m_vertices[indices[1]], m_vertices[indices[2]]));
            edgeCouples.push_back(EdgeCouple(m_vertices[indices[0]], m_vertices[indices[2]]));

            // for every edge couples
            for (auto ec : edgeCouples)
            {
                // try to find if the edge already exists
                Edge *e = findEdge(edge_map, ec.v1, ec.v2);
                if (!e) // if it does't exists, create the edge and insert it in the map
                {
                    e = new Edge(ec.v1, ec.v2);
                    ec.v1->addEdge(e);
                    ec.v2->addEdge(e);

                    edge_map.insert({{e->v1()->id(), e->v2()->id()}, e});
                }

                // add the current face to the edge
                e->addFace(f->id());
            }
        }
        else if(type == "o") // new object (with name) :: UNSUPPORTED
        {
            // pass
        }
        else if(type == "#") // comment
        {
            // pass
        }
        else if(type == "s")
        {
            // pass
        }
        else
        {
            std::cerr << "OBJ flag not handled:\"" << type << "\" (at "<<numLine<<")"<< std::endl;
        }
    }
    std::cout << "Found " << m_vertices.size() << " vertices and " << m_faces.size() << " faces." << std::endl;

    file.close();

    // put all element of map into a vector
    for(auto it = edge_map.begin(); it != edge_map.end() ; ++it)
        m_edges.push_back(it->second);
    return 1;
}

int Mesh::importOFF(std::string filename)
{
    std::map<std::pair<int, int>, Edge *> edge_map;
    std::ifstream file;   // input file
    std::string line;     // current line
    std::stringstream ss; // stream to read content of line

    int numLine = 0;       // number of the current line
    int numVert, numFaces; // number of vertices/faces

    float x, y, z;

    // open file
    file.open(filename);
    if (!file.is_open())
    {
        std::cerr << "Unable to open file: " << filename << std::endl;
        return 0;
    }
    else
    {
        std::cout << "filename: " << filename << std::endl;
    }

    // detect OFF tag
    std::getline(file, line);
    numLine++;
    ss.str(line);
    if (!(ss.str() == "OFF"))
    {
        std::cerr << "file token is not OFF" << std::endl;
        return 0;
    }

    std::getline(file, line);
    numLine++;

    // get the number of vertices end faces indicated in the off file
    ss.str(line);
    ss >> numVert >> numFaces;

    // read vertices info
    for (int i = 0; i < numVert; i++)
    {
        std::getline(file, line);
        numLine++;

        std::istringstream iss(line);

        // tries to read at least 3 consecutive numbers as float
        if (iss >> x >> y >> z)
        {
            // add the new vertex to the mesh
            m_vertices.push_back(new Vertex({x, y, z}));
            m_raw_vertices.push_back({{x,y,z},0});
        }
        else
        {
            std::cout << "Wrong vertex format (at " << numLine << ")" << std::endl;
            return 0;
        }
    }

    // read face composition
    for (int i = 0; i < numFaces; i++)
    {
        std::getline(file, line);
        numLine++;

        uint nbv, id;
        std::istringstream iss(line);
        std::vector<uint> indices;

        // read number of vertices in face
        if (!(iss >> nbv))
        {
            std::cerr << "Unable to read vertex count of face! (at " << numLine << ")" << std::endl;
            return 0;
        }

        // read indices of vertices connected to the face
        // expects to read at least nbv indices
        for (uint i = 0; i < nbv; i++)
        {
            if (iss >> id)
            {
                indices.push_back(id);
            }
            else
            {
                std::cerr << "Unable to read vertex coordinate! (at " << numLine << ")" << std::endl;
                return 0;
            }
        }

        float area = computeArea(
                    m_vertices[indices[0]]->position(),
                m_vertices[indices[1]]->position(),
                m_vertices[indices[2]]->position());
        Face *f = new Face(indices[0], indices[1], indices[2],area);
        m_faces.push_back(f);

        // sort indices list in ascending order
        bubbleSort(indices);

        // gen edge couple [0,1] [1,2] [0,2]
        // we only want edges with v1.id < v2.id
        std::vector<EdgeCouple> edgeCouples;
        edgeCouples.push_back(EdgeCouple(m_vertices[indices[0]], m_vertices[indices[1]]));
        edgeCouples.push_back(EdgeCouple(m_vertices[indices[1]], m_vertices[indices[2]]));
        edgeCouples.push_back(EdgeCouple(m_vertices[indices[0]], m_vertices[indices[2]]));

        // for every edge couples
        for (auto ec : edgeCouples)
        {
            // try to find if the edge already exists
            Edge *e = findEdge(edge_map, ec.v1, ec.v2);
            if (!e) // if it does't exists, create the edge and insert it in the map
            {
                e = new Edge(ec.v1, ec.v2);
                ec.v1->addEdge(e);
                ec.v2->addEdge(e);

                edge_map.insert({{e->v1()->id(), e->v2()->id()}, e});
            }

            // add the current face to the edge
            e->addFace(f->id());
        }
    }
    // end
    std::cout << "Found " << m_vertices.size() << " vertices and " << m_faces.size() << " faces." << std::endl;

    file.close();

    // put all element of map into a vector
    for(auto it = edge_map.begin(); it != edge_map.end() ; ++it)
        m_edges.push_back(it->second);
    return 1;
}

int Mesh::simpleExport(std::string filename)
{
    std::ofstream file;   // output file

    //open file
    file.open(filename);
    if (!file.is_open())
    {
        std::cerr << "Unable to open file: " << filename << std::endl;
        return 0;
    }

    file << "o element" << std::endl;
    for(auto v: m_raw_vertices)
    {
        file << "v " << v.pos.x << " " << v.pos.y << " " << v.pos.z << std::endl;
    }
    file << "s off" << std::endl;
    for(auto f: m_faces)
    {
        file << "f " << f->A()+1 << " " << f->B()+1 << " " << f->C()+1 << std::endl;
    }

    file.close();
    return 1;
}

int Mesh::exportOBJ(std::string filename)
{
    std::ofstream file;   // output file

    //open file
    file.open(filename);
    if (!file.is_open())
    {
        std::cerr << "Unable to open file: " << filename << std::endl;
        return 0;
    }

    file << "o objet" << std::endl;
    unsigned int i = 1;
    for(Vertex *v : m_vertices)
    {
        file << "v " << v->position().x << " " << v->position().y << " " << v->position().z << std::endl;
        v->exportId(i);
        i++;
    }

    for(Edge *e : m_edges)
    {
        // line between the vertices
        file << "l " << e->v1()->exportId() << " " << e->v2()->exportId() << std::endl;
    }

    file.close();
    return 1;
}


void Mesh::exportMesh(std::string filename) {
    std::ofstream file;

    file.open(filename);
    if (!file.is_open())
    {
        std::cerr << "Unable to open file: " << filename << std::endl;
        return;
    }

    int elem_num = 0;
    ID i = 1;
    for (auto mesh : m_components)
    {
        file << "o element" << elem_num++ << std::endl;
        std::map<ID,SimpleVertex> map;
        std::vector<ID> indices_sorted;
        for(auto face: mesh)
        {
            auto it1 = map.find(face->A());
            if(it1 == map.end())
            {
                m_raw_vertices.at(face->A()).id = i;
                map.insert({face->A(),m_raw_vertices.at(face->A())});
                indices_sorted.push_back(face->A());
                i++;
            }
            auto it2 = map.find(face->B());
            if(it2 == map.end())
            {
                m_raw_vertices.at(face->B()).id = i;
                map.insert({face->B(),m_raw_vertices.at(face->B())});
                indices_sorted.push_back(face->B());
                i++;
            }
            auto it3 = map.find(face->C());
            if(it3 == map.end())
            {
                m_raw_vertices.at(face->C()).id = i;
                map.insert({face->C(),m_raw_vertices.at(face->C())});
                indices_sorted.push_back(face->C());
                i++;
            }
        }

        for(auto id : indices_sorted)
        {
            auto v = m_raw_vertices.at(id);
            file << "v " << v.pos.x << " " << v.pos.y << " " << v.pos.z << " " << std::endl;
        }
        file << "s off" << std::endl;

        for(auto face: mesh) {
            file << "f "
                << m_raw_vertices.at(face->A()).id << " "
                << m_raw_vertices.at(face->B()).id << " "
                << m_raw_vertices.at(face->C()).id << std::endl;
        }
    }
    file.close();
}


void Mesh::addFace(Face* f) {
    m_faces.push_back(f);
}

void Mesh::addVertex(Vertex* v) {
    m_vertices.push_back(v);
}


void Mesh::removeEdge(Edge *e)
{
    erase(m_edges, e);
}

void Mesh::removeVertex(Vertex *v)
{
    erase(m_vertices, v);
}

void Mesh::debug()
{
    std::cout << "Vertices (" << m_vertices.size() << ")" << std::endl;
    for(auto v: m_vertices)
    {
        std::cout << *v << std::endl;
    }

    std::cout << "Edges (" << m_edges.size() << ")" << std::endl;
    for(auto e: m_edges)
    {
        std::cout << *e << std::endl;
    }

    /*for(auto f: m_faces)
    {
        std::cout << *f << std::endl;
    }*/
}

int Mesh::allFacesInATL() 
{
    int compteur = 0;
    for(unsigned int i = 0; i < m_edges.size(); ++i)
    {
        compteur += m_edges.at(i)->ATL().size();
    }
    return compteur;
}

std::vector<Face*> Mesh::faces()
{
    return m_faces;
}
