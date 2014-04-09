////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//

#include <vector>

namespace octet {
  /// Scene containing a box with octet.
  class surfaceexplorer : public app {
    // scene for drawing box
    ref<visual_scene> app_scene;

    vec4 camera_position;
    vec3 camera_rotation;
    mat4t cameraToWorld;

    int  mouse_x, mouse_y;
    int prev_x, prev_y;

    node_selector nodeSelector;

    dynarray<vec3> *surfaceControlPoints;

    linear_surface linearSurface;
    color_shader cShader;
    surface_explorer_shader seShader;


    std::vector<mesh*> surfaceMesh;


  public:
    /// this is called when we construct the class before everything is initialised.
    surfaceexplorer(int argc, char **argv)
    : app(argc, argv)
    , camera_position(-1.0f, 0.0f, 10.0f, 0.0f)
    , camera_rotation(45.0f, 0.0f, 0.0f)
    , cameraToWorld(1.0f)
    , mouse_x(0), mouse_y(0)
    , prev_x(0), prev_y(0)
    , nodeSelector()
    , surfaceControlPoints(NULL)
    , linearSurface()
    , cShader()
    { }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();

      nodeSelector.init(4, 4, 2.0f, 2.0f);
      app_scene->add_child(&nodeSelector);

      surfaceControlPoints = nodeSelector.get_positions();

      linearSurface.init(surfaceControlPoints, nodeSelector.get_width(), nodeSelector.get_height(), 16, 16);
      cShader.init();
      seShader.init();

      dynarray<ref<mesh_instance>> *nodeMeshInstances = nodeSelector.getMeshInstances();

      for (auto i = nodeMeshInstances->begin(); i != nodeMeshInstances->end(); i++) {
        app_scene->add_mesh_instance(*i);
      }

    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      setCamera();
      keyboardInput();
      mouseMovement();

      linearSurface.update();

      // update matrices. assume 30 fps.
      app_scene->update(1.0f/30);

      // draw the scene
      app_scene->render((float)vx / vy);

      renderLinearSurfaceAsMesh();
    }

    void mouseMovement()
    {
      get_mouse_pos(mouse_x, mouse_y);

      bool isLeftClick = is_key_down(key_lmb);
      bool isAltPressed = is_key_down(key_alt);

      if (isLeftClick && !isAltPressed) {
        //Detect if cube is clicked
        vec3 rayOrigin;
        vec3 rayDirection;

        buildRay(rayOrigin, rayDirection);

        //printf("Ray origin(%.2f, %.2f, %.2f), direction(%.2f, %.2f, %.2f)\n", rayOrigin.x(), rayOrigin.y(), rayOrigin.z(), rayDirection.x(), rayDirection.y(), rayDirection.z());
        nodeSelector.updateState(true, mouse_y, &rayOrigin, &rayDirection);
      } else if (!isLeftClick && !isAltPressed) {
        nodeSelector.updateState(false, mouse_y);
      } else if (isLeftClick && isAltPressed) {

        if (prev_x < mouse_x)
        {
          camera_rotation[1] -= 4.0f;
          prev_x = mouse_x;

          if (camera_rotation[1] < 0.0f)
            camera_rotation[1] += 360.0f;

        }
        else if (prev_x > mouse_x)
        {
          camera_rotation[1] += 4.0f;
          prev_x = mouse_x;

          if (camera_rotation[1] >= 360.0f)
            camera_rotation[1] -= 360.0f;
        }

        if (prev_y < mouse_y)
        {
          camera_rotation[0] += 4.0f;
          prev_y = mouse_y;

          if (camera_rotation[0] > 90.0f)
            camera_rotation[0] = 90.0f;
        }
        else if (prev_y > mouse_y)
        {
          camera_rotation[0] -= 4.0f;
          prev_y = mouse_y;

          if (camera_rotation[0] < -90.0f)
            camera_rotation[0] = -90.0f;
        }
      }
    }

    void keyboardInput() 
    {
      static bool justPressed = false;
      vec4 direction(0.0f);
      
      if(is_key_down(key_space)){
        this->camera_position = vec4(-1.0f, 0.0f, 10.0f, 0.0f);
        this->camera_rotation = vec3(45.0f, 0.0f, 0.0f);
      }

      if(is_key_down('G')){
        nodeSelector.resetAllPositions();
      }

      if (!is_key_down(key_alt)) {
        if (is_key_down('W')) {
          direction[1] = -0.25f * (camera_position[2]/5.0f);
        } else if (is_key_down('S')) {
          direction[1] = 0.25f * (camera_position[2]/5.0f);
        }

        if (is_key_down('A')) {
          direction[0] = -0.25f * (camera_position[2]/5.0f);
        } else if (is_key_down('D')) {
          direction[0] = 0.25f * (camera_position[2]/5.0f);
        }

        mat4t directionMatrix(1.0f);
        directionMatrix.rotate(-camera_rotation[1], 0.0f, 0.0f, 1.0f);
        direction = direction * directionMatrix;

        camera_position[0] += direction[0];
        camera_position[1] += direction[1];

        if (is_key_down('Q')) {
          camera_position[2] -= 0.25f;
          if (camera_position[2] < 0.5f) camera_position[2] = 0.5f;
        } else if (is_key_down('E')) {
          camera_position[2] += 0.25f;
        }

        if (is_key_down('R')) {
          camera_position[3] += 0.25f * (camera_position[2]/5.0f);
        } else if (is_key_down('Y')) {
          camera_position[3] -= 0.25f * (camera_position[2]/5.0f);
        }
      } else {
        if (is_key_down('A')) {
          camera_rotation[1] += 5.0f;
          if (camera_rotation[1] >= 360.0f) camera_rotation[1] -= 360.0f;
        } else if (is_key_down('D')) {
          camera_rotation[1] -= 5.0f;
          if (camera_rotation[1] < 0.0f) camera_rotation[1] += 360.0f;
        }

        if (is_key_down('W')) {
          camera_rotation[0] -= 5.0f;
          if (camera_rotation[0] < -90.0f) camera_rotation[0] = -90.0f;
        } else if (is_key_down('S')) {
          camera_rotation[0] += 5.0f;
          if (camera_rotation[0] > 90.0f) camera_rotation[0] = 90.0f;
        }
        
      }
    }

