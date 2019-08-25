#include "Renderer.h"

RendererGL* RendererGL::Renderer = nullptr;
RendererGL::RendererGL() : Window( nullptr ), IsVideo( false )
{
   Renderer = this;

   initialize();
   printOpenGLInformation();
}

RendererGL::~RendererGL()
{
   glfwTerminate();
}

void RendererGL::printOpenGLInformation() const
{
   cout << "****************************************************************" << endl;
   cout << " - GLFW version supported: " << glfwGetVersionString() << endl;
   cout << " - GLEW version supported: " << glewGetString( GLEW_VERSION ) << endl;
   cout << " - OpenGL renderer: " << glGetString( GL_RENDERER ) << endl;
   cout << " - OpenGL version supported: " << glGetString( GL_VERSION ) << endl;
   cout << " - OpenGL shader version supported: " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << endl  ;
   cout << "****************************************************************" << endl << endl;
}

void RendererGL::initialize()
{
   if (!glfwInit()) {
      cout << "Cannot Initialize OpenGL..." << endl;
      return;
   }
   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 5 );
   glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );
   glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

   const int width = 1920;
   const int height = 1080;
   Window = glfwCreateWindow( width, height, "Main Camera", nullptr, nullptr );
   glfwMakeContextCurrent( Window );
   glewInit();
   
   registerCallbacks();
   
   glEnable( GL_DEPTH_TEST );
   glClearColor( 0.1f, 0.1f, 0.1f, 1.0f );

   MainCamera = new CameraGL();
   MainCamera->updateWindowSize( width, height );
   ObjectShader.setShader(
      "Shaders/VertexShaderForSlideProjector.glsl",
      "Shaders/FragmentShaderForSlideProjector.glsl"
   );

   Projector = new CameraGL(
      { 40.0f, 30.0f, 20.0f },
      { 0.0f, 0.0f, 0.0f },
      { 0.0f, 1.0f, 0.0f },
      30.0f, 10.0f, 60.0f
   );
   prepareFrame();
}

void RendererGL::error(int error, const char* description) const
{
   puts( description );
}

void RendererGL::errorWrapper(int error, const char* description)
{
   Renderer->error( error, description );
}

void RendererGL::cleanup(GLFWwindow* window)
{
   glDeleteProgram( ObjectShader.ShaderProgram );
   glDeleteVertexArrays( 1, &ProjectorPyramidObject.ObjVAO );
   glDeleteBuffers( 1, &ProjectorPyramidObject.ObjVBO );
   glDeleteVertexArrays( 1, &ScreenObject.ObjVAO );
   glDeleteBuffers( 1, &ScreenObject.ObjVBO );
   glDeleteVertexArrays( 1, &WallObject.ObjVAO );
   glDeleteBuffers( 1, &WallObject.ObjVBO );
   glfwSetWindowShouldClose( window, GLFW_TRUE );
}

void RendererGL::cleanupWrapper(GLFWwindow* window)
{
   Renderer->cleanup( window );
}

void RendererGL::prepareFrame()
{
   if (!IsVideo) {
      Frame = imread( "Samples/image.jpg" );
      Projector->updateWindowSize( Frame.cols / 100, Frame.rows / 100 );
   }
   else {
      if (Video.isOpened()) Video.release();

      Video.open( "Samples/video.mp4" );
      if (!Video.isOpened()) {
         cout << "Cannot Read Video File..." << endl;
         return;
      }
      Video >> Frame;
      Projector->updateWindowSize( Frame.cols / 100, Frame.rows / 100 );
   }
   ScreenObject.replaceTexture( Frame );
}

