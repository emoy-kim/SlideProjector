#include "Renderer.h"

RendererGL::RendererGL() : 
   Window( nullptr ), FrameWidth( 1920 ), FrameHeight( 1080 ), IsVideo( true ), ClickedPoint( -1, -1 ),
   MainCamera( std::make_unique<CameraGL>() ),
   Projector( std::make_unique<CameraGL>( 
      glm::vec3{ 40.0f, 30.0f, 20.0f },
      glm::vec3{ 0.0f, 0.0f, 0.0f },
      glm::vec3{ 0.0f, 1.0f, 0.0f },
      30.0f, 10.0f, 60.0f
   ) ),
   ObjectShader( std::make_unique<ShaderGL>() ), ProjectorPyramidObject( std::make_unique<ObjectGL>() ),
   ScreenObject( std::make_unique<ObjectGL>() ), WallObject( std::make_unique<ObjectGL>() ),
   Lights( std::make_unique<LightGL>() )
{
   Renderer = this;

   initialize();
   printOpenGLInformation();
}

RendererGL::~RendererGL()
{
   glfwTerminate();
}

void RendererGL::printOpenGLInformation()
{
   std::cout << "****************************************************************\n";
   std::cout << " - GLFW version supported: " << glfwGetVersionString() << "\n";
   std::cout << " - OpenGL renderer: " << glGetString( GL_RENDERER ) << "\n";
   std::cout << " - OpenGL version supported: " << glGetString( GL_VERSION ) << "\n";
   std::cout << " - OpenGL shader version supported: " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << "\n";
   std::cout << "****************************************************************\n\n";
}

void RendererGL::initialize()
{
   if (!glfwInit()) {
      std::cout << "Cannot Initialize OpenGL...\n";
      return;
   }
   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 6 );
   glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );
   glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

   Window = glfwCreateWindow( FrameWidth, FrameHeight, "Main Camera", nullptr, nullptr );
   glfwMakeContextCurrent( Window );

   if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      std::cout << "Failed to initialize GLAD" << std::endl;
      return;
   }
   
   registerCallbacks();
   
   glEnable( GL_DEPTH_TEST );
   glClearColor( 0.1f, 0.1f, 0.1f, 1.0f );

   MainCamera->updateWindowSize( FrameWidth, FrameHeight );

   const std::string shader_directory_path = std::string(CMAKE_SOURCE_DIR) + "/shaders";
   ObjectShader->setShader(
      std::string(shader_directory_path + "/SlideProjector.vert").c_str(),
      std::string(shader_directory_path + "/SlideProjector.frag").c_str()
   );
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
   glfwSetWindowShouldClose( window, GLFW_TRUE );
}

void RendererGL::cleanupWrapper(GLFWwindow* window)
{
   Renderer->cleanup( window );
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
            prepareSlide();
            std::cout << "Replay Video!\n";
         }
         break;
      case GLFW_KEY_L:
         Lights->toggleLightSwitch();
         std::cout << "Light Turned " << (Lights->isLightOn() ? "On!\n" : "Off!\n");
         break;
      case GLFW_KEY_ENTER:
         IsVideo = !IsVideo;
         prepareSlide();
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
   CameraGL* camera = glfwGetKey( window, GLFW_KEY_LEFT_CONTROL ) == GLFW_PRESS ? MainCamera.get() : Projector.get();
   if (camera->getMovingState()) {
      const auto x = static_cast<int>(round( xpos ));
      const auto y = static_cast<int>(round( ypos ));
      const int dx = x - ClickedPoint.x;
      const int dy = y - ClickedPoint.y;
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
         ClickedPoint.x = static_cast<int>(round( x ));
         ClickedPoint.y = static_cast<int>(round( y ));
      }

      if (glfwGetKey( window, GLFW_KEY_LEFT_CONTROL ) == GLFW_PRESS) {
         MainCamera->setMovingState( moving_state );
      }
      else Projector->setMovingState( moving_state );
   }
}

void RendererGL::mouseWrapper(GLFWwindow* window, int button, int action, int mods)
{
   Renderer->mouse( window, button, action, mods );
}

