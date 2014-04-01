namespace octet {
  class node_selector : public scene_node {
    
    material *red;
    material *green;
    mesh_box *pointMesh;

    dynarray <ref<scene_node>> cubeNodes;
    dynarray <ref<mesh_instance>> meshInstances;

    unsigned int width;
    unsigned int height;

  public:
    node_selector()
    : cubeNodes()
    , meshInstances()
    {
      red = NULL;
      green = NULL;
      pointMesh = NULL;
    }

    ~node_selector() {
      if (red) delete red;
      if (green) delete green;
      if (pointMesh) delete pointMesh;
    }

    void init(int width_, int height_, float widthSize, float lengthSize) {
      
      float halfSelectorWidth = width_*widthSize/2.0f;
      float halfSelectorLength = height_*lengthSize/2.0f;

      width = width_;
      height = height_;
      cubeNodes.resize(width*height);
      meshInstances.resize(width*height);

      red = new material(vec4(1.0f, 0.4f, 0, 1));
      green = new material(vec4(0.4f, 1.0f, 0, 1));
      pointMesh = new mesh_box(vec3(1));

      for (int j = 0; j != height; j++) {
        for (int i = 0 ; i != width; i++) {
          cubeNodes[j*width+i] = new scene_node();
          scene_node &cn = *(cubeNodes[j*width+i]);
          cn.translate(vec3(i*widthSize-halfSelectorWidth, 0, j*lengthSize-halfSelectorLength));
          
          meshInstances[j*width+i] = new mesh_instance(cubeNodes[j*width+i], pointMesh, red);
        }
      }
    }

    dynarray<ref<mesh_instance> > *getMeshInstances() {
      return &meshInstances;
    }
  };
}