void RendererGL::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   if (action != GLFW_PRESS) return;

   switch (key) {
      case GLFW_KEY_UP:
         MainCamera->moveForward();
         break;
      case GLFW_KEY_DOWN:
         MainCamera->moveBackward();
         break;
      case GLFW_KEY_LEFT:
         MainCamera->moveLeft();
         break;
      case GLFW_KEY_RIGHT:
         MainCamera->moveRight();
         break;
      case GLFW_KEY_W:
         MainCamera->moveUp();
         break;
      case GLFW_KEY_S:
         MainCamera->moveDown();
         break;
      case GLFW_KEY_I:
         MainCamera->resetCamera();
         Projector->resetCamera();
         break;
      case GLFW_KEY_R:
         if (IsVideo) {
            prepareFrame();
            cout << "Replay Video!" << endl;
         }
         break;
      case GLFW_KEY_L:
         Lights.toggleLightSwitch();
         cout << "Light Turned " << (Lights.isLightOn() ? "On!" : "Off!") << endl;
         break;
      case GLFW_KEY_ENTER:
         IsVideo = !IsVideo;
         prepareFrame();
         break;
      case GLFW_KEY_Q:
      case GLFW_KEY_ESCAPE:
         cleanupWrapper( window );
         break;
      default:
         return;
   }
}

void RendererGL::keyboardWrapper(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   Renderer->keyboard( window, key, scancode, action, mods );
}

void RendererGL::cursor(GLFWwindow* window, double xpos, double ypos)
{
   CameraGL* camera = glfwGetKey( window, GLFW_KEY_LEFT_CONTROL ) == GLFW_PRESS ? Projector : MainCamera;
   if (camera->getMovingState()) {
      const auto x = static_cast<float>(round( xpos ));
      const auto y = static_cast<float>(round( ypos ));
      const int dx = static_cast<int>(x - ClickedPoint.x);
      const int dy = static_cast<int>(y - ClickedPoint.y);
      camera->moveForward( -dy );
      camera->rotateAroundWorldY( -dx );

      if (glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_RIGHT ) == GLFW_PRESS) {
         camera->pitch( -dy );
      }

      ClickedPoint.x = x;
      ClickedPoint.y = y;
   }
}

void RendererGL::cursorWrapper(GLFWwindow* window, double xpos, double ypos)
{
   Renderer->cursor( window, xpos, ypos );
}

void RendererGL::mouse(GLFWwindow* window, int button, int action, int mods)
{
   if (button == GLFW_MOUSE_BUTTON_LEFT) {
      const bool moving_state = action == GLFW_PRESS;
      if (moving_state) {
         double x, y;
         glfwGetCursorPos( window, &x, &y );
         ClickedPoint.x = static_cast<float>(round( x ));
         ClickedPoint.y = static_cast<float>(round( y ));
      }

      if (glfwGetKey( window, GLFW_KEY_LEFT_CONTROL ) == GLFW_PRESS) {
         Projector->setMovingState( moving_state );
      }
      else MainCamera->setMovingState( moving_state );
   }
}

void RendererGL::mouseWrapper(GLFWwindow* window, int button, int action, int mods)
{
   Renderer->mouse( window, button, action, mods );
}

void RendererGL::mousewheel(GLFWwindow* window, double xoffset, double yoffset)
{
   if (yoffset >= 0.0) MainCamera->zoomIn();
   else MainCamera->zoomOut();
}

void RendererGL::mousewheelWrapper(GLFWwindow* window, double xoffset, double yoffset)
{
   Renderer->mousewheel( window, xoffset, yoffset );
}

void RendererGL::reshape(GLFWwindow* window, int width, int height)
{
   MainCamera->updateWindowSize( width, height );
   glViewport( 0, 0, width, height );
}

void RendererGL::reshapeWrapper(GLFWwindow* window, int width, int height)
{
   Renderer->reshape( window, width, height );
}

void RendererGL::registerCallbacks() const
{
   glfwSetErrorCallback( errorWrapper );
   glfwSetWindowCloseCallback( Window, cleanupWrapper );
   glfwSetKeyCallback( Window, keyboardWrapper );
   glfwSetCursorPosCallback( Window, cursorWrapper );
   glfwSetMouseButtonCallback( Window, mouseWrapper );
   glfwSetScrollCallback( Window, mousewheelWrapper );
   glfwSetFramebufferSizeCallback( Window, reshapeWrapper );
}

void RendererGL::setLights()
{  
   const vec4 light_position(Projector->CamPos, 1.0f);
   const vec4 ambient_color(0.3f, 0.3f, 0.3f, 1.0f);
   const vec4 diffuse_color(0.7f, 0.7f, 0.7f, 1.0f);
   const vec4 specular_color(0.9f, 0.9f, 0.9f, 1.0f);
   Lights.addLight( light_position, ambient_color, diffuse_color, specular_color );
}

