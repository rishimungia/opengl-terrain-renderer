#ifndef CONTROLS_HPP
#define CONTROLS_HPP

void computeMatricesFromInputs();
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();
glm::mat4 getSkyboxViewMatrix();
glm::vec3 getCameraPosition();
#endif