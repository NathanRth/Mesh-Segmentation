#include "mesh.hpp"
#include "edge.hpp"
#include "vertex.hpp"
#include <algorithm>
#include <iostream>

void Mesh::skeletonization()
{
    bool stop = false;

    // while not all remaining edges ar not bones (or virtuals), keep going
    while(!stop)
    {
        std::size_t i = 0;

        // sort all edges from shortest to longest
        std::sort(m_edges.begin(), m_edges.end(), Edge::compEdgeCost);

        // while we are not dealing with a MESH type edge
        while(i < m_edges.size())
        {
            Edge *e = m_edges[i];
            if(e) // if the edge e is not null
            {
                if(e->type() == Edge::MESH)
                {
                    if(e->faces().size() > 0) // if an edge is connect to at least one face, we can collapse it
                    {
                        //debug();
                        dissolveEdge(e);
                        //std::cout << std::endl;
                        if(e->v1()->locked() == false)
                        {
                            removeVertex(e->v1());
                            delete e->v1();
                        }
                        if(e->v2()->locked() == false)
                        {
                            removeVertex(e->v2());
                            delete e->v2();
                        }

                        // remove the edge
                        removeEdge(e);
                        delete e;
                        break;

                    }
                    else // if an edge is not connected to any face, lock it
                    {
                        //std::cout << *e <<"to bone" << std::endl;
                        e->type(Edge::BONE);
                        e->v1()->locked(true);
                        e->v2()->locked(true);
                    }
                }
                else
                {
                    // is a bone or a virtual edge
                }
                //debug();
                i++;
            }
            else
            {
                std::cerr << "the edge does not extist" << std::endl;
            }
        }
        if(i >= m_edges.size()-1)
        {
            stop = true;
        }

        std::cout << "\r" << static_cast<float>(m_vertices.size())<<std::flush;
        //std::cout << m_vertices.size() << std::endl;
        //debug();
    }
    //std::cout << std::endl;
}