    void setCamera() 
    {
      scene_node *camTransform = app_scene->get_camera_instance(0)->get_node();
      camTransform->loadIdentity();
      camTransform->translate(vec3(camera_position.x(), camera_position.w(), camera_position.y()));
      camTransform->rotate(camera_rotation[1], vec3(0.0f, 1.0f, 0.0f));
      camTransform->rotate(-camera_rotation[0], vec3(1.0f, 0.0f, 0.0f));
      camTransform->translate(vec3(0.0f, 0.0f, camera_position.z()));
    }

    void buildRay(vec3 &rayOrigin, vec3 &rayDirection) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);

      //ray r = app_scene->get_camera_instance(0)->get_ray(mouse_x, mouse_y);
      //nodeSelector.detectClick(r, cubeSelected);
      float near_plane = app_scene->get_camera_instance(0)->get_near_plane();

      rayOrigin = vec3(0.0f, 0.0f, 0.0f);
      //printf("Mouse (%d, %d), Viewport (%d, %d), uv (%g, %g).\n", mouse_x, mouse_y, vx, vy, (((float)mouse_x/vx)-0.5f)/20.0f, ((1.0f-(float)mouse_y/vy)-0.5f)/20.0f);
      rayDirection = vec3(((1.0f-(float)mouse_x/vx)-0.5f)/20.0f, (((float)mouse_y/vy)-0.5f)/20.0f, near_plane);
      rayDirection = rayDirection.normalize();

      mat4t directionMatrix(1.0f);
      directionMatrix.translate(camera_position.x(), camera_position.w(), camera_position.y());
      directionMatrix.rotate(camera_rotation[1], 0.0f, 1.0f, 0.0f);
      directionMatrix.rotate(-camera_rotation[0], 1.0f, 0.0f, 0.0f);
      directionMatrix.translate(0.0f, 0.0f, camera_position.z());

      rayOrigin = rayOrigin * directionMatrix;

      directionMatrix.loadIdentity();
      directionMatrix.rotate(camera_rotation[1], 0.0f, 1.0f, 0.0f);
      directionMatrix.rotate(-camera_rotation[0], 1.0f, 0.0f, 0.0f);

      rayDirection = rayDirection * directionMatrix;

    }

    void renderLinearSurfaceAsMesh(){

      surfaceMesh.clear();

      for(int i=0;i!=15;++i){
        for(int j=0;j!=15;++j){

        vec4 p1(linearSurface.getPointAt(i,j).x(),linearSurface.getPointAt(i,j).y(),linearSurface.getPointAt(i,j).z(),0.0f);
        vec4 p2(linearSurface.getPointAt(i+1,j).x(),linearSurface.getPointAt(i+1,j).y(),linearSurface.getPointAt(i+1,j).z(),0.0f);
        vec4 p3(linearSurface.getPointAt(i+1,j+1).x(),linearSurface.getPointAt(i+1,j+1).y(),linearSurface.getPointAt(i+1,j+1).z(),0.0f);
        vec4 p4(linearSurface.getPointAt(i,j+1).x(),linearSurface.getPointAt(i,j+1).y(),linearSurface.getPointAt(i,j+1).z(),0.0f);

        mesh* m = new mesh();
        m->make_plane(&p1,&p2,&p3,&p4);
        surfaceMesh.push_back(m);

        }
      }

     
      cShader.render(app_scene->get_camera_instance(0)->get_worldToProjection(), vec4(0.0f, 0.0f, 1.0f, 0.3f));

      for(int i=0;i!=surfaceMesh.size();++i){
        surfaceMesh[i]->render();
      }

    }

    void renderLinearSurfaceAsPoints() {
      dynarray <float> vertices;

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      glPointSize(5.0f);

      cShader.render(app_scene->get_camera_instance(0)->get_worldToProjection(), vec4(0.0f, 0.0f, 1.0f, 1.0f));

      linearSurface.get_points(vertices);

      glDisable(GL_DEPTH_TEST);
      glEnableVertexAttribArray(attribute_pos);
      glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 0, &vertices[0]);
      glDrawArrays(GL_POINTS, 0, vertices.size()/3);
      glDisableVertexAttribArray(attribute_pos);
      glEnable(GL_DEPTH_TEST);
    }
  };
}
