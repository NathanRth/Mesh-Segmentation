// Code utilisé pour la première version de la segmentation (non terminée)

// #include "plane.hpp"
// #include <iostream>

// Plane::Plane(glm::vec3 v1, glm::vec3 v2, glm::vec3 n, float step) {
// 	m_point = v1;
// 	m_lastPoint = v2;
// 	m_vecteurNormale = n*step;
// }

// glm::vec3 Plane::point() {
// 	return m_point;
// }

// glm::vec3 Plane::vecNorm(){
// 	return m_vecteurNormale;
// }

// void Plane::point(glm::vec3 p){
// 	m_point = p;
// }

// void Plane::vecNorm(glm::vec3 v){
// 	m_vecteurNormale = v;
// }

// bool Plane::nextPoint(){
// 	m_point += m_vecteurNormale;

// 	// while last point is below the plane, return true
// 	return (relativePosition(m_lastPoint) < 0 );
// }

// int Plane::relativePosition(glm::vec3 v){
// 	float dotProduct = glm::dot(m_point-v, m_vecteurNormale);
// 	if(dotProduct < -0.001)
// 		return -1;
// 	else if(dotProduct > 0.001)
// 		return 1;
// 	else
// 		return 0;
// }
