/** CODE NON TERMINE DE LA SEGMENTATION 

void Mesh::segmentation(float step = 1.0f) {
  // we get a copy of the mesh, the current mesh being the skeleton  
  Mesh initial;
  initial.importOFF("MeshTest/test1.OFF");

  m_bones = m_edges;
  m_edges = initial.edges();

  Plane *sweepPlane;
  std::vector<Edge*> candidates;
  Mesh *intersectionWithPlane = new Mesh();
  Mesh *crossSection = new Mesh();

  // some operations made only after a given number of iterations
  int countIterations = 0;
  // 5 last values of F (area or perimeter of the cross section)
  float F1 = 0.0f;
  float F2 = 0.0f;
  float F3 = 0.0f;
  float F4 = 0.0f;
  float F5 = 0.0f;

  // dérivations
  float d1 = 0.0f;
  float d2 = 0.0f;
  float previousD3 = 0.0f;
  float currentD3 = 0.0f;
  float nextD3 = 0.0f;

  // H : nb of simple polygons in the cross section
  int currentH = 0;
  int previousH = 0;
  float perimeter;

  int topology;

  std::vector<Mesh *> components;
  Mesh* currentComponent = new Mesh();

  for(unsigned int i=0; i<m_bones.size(); i++) {
    m_bones.at(i)->computeArea();
  }

  // order bones
  std::sort(m_bones.begin(), m_bones.end(), Edge::compEdgePtrArea);

  for(unsigned int i=0; i<m_bones.size(); i++) {
    countIterations = 0;

    // get the point V1 and the vector (V1,V2)
    sweepPlane = new Plane(m_bones.at(i)->v1()->pos(), m_bones.at(i)->v2()->pos(), m_bones.at(i)->getNormal(), step);

    while(sweepPlane->nextPoint()) {
      countIterations++;

      // TODO : search for less candidates to the intersection (= ATL of the bone ?)
      candidates = initial.edges();

      // intersections between sweepPlane and original mesh : edges

      for(unsigned int j=0; j<candidates.size(); j++) {
          int position1 = sweepPlane->relativePosition(candidates.at(j)->v1()->pos());
          int position2 = sweepPlane->relativePosition(candidates.at(j)->v2()->pos());

          if((position1 < 0 && position2 > 0) || (position1 > 0 && position2 < 0)) {
              intersectionWithPlane->addEdge(candidates.at(j));
          }
      }

      // intersections : faces
      for(unsigned int j=0; j<initial.faces().size(); j++) {
          for(unsigned int k=0; k<initial.faces().at(j)->edges().size(); k++) {

            // if one of the edges of the face is intersecting the cross section
            if(std::find(intersectionWithPlane->edges().begin(), intersectionWithPlane->edges().end(),
                initial.faces().at(j)->edges().at(k)) != intersectionWithPlane->edges().end()) {
                
                // the face is intersecting the cross section
                intersectionWithPlane->addFace(initial.faces().at(j));
                break;
            }

          }
      }

      intersectionWithPlane->constructFaces();
      crossSection->constructSection(intersectionWithPlane->faces());

      // perimeter : we suppose the polygons are simple
      for(unsigned int i=0; i<crossSection->edges().size(); i++) {
        perimeter += std::sqrt(crossSection->edges().at(i)->cost()); 
      }

      F1 = F2;
      F2 = F3;
      F3 = F4;
      F4 = F5;
      F5 = perimeter;

      previousD3 = currentD3;
      currentD3 = nextD3;

      if(countIterations >= 2) {
        // compute the first derivative (with 2 values)
        d1 = changeRate(F5, F4);
      }

      if(countIterations >= 3) {
        // compute the second derivative (change rate of 2 first derivatives = 3 values)
        d2 = changeRate(d1, changeRate(F4, F3));
      }

      if(countIterations >= 5) {
        // compute the third derivative (change rate of 2 second derivatives = 5 values)
        
        nextD3 = changeRate(
          d2,
          changeRate(
            changeRate(F3, F2),
            changeRate(F2, F1)
          )
        );
      }

      previousH = currentH;
      currentH = 0;

      // get H (number of simple polygons in the cross section)
      for(unsigned int j=0; j<crossSection->faces().size(); j++) {
        if(crossSection->faces().at(j)->isSimple()) {
          currentH++;
        }
      }

      if(currentH != previousH) {
        topology = 0;
      }
      else {
        topology = 1;
      }

      if(countIterations >= 7) {
        if(topology == 0 || (currentD3 == 0 && (previousD3*nextD3 < 0))) {
          // get the previous triangles and count them as a component
          
          // we look for the triangles belonging to the bones (ATL)
          for(unsigned int j=0; j<m_bones.at(i)->ATL().size(); j++) {
            // we look at its edges
            for(unsigned int k=0; k<m_bones.at(i)->ATL().at(j)->edges().size(); k++) {

              // if one vertex is under the sweep plane, it is part of our new component
              if(sweepPlane->relativePosition(m_bones.at(i)->ATL().at(j)->edges().at(k)->v1()->pos()) < 0) {
                currentComponent->addFace(m_bones.at(i)->ATL().at(j));
                // remove from ATL so it won't be considered in next components
                m_bones.at(i)->removeFaceATL(m_bones.at(i)->ATL().at(j));
                break;
              }
            }

          }

		  components.push_back(currentComponent);
          currentComponent = new Mesh();
        }
      }

      intersectionWithPlane->clear();
      crossSection->clear();
    }
  }

  exportMesh("result.obj", components);

  // check if exist
  delete sweepPlane;
  delete crossSection;
}

bool Edge::compEdgeArea(Edge *a, Edge *b) {
  return a->m_area < b->m_area;
}

// Construct faces using edges only
void Mesh::constructFaces() {
    std::vector<Edge*> belongsToFace;
    Edge* e1;
    Edge* e2;
    Vertex* sharedVertex = NULL;
    bool faceOver; // not working
    int count = m_vertices.size(); // avoid infinite loop (TODO)

    for(unsigned int i=0; i<m_edges.size() - 1; i++) {
      // ignore edges that already belongs to a face
      if(std::find(belongsToFace.begin(), belongsToFace.end(), m_edges[i]) != belongsToFace.end()) {
        continue;
      }

      for(unsigned int j=i+1; j<m_edges.size(); j++) {
        if(std::find(belongsToFace.begin(), belongsToFace.end(), m_edges[j]) != belongsToFace.end()) {
          continue;
        }

        e1 = m_edges[i];
        e2 = m_edges[j];
        sharedVertex = NULL;

        // they share a vertex
        if(e1->v1() == e2->v1() || e1->v1() == e2->v2()) {
          sharedVertex = e1->v1();
        }
        if(e1->v2() == e2->v1() || e1->v2() == e2->v2()) {
          sharedVertex = e1->v2();
        }

        if(sharedVertex != NULL) {
          Face* face = new Face();
          face->edges().push_back(e1);
          belongsToFace.push_back(e1);

          Vertex* currentVertex = sharedVertex;

          // get to the next vertex
          for(unsigned int k=0; k<m_edges.size(); k++) {
            if(std::find(belongsToFace.begin(), belongsToFace.end(), m_edges[k]) != belongsToFace.end()) {
              continue;
            }
            bool foundNext = false;

            if(m_edges[k]->v1() == currentVertex) {
              currentVertex = m_edges[k]->v2();
              foundNext = true;
            } else if(m_edges[k]->v2() == currentVertex) {
              currentVertex = m_edges[k]->v1();
              foundNext = true;
            }

            if(foundNext) {
              // add the edge to the face
              face->edges().push_back(m_edges[k]);
              belongsToFace.push_back(m_edges[k]);
              break;
            }
          }

          faceOver = false;

          // we look at the entire face
          while((currentVertex != sharedVertex) && (!faceOver) && count > 0) {
            count--;

            // get to the next vertex
            for(unsigned int k=0; k<m_edges.size(); k++) {
              if(std::find(belongsToFace.begin(), belongsToFace.end(), m_edges[k]) != belongsToFace.end()) {
                continue;
              }
              bool foundNext = false;

              if(m_edges[k]->v1() == currentVertex) {
                currentVertex = m_edges[k]->v2();
                foundNext = true;
              } else if(m_edges[k]->v2() == currentVertex) {
                currentVertex = m_edges[k]->v1();
                foundNext = true;
              }

              if(foundNext) {
                // add the edge to the face
                face->edges().push_back(m_edges[k]);
                belongsToFace.push_back(m_edges[k]);
                break;
              }

              // we didn't find the next vertex
              if(k == m_edges.size()-1) {
                faceOver = true;
              }
            }
          }

          // fix
          if(face->edges().size() == 3) {
            m_faces.push_back(face);
          }
        }
      }
    }
}

// Construct a cross section (intersection between mesh and plane)
// we simplify it by taking the center of each face (instead of using the plane)

void Mesh::constructSection(std::vector<Face*> faces) {
  Vertex* v;
  bool foundSharedEdge;

  // for each face, get its center
  for(unsigned int i=0; i<faces.size(); i++) {
    faces.at(i)->computeCenter();
    m_vertices.push_back(faces.at(i)->center());
  }

  for(unsigned int i=0; i<faces.size(); i++) {
    for(unsigned int j=i+1; j<faces.size(); j++) {

      // construct an edge between 2 centers if their faces shared an edge
      foundSharedEdge = false;

      for(unsigned int k=0; k<faces.at(i)->edges().size() && !foundSharedEdge; k++) {
        for(unsigned int l=0; l<faces.at(j)->edges().size() && !foundSharedEdge; l++) {

          if(faces.at(i)->edges().at(k)->id() == faces.at(j)->edges().at(l)->id()) {

            if((faces.at(i)->center() == nullptr) || faces.at(j)->center() == nullptr) {
              // ?
            } else {
              Edge* e = new Edge(faces.at(i)->center(), faces.at(j)->center());
              m_edges.push_back(e);
              foundSharedEdge = true;
            }

          }
        }
      }

    }
  }

  // we get a list of edges, lets construct faces
  constructFaces();
}

// test if a polygon is simple
bool Face::isSimple()
{
    bool isSimple = true;

    // test each couple of edges
    for (unsigned int i = 0; (i < m_edges.size() - 1) && isSimple; i++)
    {
        for (unsigned int j = i + 1; (j < m_edges.size()) && isSimple; j++)
        {
            if (m_edges[i]->intersectWith(m_edges[j]))
            {
                isSimple = false;
            }
        }
    }
    return isSimple;
}

glm::vec3 Edge::getNormal() {
    return glm::normalize(m_v2->position() - m_v1->position());
}

void Face::computeCenter() {
    std::vector<Vertex*> vertices;
    float x, y, z;

    if(m_edges.size() != 3) {
        std::cout << "Not a triangle" << std::endl;
        return;
    }

    // get the 3 vertices
    for(unsigned int i=0; i<3; i++) {
        // add the vertices we didn't find
        if(std::find(vertices.begin(), vertices.end(), m_edges.at(i)->v1()) == vertices.end()) {
            vertices.push_back(m_edges.at(i)->v1());
        }
        if(std::find(vertices.begin(), vertices.end(), m_edges.at(i)->v2()) == vertices.end()) {
            vertices.push_back(m_edges.at(i)->v2());
        }
    }

    x = vertices.at(0)->pos().x + vertices.at(1)->pos().x + vertices.at(2)->pos().x;
    x = x / 3;

    y = vertices.at(0)->pos().y + vertices.at(1)->pos().y + vertices.at(2)->pos().y;
    y = y / 3;

    z = vertices.at(0)->pos().z + vertices.at(1)->pos().z + vertices.at(2)->pos().z;
    z = z / 3;

    m_center = new Vertex({x, y, z});
}



**/