void RendererGL::setProjectorPyramidObject()
{
   if (ProjectorPyramidObject.ObjVAO != 0) {
      glDeleteVertexArrays( 1, &ProjectorPyramidObject.ObjVAO );
      glDeleteBuffers( 1, &ProjectorPyramidObject.ObjVBO );
   }

   const float near_plane = Projector->getCameraNearPlane();
   const float far_plane = Projector->getCameraFarPlane();
   const float half_width = static_cast<float>(Projector->getCameraWidth()) * 0.5f * far_plane / near_plane;
   const float half_height = static_cast<float>(Projector->getCameraHeight()) * 0.5f * far_plane / near_plane;

   vector<vec3> pyramid_vertices;
   pyramid_vertices.emplace_back( 0.0f, 0.0f, 0.0f );
   pyramid_vertices.emplace_back( -half_width, half_height, -far_plane );

   pyramid_vertices.emplace_back( 0.0f, 0.0f, 0.0f );
   pyramid_vertices.emplace_back( half_width, half_height, -far_plane );

   pyramid_vertices.emplace_back( 0.0f, 0.0f, 0.0f );
   pyramid_vertices.emplace_back( half_width, -half_height, -far_plane );

   pyramid_vertices.emplace_back( 0.0f, 0.0f, 0.0f );
   pyramid_vertices.emplace_back( -half_width, -half_height, -far_plane );

   pyramid_vertices.emplace_back( -half_width, half_height, -far_plane );
   pyramid_vertices.emplace_back( half_width, half_height, -far_plane );

   pyramid_vertices.emplace_back( half_width, half_height, -far_plane );
   pyramid_vertices.emplace_back( half_width, -half_height, -far_plane );

   pyramid_vertices.emplace_back( half_width, -half_height, -far_plane );
   pyramid_vertices.emplace_back( -half_width, -half_height, -far_plane );

   pyramid_vertices.emplace_back( -half_width, -half_height, -far_plane );
   pyramid_vertices.emplace_back( -half_width, half_height, -far_plane );

   ProjectorPyramidObject.setObject( GL_LINES, pyramid_vertices );
   ProjectorPyramidObject.setDiffuseReflectionColor( { 1.0f, 1.0f, 0.0f, 1.0f } );
}

void RendererGL::setScreenObject()
{
   if (ScreenObject.ObjVAO != 0) {
      glDeleteVertexArrays( 1, &ScreenObject.ObjVAO );
      glDeleteBuffers( 1, &ScreenObject.ObjVBO );
   }

   const float near_plane = Projector->getCameraNearPlane();
   const float half_width = static_cast<float>(Projector->getCameraWidth()) * 0.5f;
   const float half_height = static_cast<float>(Projector->getCameraHeight()) * 0.5f;

   vector<vec3> screen_vertices;
   screen_vertices.emplace_back( half_width, -half_height, -near_plane );
   screen_vertices.emplace_back( half_width, half_height, -near_plane );
   screen_vertices.emplace_back( -half_width, half_height, -near_plane );
   
   screen_vertices.emplace_back( half_width, -half_height, -near_plane );
   screen_vertices.emplace_back( -half_width, half_height, -near_plane );
   screen_vertices.emplace_back( -half_width, -half_height, -near_plane );

   vector<vec2> screen_textures;
   screen_textures.emplace_back( 1.0f, 0.0f );
   screen_textures.emplace_back( 1.0f, 1.0f );
   screen_textures.emplace_back( 0.0f, 1.0f );
   
   screen_textures.emplace_back( 1.0f, 0.0f );
   screen_textures.emplace_back( 0.0f, 1.0f );
   screen_textures.emplace_back( 0.0f, 0.0f );
      
   ScreenObject.setObject( GL_TRIANGLES, screen_vertices, screen_textures, Frame );
}