void Mesh::dissolveEdge(Edge *edge)
{
    Vertex *mean = edge->getMeanPosition();
    m_vertices.push_back(mean);
    auto connectedEdges = edge->getConnectedEdges();
    //std::cout << "concat : " << *edge << std::endl;
    for(auto e: connectedEdges)
    {
        if(e->type() == Edge::MESH && e != edge)
        {
            // on déplace le vertex de e connécté à edge

            if(e->v1() == edge->v1() || e->v1() == edge->v2())
            {
                e->v1(mean);
            }
            else if(e->v2() == edge->v1() || e->v2() == edge->v2())
            {
                e->v2(mean);
            }
            mean->addEdge(e);
        }
        else if(e->type() == Edge::BONE && e != edge)
        {
            // on créer une virtual entre mean et le vertex connecté

            Edge *virtualEdge = new Edge();
            virtualEdge->type(Edge::VIRTUAL);
            if(e->v1() == edge->v1() || e->v1() == edge->v2())
            {
                virtualEdge->v1(e->v1());
            }
            else if(e->v2() == edge->v1() || e->v2() == edge->v2())
            {
                virtualEdge->v1(e->v2());
            }
            virtualEdge->v2(mean);
            virtualEdge->v1()->locked(true);
            //virtualEdge->v2()->locked(true);
            m_edges.push_back(virtualEdge);
            mean->addEdge(virtualEdge); // THIS IS NEW
        } else {
            e->v2(mean);
            mean->addEdge(e);
        }
    }

    // cleaning

    //std::cout << "cleaning" << std::endl;

    std::map<std::pair<std::size_t, std::size_t>, Edge *> map;


    // no face in common handling
    std::vector<FIC> fic_list;
    FIC no_fic;
    bool gotSpecialCase = false;
    std::size_t i = 0;
    for(auto it_e2 = mean->edges().begin(); it_e2 != mean->edges().end() && i < mean->edges().size(); it_e2++) // iterator
    {
        Edge *e2 = *it_e2;

        auto it_e1 = map.find({(*it_e2)->v1()->id(),(*it_e2)->v2()->id()});

        // if two edges have same vertices and are not the same edge
        if(it_e1 != map.end())
        {
            Edge *e1 = (*it_e1).second;
            ID f = 0;
            // si elles ont une face en commun
            if(faceInCommon(e1,e2,&f))
            {
                fic_list.push_back({e1,e2,true,f});
            }
            else
            {
                //std::cout << "detect special case:" << *e1 << "  " << *e2 << std::endl;
                gotSpecialCase = true;
                no_fic = {e1,e2,false,f};
                // le cas qui nous interresse
            }
        }
        else
        {
            map.insert({{e2->v1()->id(),e2->v2()->id()},e2});
        }
        ++i;
    }
    map.clear();

    if(gotSpecialCase)
    {
        for(auto fic_e: fic_list)
        {
            ID fic1, fic2;
            if((faceInCommon(no_fic.e1, fic_e.e1,&fic1) || faceInCommon(no_fic.e1, fic_e.e2,&fic1)) &&
                    (faceInCommon(no_fic.e2, fic_e.e1,&fic2) || faceInCommon(no_fic.e2, fic_e.e2,&fic2)))
            {
                no_fic.e1->addFace(fic2);
                fic_e.e1->removeFace(fic2);
                fic_e.e2->removeFace(fic2);
                break;
            }
            else
            {
                // pas le bon couple
            }
        }
    }

    //std::cout << "mean" << mean->id() << " has:" <<std::endl;
    //for(auto e: mean->edges())
    //{
        //std::cout << *e << std::endl;
    //}


    // final cleaning
    bool usedATLofCollapsed = false;
    i=0;
    for(auto it_e2 = mean->edges().begin(); it_e2 != mean->edges().end() && i < mean->edges().size(); it_e2++) // iterator
    {
        Edge *e2 = *it_e2;

        auto it_e1 = map.find({(*it_e2)->v1()->id(),(*it_e2)->v2()->id()});

        // if two edges have same vertices and are not the same edge
        if(it_e1 != map.end())
        {
            Edge *e1 = (*it_e1).second;

            //std::cout << "got same edges: " << *e1 << " " << *e2 << std::endl;

            //ID f = 0;
            // si elles ont une face en commun
            std::vector<ID> fic = facesInCommon(e1,e2);
            if(fic.size()>0)
            {
                for(ID f: fic)
                {
                    //std::cout << "face in common " << f << std::endl;
                    // on la retire de la liste des faces de e1
                    e1->removeFace(f);
                    // on la retire de la liste des faces de e2
                    // on la retire car pour la suite on peut tester s'il n'en reste qu'une seule
                    e2->removeFace(f);
                    // on la met dans la liste ATL de e1
                    e1->addFaceATL(f);
                }
            }
            else
            {
                std::cout << "#### no face in common ####" << std::endl;
                /*std::cout << "got same edges: " << *e1 << " " << *e2 << std::endl;
                std::cout << "concat : " << *edge << std::endl;
                std::cout << "mean" << mean->id() << " has:" <<std::endl;
                for(auto e: mean->edges())
                {
                    std::cout << *e << std::endl;
                }
                //std::cout << "#### no face in common ####" << std::endl;*/
            }

            // on met la liste des ATL de e2 dans e1
            if(e2->ATL().size() > 0)
            {
                for(ID f_id : e2->ATL())
                {
                    //std::cout << "add face to ATL: "<< f_id << std::endl;
                    e1->addFaceATL(f_id);
                }
            }

            // on met la liste des ATL de edge dans e1
            if(edge->ATL().size() > 0 && usedATLofCollapsed == false)
            {
                for(ID f_id : edge->ATL())
                {
                    //std::cout << "add face to ATL: "<< f_id << std::endl;
                    e1->addFaceATL(f_id);
                }
                usedATLofCollapsed = true;
            }

            // si e2 est adjacent à une autre face
            for(unsigned int j = 0; j < e2->faces().size(); ++j)
            {
                // on la met dans e1
                ID tf = e2->faces().at(j);
                e1->addFace(tf);
            }

            // on supprime e2 de la liste des edges de l'extremité qui n'est pas mean
            // on ne supprime pas de mean car on est en train d'itérer sur ses edges
            // le probleme sera résolu avec le transfère des edges par le map
            if(e2->v1() != mean)
            {
                e2->v1()->removeEdge(e2);
            }
            else
            {
                e2->v2()->removeEdge(e2);
            }

            // on supprime e2
            removeEdge(e2);
        }
        else
        {
            map.insert({{e2->v1()->id(),e2->v2()->id()},e2});
        }
    }

    mean->edges().clear();
    //std::cout << "mean (v"<< mean->id() << ") now has:"<<std::endl;
    for(auto e : map)
    {
        mean->edges().push_back(e.second);
    //    std::cout << *(e.second) << std::endl;
    }
}
