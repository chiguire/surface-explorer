namespace octet {
  
  enum NodeSelectorState {
    Idle,
    Selected
  };
  
  class node_selector : public scene_node {
    
    mesh_sphere *pointMesh;

    bool isLeftButtonPrevClicked;

    dynarray <ref<scene_node>> cubeNodes;
    dynarray <ref<mesh_instance>> meshInstances;
    dynarray <ref<material>> red;
    dynarray <ref<material>> green;
    dynarray <vec3> meshPositions;

    mesh_instance *meshSelected;
    ivec3 meshSelectedIndex;
    float meshSelectedHeight;
    float mouse_y_start;
    mat4t nodeToParentStart;

    unsigned int width;
    unsigned int height;

    NodeSelectorState state;
  public:
    

    node_selector()
    : pointMesh(NULL)
    , isLeftButtonPrevClicked(false)
    , cubeNodes()
    , meshInstances()
    , red()
    , green()
    , meshPositions()
    , meshSelected(NULL)
    , meshSelectedIndex(0)
    , meshSelectedHeight(0.0f)
    , mouse_y_start(0.0f)
    , nodeToParentStart(1.0f)
    , width(0)
    , height(0)
    , state(Idle)
    {
    }

    ~node_selector() {
      if (pointMesh) delete pointMesh;
    }

    void init(int width_, int height_, float widthSize, float lengthSize) {
      
      float halfSelectorWidth = width_*widthSize/2.0f;
      float halfSelectorLength = height_*lengthSize/2.0f;

      width = width_;
      height = height_;
      cubeNodes.resize(width*height);
      meshInstances.resize(width*height);

      pointMesh = new mesh_sphere(vec3(0,0,0), 0.25f, 1);

      for (int j = 0; j != height; j++) {
        for (int i = 0 ; i != width; i++) {
          vec3 pos(i*widthSize-halfSelectorWidth, 0, j*lengthSize-halfSelectorLength);
          cubeNodes[j*width+i] = new scene_node();
          scene_node &cn = *(cubeNodes[j*width+i]);
          cn.translate(pos);
          
          red.push_back(new material(vec4(1.0f, 0.4f, 0, 1)));
          green.push_back(new material(vec4(0.4f, 1.0f, 0, 1)));
          meshPositions.push_back(pos);

          meshInstances[j*width+i] = new mesh_instance(cubeNodes[j*width+i], pointMesh, red[red.size()-1]);
        }
      }
    }

    dynarray<ref<mesh_instance> > *getMeshInstances() {
      return &meshInstances;
    }

    void updateState(bool leftButtonClicked, float mouse_y, const vec3 *origin = NULL, const vec3 *direction = NULL) {
      if (leftButtonClicked && !isLeftButtonPrevClicked) {
        dynarray<scene_node *> candidateNodes;
        dynarray<ivec3> candidatePositions;
      
        for (int j = 0; j != height; j++) {
          for (int i = 0; i != width; i++) {
            if (rayIntersectsSphere(meshInstances[j*width+i]->get_node(), 0.5f, *origin, *direction)) {
              candidateNodes.push_back(cubeNodes[j*width+i]);
              candidatePositions.push_back(ivec3(i, j, 0));
            }
          }
        }

        if (candidateNodes.size() == 0) {
          meshSelected = NULL;
          state = Idle;
          meshSelectedIndex[0] = meshSelectedIndex[1] = -1;
          //printf("No candidates\n");
        } else if (candidateNodes.size() == 1) {
          state = Selected;
          //printf("One candidate\n");
          meshSelected = meshInstances[candidatePositions[0][1]*width+candidatePositions[0][0]];
          meshSelectedIndex[0] = candidatePositions[0][0];
          meshSelectedIndex[1] = candidatePositions[0][1];
        } else {
          // Test whoever is closest
          state = Selected;
          //printf("%d candidates\n", candidateNodes.size());
          float minDistance = 1000000.0f;

          for (int i = 0; i != candidateNodes.size(); i++) {
            float distance = (*origin - (vec4(0.0f, 0.0f, 0.0f, 1.0f)*candidateNodes[i]->get_nodeToParent()).xyz()).length();
            if (distance < minDistance) {
              minDistance = distance;
              meshSelected = meshInstances[candidatePositions[i][1]*width+candidatePositions[i][0]];
              meshSelectedIndex[0] = candidatePositions[i][0];
              meshSelectedIndex[1] = candidatePositions[i][1];
            }
          }
        }

        if (meshSelected) {
          printf("Selected node (%d, %d)\n", meshSelectedIndex[0], meshSelectedIndex[1]);
          meshSelected->set_material(green[meshSelectedIndex[1]*width+meshSelectedIndex[0]]);
          nodeToParentStart = meshSelected->get_node()->get_nodeToParent();
          meshSelectedHeight = (vec4(0.0f, 0.0f, 0.0f, 1.0f)*nodeToParentStart).y();
          mouse_y_start = mouse_y;
        } else {
          printf("Not selected node\n");
        }
        isLeftButtonPrevClicked = true;
      } else if (leftButtonClicked && isLeftButtonPrevClicked) {
        //drag if meshSelected, else do nothing
        if (meshSelected) {
          float mouse_y_diff = (mouse_y_start - mouse_y)*0.015f;
          mat4t nodeToParentDiff = nodeToParentStart;
          nodeToParentDiff.translate(0.0f, mouse_y_diff, 0.0f);
          meshSelected->get_node()->access_nodeToParent() = nodeToParentDiff;

          meshPositions[meshSelectedIndex[1]*width+meshSelectedIndex[0]] = (vec4(0.0f, 0.0f, 0.0f, 1.0f) * nodeToParentDiff).xyz();
          //printf("Drag distance: %g\n", mouse_y_diff);
        }
      } else if (!leftButtonClicked) {
        isLeftButtonPrevClicked = false;
        if (meshSelected) {
          meshSelected->set_material(red[meshSelectedIndex[1]*width+meshSelectedIndex[0]]);
        }
        meshSelectedIndex[0] = meshSelectedIndex[1] = -1;
        meshSelected = NULL;
        state = Idle; 
      }
    }

    bool rayIntersectsSphere(scene_node *node, float radius, const vec3 &origin, const vec3 &direction) {
      vec4 p(0.0f, 0.0f, 0.0f, 1.0f);
      p = p * node->get_nodeToParent();
      vec3 temp = origin-p.xyz();
      float a = direction.dot(direction);
      float b = 2.0f*(temp.dot(direction));
      float c = temp.dot(temp) - radius*radius;

      //printf("node in (%g, %g, %g), a=%g, b=%g, c=%g, (b*b-4*a*c)=%g >= 0\n", p[0], p[1], p[2], a, b, c, (b*b - 4*a*c));
      return (b*b - 4*a*c) > 0;
    }

    unsigned int get_width() { return width; }
    unsigned int get_height() { return height; }
    dynarray<vec3> *get_positions() { return &meshPositions; }
  };
}