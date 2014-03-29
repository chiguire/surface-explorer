////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
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

  public:
    /// this is called when we construct the class before everything is initialised.
    surfaceexplorer(int argc, char **argv)
    : app(argc, argv)
    , camera_position(0.0f, 0.0f, 10.0f, 0.0f)
    , camera_rotation(45.0f, 0.0f, 0.0f)
    , cameraToWorld(1.0f)
    { }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();

      material *red = new material(vec4(0.4f, 1.0f, 0, 1));
      mesh_box *box = new mesh_box(vec3(1));
      scene_node *node = new scene_node();
      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, box, red));
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      mouseMovement();
      setCamera();

      // update matrices. assume 30 fps.
      app_scene->update(1.0f/30);

      // draw the scene
      app_scene->render((float)vx / vy);

    }

    void mouseMovement()
    {
      get_mouse_pos(mouse_x, mouse_y);

      bool is_mouse_down = is_key_down(key_lmb) && is_key_down(key_alt);
      if (!is_mouse_down) return;

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

    void setCamera() 
    {
      scene_node *camTransform = app_scene->get_camera_instance(0)->get_node();
      camTransform->loadIdentity();
      camTransform->translate(vec3(camera_position.x(), camera_position.w(), camera_position.y()));
      camTransform->rotate(camera_rotation[1], vec3(0.0f, 1.0f, 0.0f));
      camTransform->rotate(-camera_rotation[0], vec3(1.0f, 0.0f, 0.0f));
      camTransform->translate(vec3(0.0f, 0.0f, camera_position.z()));
    }
  };
}