void RendererGL::mousewheel(GLFWwindow* window, double xoffset, double yoffset) const
{
   if (yoffset >= 0.0) MainCamera->zoomIn();
   else MainCamera->zoomOut();
}

void RendererGL::mousewheelWrapper(GLFWwindow* window, double xoffset, double yoffset)
{
   Renderer->mousewheel( window, xoffset, yoffset );
}

void RendererGL::reshape(GLFWwindow* window, int width, int height) const
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

void RendererGL::setLights() const
{  
   const glm::vec4 light_position(Projector->getCameraPosition(), 1.0f);
   const glm::vec4 ambient_color(0.3f, 0.3f, 0.3f, 1.0f);
   const glm::vec4 diffuse_color(0.7f, 0.7f, 0.7f, 1.0f);
   const glm::vec4 specular_color(0.9f, 0.9f, 0.9f, 1.0f);
   Lights->addLight( light_position, ambient_color, diffuse_color, specular_color );
}

void RendererGL::setWallObject() const
{
   constexpr float size = 30.0f;
   std::vector<glm::vec3> wall_vertices;
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

   std::vector<glm::vec3> wall_normals;
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
      
   WallObject->setObject( GL_TRIANGLES, wall_vertices, wall_normals );
   WallObject->setDiffuseReflectionColor( { 0.52f, 0.12f, 0.15f, 1.0f } );
}

void RendererGL::prepareSlide()
{
   static const std::string sample_directory_path = std::string(CMAKE_SOURCE_DIR) + "/samples";
   static const std::string image_path = sample_directory_path + "/image.jpg";
   static const std::string video_path = sample_directory_path + "/video.mp4";

   if (!IsVideo) {
      Slide = cv::imread( image_path );
      Projector->updateWindowSize( Slide.cols / 100, Slide.rows / 100 );
   }
   else {
      if (Video.isOpened()) Video.release();

      Video.open( video_path );
      if (!Video.isOpened()) {
         std::cout << "Cannot Read Video File...\n";
         return;
      }
      Video >> Slide;
      Projector->updateWindowSize( Slide.cols / 100, Slide.rows / 100 );
   }
   ScreenObject->reallocateTexture( Slide, 0 );
}

void RendererGL::setScreenObject()
{
   prepareSlide();
   const float near_plane = Projector->getNearPlane();
   const float half_width = static_cast<float>(Projector->getWidth()) * 0.5f;
   const float half_height = static_cast<float>(Projector->getHeight()) * 0.5f;

   std::vector<glm::vec3> screen_vertices;
   screen_vertices.emplace_back( half_width, -half_height, -near_plane );
   screen_vertices.emplace_back( half_width, half_height, -near_plane );
   screen_vertices.emplace_back( -half_width, half_height, -near_plane );
   
   screen_vertices.emplace_back( half_width, -half_height, -near_plane );
   screen_vertices.emplace_back( -half_width, half_height, -near_plane );
   screen_vertices.emplace_back( -half_width, -half_height, -near_plane );

   std::vector<glm::vec2> screen_textures;
   screen_textures.emplace_back( 1.0f, 0.0f );
   screen_textures.emplace_back( 1.0f, 1.0f );
   screen_textures.emplace_back( 0.0f, 1.0f );
   
   screen_textures.emplace_back( 1.0f, 0.0f );
   screen_textures.emplace_back( 0.0f, 1.0f );
   screen_textures.emplace_back( 0.0f, 0.0f );
      
   ScreenObject->setObject( GL_TRIANGLES, screen_vertices, screen_textures, Slide );
}

void RendererGL::setProjectorPyramidObject() const
{
   const float near_plane = Projector->getNearPlane();
   const float far_plane = Projector->getFarPlane();
   const float half_width = static_cast<float>(Projector->getWidth()) * 0.5f * far_plane / near_plane;
   const float half_height = static_cast<float>(Projector->getHeight()) * 0.5f * far_plane / near_plane;

   std::vector<glm::vec3> pyramid_vertices;
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

   ProjectorPyramidObject->setObject( GL_LINES, pyramid_vertices );
   ProjectorPyramidObject->setDiffuseReflectionColor( { 1.0f, 1.0f, 0.0f, 1.0f } );
}