void RendererGL::setWallObject()
{
   if (WallObject.ObjVAO != 0) {
      glDeleteVertexArrays( 1, &WallObject.ObjVAO );
      glDeleteBuffers( 1, &WallObject.ObjVBO );
   }

   const float size = 30.0f;
   vector<vec3> wall_vertices;
   wall_vertices.emplace_back( size, 0.0f, 0.0f );
   wall_vertices.emplace_back( size, size, 0.0f );
   wall_vertices.emplace_back( 0.0f, size, 0.0f );
   
   wall_vertices.emplace_back( size, 0.0f, 0.0f );
   wall_vertices.emplace_back( 0.0f, size, 0.0f );
   wall_vertices.emplace_back( 0.0f, 0.0f, 0.0f );

   wall_vertices.emplace_back( size, 0.0f, size );
   wall_vertices.emplace_back( size, 0.0f, 0.0f );
   wall_vertices.emplace_back( 0.0f, 0.0f, 0.0f );
   
   wall_vertices.emplace_back( size, 0.0f, size );
   wall_vertices.emplace_back( 0.0f, 0.0f, 0.0f );
   wall_vertices.emplace_back( 0.0f, 0.0f, size );

   wall_vertices.emplace_back( 0.0f, 0.0f, 0.0f );
   wall_vertices.emplace_back( 0.0f, size, 0.0f );
   wall_vertices.emplace_back( 0.0f, size, size );
   
   wall_vertices.emplace_back( 0.0f, 0.0f, 0.0f );
   wall_vertices.emplace_back( 0.0f, size, size );
   wall_vertices.emplace_back( 0.0f, 0.0f, size );

   vector<vec3> wall_normals;
   wall_normals.emplace_back( 0.0f, 0.0f, 1.0f );
   wall_normals.emplace_back( 0.0f, 0.0f, 1.0f );
   wall_normals.emplace_back( 0.0f, 0.0f, 1.0f );
   
   wall_normals.emplace_back( 0.0f, 0.0f, 1.0f );
   wall_normals.emplace_back( 0.0f, 0.0f, 1.0f );
   wall_normals.emplace_back( 0.0f, 0.0f, 1.0f );

   wall_normals.emplace_back( 0.0f, 1.0f, 0.0f );
   wall_normals.emplace_back( 0.0f, 1.0f, 0.0f );
   wall_normals.emplace_back( 0.0f, 1.0f, 0.0f );
   
   wall_normals.emplace_back( 0.0f, 1.0f, 0.0f );
   wall_normals.emplace_back( 0.0f, 1.0f, 0.0f );
   wall_normals.emplace_back( 0.0f, 1.0f, 0.0f );
  
   wall_normals.emplace_back( 1.0f, 0.0f, 0.0f );
   wall_normals.emplace_back( 1.0f, 0.0f, 0.0f );
   wall_normals.emplace_back( 1.0f, 0.0f, 0.0f );
   
   wall_normals.emplace_back( 1.0f, 0.0f, 0.0f );
   wall_normals.emplace_back( 1.0f, 0.0f, 0.0f );
   wall_normals.emplace_back( 1.0f, 0.0f, 0.0f );
      
   WallObject.setObject( GL_TRIANGLES, wall_vertices, wall_normals );
   WallObject.setDiffuseReflectionColor( { 0.52f, 0.12f, 0.15f, 1.0f } );
}

void RendererGL::drawWallObject()
{
   glUseProgram( ObjectShader.ShaderProgram );

   const mat4 to_world = mat4(1.0f);
   const mat4 model_view_projection = MainCamera->ProjectionMatrix * MainCamera->ViewMatrix * to_world;
   glUniformMatrix4fv( ObjectShader.Location.World, 1, GL_FALSE, &to_world[0][0] );
   glUniformMatrix4fv( ObjectShader.Location.View, 1, GL_FALSE, &MainCamera->ViewMatrix[0][0] );
   glUniformMatrix4fv( ObjectShader.Location.Projection, 1, GL_FALSE, &MainCamera->ProjectionMatrix[0][0] );
   glUniformMatrix4fv( ObjectShader.Location.ModelViewProjection, 1, GL_FALSE, &model_view_projection[0][0] );
   glUniformMatrix4fv( ObjectShader.CustomLocations["ProjectorViewMatrix"], 1, GL_FALSE, &Projector->ViewMatrix[0][0] );
   glUniformMatrix4fv( ObjectShader.CustomLocations["ProjectorProjectionMatrix"], 1, GL_FALSE, &Projector->ProjectionMatrix[0][0] );
   glUniform1i( ObjectShader.CustomLocations["WhichObject"], 0 );

   WallObject.transferUniformsToShader( ObjectShader );
   Lights.transferUniformsToShader( ObjectShader );

   glBindTextureUnit( ObjectShader.Location.TextureUnit, ScreenObject.TextureID );
   glBindVertexArray( WallObject.ObjVAO );
   glDrawArrays( WallObject.DrawMode, 0, WallObject.VerticesCount );
}

