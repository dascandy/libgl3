#include "GL.h"
#include "bpgl.h"
#include <stdio.h>

int main() {
  BpglWindow* window = bpglCreateWindow(640, 480, "Hello Triangle");

  const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
  const GLubyte* version = glGetString(GL_VERSION); // version as a string
  printf("Renderer: %s\n", renderer);
  printf("OpenGL version supported %s\n", version);

  // tell GL to only draw onto a pixel if the shape is closer to the viewer
  glEnable(GL_DEPTH_TEST); // enable depth-testing
  glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"

  float points[] = {
     0.0f,  0.5f,  0.0f,
     0.5f, -0.5f,  0.0f,
    -0.5f, -0.5f,  0.0f,
  };

  GLuint vbo = 0;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), points, GL_STATIC_DRAW);

  GLuint vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  GLCpuShader vertex_shader = [](const GLvec4* uniforms, const GLvec4* in, GLvec4* out, GlState& state) -> void {
    (void)uniforms;
    (void)state;
    out[0] = in[0];
  };

  GLCpuShader fragment_shader = [](const GLvec4* uniforms, const GLvec4* in, GLvec4* out, GlState& state) -> void {
    (void)uniforms;
    (void)state;
    (void)in;
    out[0] = GLvec4{0.5, 0.0, 0.5, 1.0};
  };

  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, vertex_shader);
  glCompileShader(vs);
  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, fragment_shader);
  glCompileShader(fs);

  GLuint shader_programme = glCreateProgram();
  glAttachShader(shader_programme, fs);
  glAttachShader(shader_programme, vs);
  glLinkProgram(shader_programme);

  glClearColor4f(0.7f, 0.6f, 0.8f, 1.0f);

  size_t frame = 0;
  while(frame++ < 60) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shader_programme);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    bpglSwapBuffers(window);
  }
}

