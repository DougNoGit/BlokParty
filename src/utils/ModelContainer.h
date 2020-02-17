#ifndef MODEL_CONTAINER_H
#define MODEL_CONTAINER_H
#include <glm/glm.hpp>
#include <iostream>
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include "tiny_gltf.h"
using namespace tinygltf;

class Program;

struct SimpleVertex {
    glm::vec3 pos;
    glm::vec4 color;
};

class ModelContainer
{
public:
	ModelContainer(const std::string filename);
	virtual ~ModelContainer();
	//void draw(const std::shared_ptr<Program> prog) const;
	//glm::vec3 min;
	//glm::vec3 max;
	std::vector<SimpleVertex> verts;
private:	
	//void init();
	//void measure();
	void loadModel();
	void createModelContainer();
	Model model;
	
	std::vector<unsigned int> eleBuf;
	std::vector<float> posBuf;
	std::vector<float> norBuf;
	std::vector<float> texBuf;
	unsigned eleBufID;
	unsigned posBufID;
	unsigned norBufID;
	unsigned texBufID;
    unsigned vaoID;
};

#endif //MODEL_CONTAINER_H