void RendererGL::drawWallObject() const
{
   glUseProgram( ObjectShader->getShaderProgram() );

   ObjectShader->transferBasicTransformationUniforms( glm::mat4(1.0f), MainCamera.get(), true );
   const glm::mat4& view = Projector->getViewMatrix();
   const glm::mat4& projection = Projector->getProjectionMatrix();
   glUniformMatrix4fv( ObjectShader->getLocation( "ProjectorViewMatrix" ), 1, GL_FALSE, &view[0][0] );
   glUniformMatrix4fv( ObjectShader->getLocation( "ProjectorProjectionMatrix" ), 1, GL_FALSE, &projection[0][0] );
   glUniform1i( ObjectShader->getLocation( "WhichObject" ), WALL );

   WallObject->transferUniformsToShader( ObjectShader.get() );
   Lights->transferUniformsToShader( ObjectShader.get() );

   glBindTextureUnit( 0, ScreenObject->getTextureID( 0 ) );
   glBindVertexArray( WallObject->getVAO() );
   glDrawArrays( WallObject->getDrawMode(), 0, WallObject->getVertexNum() );
}

void RendererGL::drawScreenObject() const
{
   glUseProgram( ObjectShader->getShaderProgram() );

   const glm::mat4 to_world = inverse( Projector->getViewMatrix() );
   ObjectShader->transferBasicTransformationUniforms( to_world, MainCamera.get(), true );
   glUniform1i( ObjectShader->getLocation( "WhichObject" ), SCREEN );

   ScreenObject->transferUniformsToShader( ObjectShader.get() );

   glBindTextureUnit( 0, ScreenObject->getTextureID( 0 ) );
   glBindVertexArray( ScreenObject->getVAO() );
   glDrawArrays( ScreenObject->getDrawMode(), 0, ScreenObject->getVertexNum() );
}

void RendererGL::drawProjectorObject() const
{
   glUseProgram( ObjectShader->getShaderProgram() );
   glLineWidth( 3.0f );

   const glm::mat4 to_world = inverse( Projector->getViewMatrix() );
   ObjectShader->transferBasicTransformationUniforms( to_world, MainCamera.get() );
   glUniform1i( ObjectShader->getLocation( "WhichObject" ), PROJECTOR );

   ProjectorPyramidObject->transferUniformsToShader( ObjectShader.get() );

   glBindVertexArray( ProjectorPyramidObject->getVAO() );
   glDrawArrays( ProjectorPyramidObject->getDrawMode(), 0, ProjectorPyramidObject->getVertexNum() );
   glLineWidth( 1.0f );
}

void RendererGL::render() const
{
   glClear( OPENGL_COLOR_BUFFER_BIT | OPENGL_DEPTH_BUFFER_BIT );

   drawWallObject();
   drawScreenObject();
   drawProjectorObject();

   glBindVertexArray( 0 );
   glUseProgram( 0 );
}

void RendererGL::setNextSlide()
{
   if (IsVideo) {
      Video >> Slide;
      if (Slide.empty()) return;

      ScreenObject->updateTexture( Slide, 0 );
   }
}

void RendererGL::play()
{
   if (glfwWindowShouldClose( Window )) initialize();

   setLights();
   setWallObject();
   setScreenObject();
   setProjectorPyramidObject();
   ObjectShader->addUniformLocation( "WhichObject" );
   ObjectShader->addUniformLocation( "ProjectorViewMatrix" );
   ObjectShader->addUniformLocation( "ProjectorProjectionMatrix" );
   ObjectShader->setUniformLocations( Lights->getTotalLightNum() );

   while (!glfwWindowShouldClose( Window )) {
      render();
      setNextSlide();

      glfwSwapBuffers( Window );
      glfwPollEvents();
   }
   glfwDestroyWindow( Window );
}