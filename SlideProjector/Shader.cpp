#include "Shader.h"

ShaderGL::ShaderGL() : ShaderProgram( 0 )
{
}

void ShaderGL::readShaderFile(string& shader_contents, const char* shader_path) const
{
   ifstream file(shader_path, ios::in);
   if (!file.is_open()) {
      cout << "Cannot Open Shader File: " << shader_path << endl;
      return;
   }

   string line;
   while (!file.eof()) {
      getline( file, line );
      shader_contents.append( line + "\n" );
   }
   file.close();
}

bool ShaderGL::checkCompileErrors(const GLuint& vertex_shader, const GLuint& fragment_shader)
{
   GLint compiled[2] = { 0, 0 };
   glGetShaderiv( vertex_shader, GL_COMPILE_STATUS, &compiled[0] );
   glGetShaderiv( fragment_shader, GL_COMPILE_STATUS, &compiled[1] );

   GLuint shaders[2] = { vertex_shader, fragment_shader };
   for (int i = 0; i <= 1; ++i) {
      if (compiled[i] == GL_FALSE) {
         GLint max_length = 0;
         glGetShaderiv( shaders[i], GL_INFO_LOG_LENGTH, &max_length );

         if (i == 0) cout << " === Vertex Shader Log ===" << endl;
         else cout << " === Fragment Shader Log ===" << endl;

         vector<GLchar> error_log(max_length);
         glGetShaderInfoLog( shaders[i], max_length, &max_length, &error_log[0] );
         for (const auto& c : error_log) {
            cout << c;
         }
         glDeleteShader( shaders[i] );
      }
   }
   return compiled[0] && compiled[1];
}

void ShaderGL::setShader(const char* vertex_shader_path, const char* fragment_shader_path)
{
   string vertex_contents, fragment_contents;
   readShaderFile( vertex_contents, vertex_shader_path );
   readShaderFile( fragment_contents, fragment_shader_path );

   const GLuint vertex_shader = glCreateShader( GL_VERTEX_SHADER );
   const GLuint fragment_shader = glCreateShader( GL_FRAGMENT_SHADER );
   const char* vertex_source = vertex_contents.c_str();
   const char* fragment_source = fragment_contents.c_str();
   glShaderSource( vertex_shader, 1, &vertex_source, nullptr );
   glShaderSource( fragment_shader, 1, &fragment_source, nullptr );
   glCompileShader( vertex_shader );
   glCompileShader( fragment_shader );
   if (!checkCompileErrors( vertex_shader, fragment_shader )) return;

   ShaderProgram = glCreateProgram();
   glAttachShader( ShaderProgram, vertex_shader );
   glAttachShader( ShaderProgram, fragment_shader );
   glLinkProgram( ShaderProgram );
   
   glDeleteShader( vertex_shader );
   glDeleteShader( fragment_shader );
}

void ShaderGL::setUniformLocations(const uint& light_num)
{
   Location.World = glGetUniformLocation( ShaderProgram, "WorldMatrix" );
   Location.View = glGetUniformLocation( ShaderProgram, "ViewMatrix" );
   Location.Projection = glGetUniformLocation( ShaderProgram, "ProjectionMatrix" );
   Location.ModelViewProjection = glGetUniformLocation( ShaderProgram, "ModelViewProjectionMatrix" );

   Location.MaterialEmission = glGetUniformLocation( ShaderProgram, "Material.EmissionColor" );
   Location.MaterialAmbient = glGetUniformLocation( ShaderProgram, "Material.AmbientColor" );
   Location.MaterialDiffuse = glGetUniformLocation( ShaderProgram, "Material.DiffuseColor" );
   Location.MaterialSpecular = glGetUniformLocation( ShaderProgram, "Material.SpecularColor" );
   Location.MaterialSpecularExponent = glGetUniformLocation( ShaderProgram, "Material.SpecularExponent" );

   Location.Texture = glGetUniformLocation( ShaderProgram, "BaseTexture" );

   Location.UseLight = glGetUniformLocation( ShaderProgram, "UseLight" );
   Location.LightNum = glGetUniformLocation( ShaderProgram, "LightNum" );
   Location.GlobalAmbient = glGetUniformLocation( ShaderProgram, "GlobalAmbient" );

   Location.Lights.resize( light_num );
   for (uint i = 0; i < light_num; ++i) {
      Location.Lights[i].LightSwitch = glGetUniformLocation( ShaderProgram, string("Lights[" + to_string( i ) + "].LightSwitch").c_str() );
      Location.Lights[i].LightPosition = glGetUniformLocation( ShaderProgram, string("Lights[" + to_string( i ) + "].Position").c_str() );
      Location.Lights[i].LightAmbient = glGetUniformLocation( ShaderProgram, string("Lights[" + to_string( i ) + "].AmbientColor").c_str() );
      Location.Lights[i].LightDiffuse = glGetUniformLocation( ShaderProgram, string("Lights[" + to_string( i ) + "].DiffuseColor").c_str() );
      Location.Lights[i].LightSpecular = glGetUniformLocation( ShaderProgram, string("Lights[" + to_string( i ) + "].SpecularColor").c_str() );
      Location.Lights[i].SpotlightDirection = glGetUniformLocation( ShaderProgram, string("Lights[" + to_string( i ) + "].SpotlightDirection").c_str() );
      Location.Lights[i].SpotlightExponent = glGetUniformLocation( ShaderProgram, string("Lights[" + to_string( i ) + "].SpotlightExponent").c_str() );
      Location.Lights[i].SpotlightCutoffAngle = glGetUniformLocation( ShaderProgram, string("Lights[" + to_string( i ) + "].SpotlightCutoffAngle").c_str() );
      Location.Lights[i].LightAttenuationFactors = glGetUniformLocation( ShaderProgram, string("Lights[" + to_string( i ) + "].AttenuationFactors").c_str() );
   }
}

void ShaderGL::addUniformLocation(const GLuint& program, const string& name)
{
   CustomLocations[name] = glGetUniformLocation( program, name.c_str() );
}