
#include "ModelContainer.h"


ModelContainer::ModelContainer(const std::string filename) 
{
  tinygltf::TinyGLTF loader;
  std::string err;
  std::string warn;

  bool res = loader.LoadASCIIFromFile(&model, &err, &warn, filename);

  if (!warn.empty()) 
    std::cout << "WARN: " << warn << std::endl;
  

  if (!err.empty()) 
    std::cout << "ERR: " << err << std::endl;
  

  if (!res) {
    std::cout << "Failed to load glTF: " << filename << std::endl;
    exit(EXIT_FAILURE);
  }
  else 
    std::cout << "Loaded glTF: " << filename << std::endl;

  createModelContainer();

}

ModelContainer::~ModelContainer() {}

void ModelContainer::createModelContainer()
{
  for(auto mesh : model.meshes) 
  {
    for(auto prim : mesh.primitives)
    {
      const tinygltf::Accessor& accessor = model.accessors[prim.attributes["POSITION"]];
      const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
      const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
      // bufferView byteoffset + accessor byteoffset tells you where the actual position data is within the buffer. From there
      // you should already know how the data needs to be interpreted.
      const float* positions = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
      // From here, you choose what you wish to do with this position data. In this case, we  will display it out.
      //for (size_t i = 0; i < accessor.count; ++i) {
      //            
      //            SimpleVertex x = SimpleVertex {
      //            glm::vec3(positions[i * 3 + 0],
      //                      positions[i * 3 + 1],
      //                      positions[i * 3 + 2]), 
      //            glm::vec4(1,0,0,1) };
      //            verts.push_back(x);
      //            std::cout << "x: " << x.pos.x << " Y : " << x.pos.y << " Z: " << x.pos.z << std::endl;
      //}

      const tinygltf::Accessor& accessor1 = model.accessors[prim.indices];
      const tinygltf::BufferView& bufferView1 = model.bufferViews[accessor1.bufferView];
      const tinygltf::Buffer& buffer1 = model.buffers[bufferView1.buffer];
      const unsigned short* indices = reinterpret_cast<const unsigned short*>(&buffer1.data[bufferView1.byteOffset + accessor1.byteOffset]);
      for(size_t i = 0; i < accessor1.count; i++) {
        verts.push_back(SimpleVertex {
                  glm::vec3(positions[(int)indices[i] * 3],
                            positions[(int)indices[i] * 3+ 1],
                            positions[(int)indices[i] * 3 + 2]), 
                  glm::vec4(1,0,0,1) });
        //std::cout << "Index: " << (int)indices[i] << std::endl;
      }
    }
  }
}