/*
 * Author: Emoy Kim
 * E-mail: emoy.kim_AT_gmail.com
 * 
 * This code is a free software; it can be freely used, changed and redistributed.
 * If you use any version of the code, please reference the code.
 * 
 */

#pragma once

#include "_Common.h"

#include "Camera.h"
#include "Object.h"

class RendererGL
{
   static RendererGL* Renderer;
   GLFWwindow* Window;

   bool IsVideo;
   vec2 ClickedPoint;
   Mat Frame;
   VideoCapture Video;

   CameraGL* MainCamera;
   CameraGL* Projector;
   ShaderGL ObjectShader;
   
   LightGL Lights;
   ObjectGL ProjectorPyramidObject, ScreenObject, WallObject;


   void registerCallbacks() const;
   void initialize();
   void printOpenGLInformation() const;

   void error(int error, const char* description) const;
   void cleanup(GLFWwindow* window);
   void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods);
   void cursor(GLFWwindow* window, double xpos, double ypos);
   void mouse(GLFWwindow* window, int button, int action, int mods);
   void mousewheel(GLFWwindow* window, double xoffset, double yoffset);
   void reshape(GLFWwindow* window, int width, int height);
   static void errorWrapper(int error, const char* description);
   static void cleanupWrapper(GLFWwindow* window);
   static void keyboardWrapper(GLFWwindow* window, int key, int scancode, int action, int mods);
   static void cursorWrapper(GLFWwindow* window, double xpos, double ypos);
   static void mouseWrapper(GLFWwindow* window, int button, int action, int mods);
   static void mousewheelWrapper(GLFWwindow* window, double xoffset, double yoffset);
   static void reshapeWrapper(GLFWwindow* window, int width, int height);

   void prepareFrame();
   void setNextFrame();

   void setLights();
   void setProjectorPyramidObject();
   void setScreenObject();
   void setWallObject();

   void drawWallObject();
   void drawScreenObject();
   void drawProjectorObject();
   void render();


public:
   RendererGL(const RendererGL&) = delete;
   RendererGL(const RendererGL&&) = delete;
   RendererGL& operator=(const RendererGL&) = delete;
   RendererGL& operator=(const RendererGL&&) = delete;


   RendererGL();
   ~RendererGL();

   void play();
};