void RendererGL::drawScreenObject()
{
   glUseProgram( ObjectShader.ShaderProgram );

   const mat4 to_world = inverse( Projector->ViewMatrix );
   const mat4 model_view_projection = MainCamera->ProjectionMatrix * MainCamera->ViewMatrix * to_world;
   glUniformMatrix4fv( ObjectShader.Location.World, 1, GL_FALSE, &to_world[0][0] );
   glUniformMatrix4fv( ObjectShader.Location.View, 1, GL_FALSE, &MainCamera->ViewMatrix[0][0] );
   glUniformMatrix4fv( ObjectShader.Location.Projection, 1, GL_FALSE, &MainCamera->ProjectionMatrix[0][0] );
   glUniformMatrix4fv( ObjectShader.Location.ModelViewProjection, 1, GL_FALSE, &model_view_projection[0][0] );
   glUniform1i( ObjectShader.CustomLocations["WhichObject"], 1 );

   ScreenObject.transferUniformsToShader( ObjectShader );

   glBindTextureUnit( ObjectShader.Location.TextureUnit, ScreenObject.TextureID );
   glBindVertexArray( ScreenObject.ObjVAO );
   glDrawArrays( ScreenObject.DrawMode, 0, ScreenObject.VerticesCount );
}

void RendererGL::drawProjectorObject()
{
   glUseProgram( ObjectShader.ShaderProgram );
   glLineWidth( 3.0f );

   const mat4 to_world = inverse( Projector->ViewMatrix );
   const mat4 model_view_projection = MainCamera->ProjectionMatrix * MainCamera->ViewMatrix * to_world;
   glUniformMatrix4fv( ObjectShader.Location.World, 1, GL_FALSE, &to_world[0][0] );
   glUniformMatrix4fv( ObjectShader.Location.View, 1, GL_FALSE, &MainCamera->ViewMatrix[0][0] );
   glUniformMatrix4fv( ObjectShader.Location.Projection, 1, GL_FALSE, &MainCamera->ProjectionMatrix[0][0] );
   glUniformMatrix4fv( ObjectShader.Location.ModelViewProjection, 1, GL_FALSE, &model_view_projection[0][0] );
   glUniform1i( ObjectShader.CustomLocations["WhichObject"], 2 );

   ProjectorPyramidObject.transferUniformsToShader( ObjectShader );

   glBindVertexArray( ProjectorPyramidObject.ObjVAO );
   glDrawArrays( ProjectorPyramidObject.DrawMode, 0, ProjectorPyramidObject.VerticesCount );
   glLineWidth( 1.0f );
}

void RendererGL::render()
{
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   drawWallObject();
   drawScreenObject();
   drawProjectorObject();

   glBindVertexArray( 0 );
   glUseProgram( 0 );
}

void RendererGL::setNextFrame()
{
   if (IsVideo) {
      Video >> Frame;
      if (Frame.empty()) return;
         
      ScreenObject.updateTexture( Frame );
   }
}

void RendererGL::play()
{
   if (glfwWindowShouldClose( Window )) initialize();

   setLights();
   setProjectorPyramidObject();
   setScreenObject();
   setWallObject();
   ObjectShader.addUniformLocation( ObjectShader.ShaderProgram, "WhichObject" );
   ObjectShader.addUniformLocation( ObjectShader.ShaderProgram, "ProjectorViewMatrix" );
   ObjectShader.addUniformLocation( ObjectShader.ShaderProgram, "ProjectorProjectionMatrix" );

   ObjectShader.setUniformLocations( Lights.TotalLightNum );

   while (!glfwWindowShouldClose( Window )) {
      render();
      setNextFrame();

      glfwSwapBuffers( Window );
      glfwPollEvents();
   }
   glfwDestroyWindow( Window );
}