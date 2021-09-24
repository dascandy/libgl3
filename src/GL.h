#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

using GLubyte = uint8_t;
using GLuint = uint32_t;
using GLsizei = uint32_t;
using GLvec4 = float __attribute__((__vector_size__(16)));

enum : GLuint {
  GL_FALSE = false,
  GL_FLOAT,
  GL_DEPTH_TEST,
  GL_LESS,
  GL_ARRAY_BUFFER,
  GL_STATIC_DRAW,
  GL_VERTEX_SHADER,
  GL_FRAGMENT_SHADER,
  GL_COLOR_BUFFER_BIT,
  GL_DEPTH_BUFFER_BIT,
  GL_TRIANGLES,
  GL_RENDERER,
  GL_VERSION,
  GL_INVALID_ENUM,
  GL_INVALID_OPERATION,
  GL_RGBA8
};

using GlBuffer = std::vector<uint8_t>;

struct VaoEntry {
  GLuint count = 0;
  GLuint type = 0;
  bool normalized = false;
  GLsizei stride = 0;
  GLsizei offset = 0;
};

using GlVao = std::vector<VaoEntry*>;

struct GlState;

using GLCpuShader = void(*)(const GLvec4*, const GLvec4*, GLvec4*, GlState&);

struct GlShader {
  GLuint type;
  GLCpuShader func = nullptr;
};

void glSetError(GLuint);
struct GlShaderProgram {
  void add(GlShader* shader) {
    switch(shader->type) {
      case GL_VERTEX_SHADER:
        vs = shader;
        break;
      case GL_FRAGMENT_SHADER:
        fs = shader;
        break;
      default:
        glSetError(GL_INVALID_ENUM);
        break;
    }
  }
  void link() {
    if (fs == nullptr || vs == nullptr) {
      glSetError(GL_INVALID_OPERATION);
    }
  }
  GlShader* fs = nullptr, *gs = nullptr, *vs = nullptr;
};

struct GlTexture {
  GLuint pixelFormat;
  GLuint width, height;
  std::vector<uint32_t> buffer;
  GlTexture(uint32_t w, uint32_t h) 
  : pixelFormat(GL_RGBA8)
  , width(w)
  , height(h)
  {
    buffer.resize(w * h);
  }
  void clear(uint32_t value) {
    for (auto& v : buffer) v = value;
  }
};

struct BpglWindow;
struct GlState {
  GLuint error = 0;
  bool depthTest = false;
  GLuint depthFunc = GL_LESS;
  std::vector<GlBuffer*> buffers = {nullptr};
  std::vector<GlVao*> vaos = {nullptr};
  std::vector<GlShader*> shaders = {nullptr};
  std::vector<GlShaderProgram*> programs = {nullptr};
  uint32_t clearColor = 0;
  GLuint arrayBuffer = 0;
  GLuint curVao = 0;
  GlShaderProgram* curShader = nullptr;
  BpglWindow* win;
};

const GLubyte* glGetString(GLuint);
void glEnable(GLuint);
void glDepthFunc(GLuint);

void glGenBuffers(GLuint, GLuint*);
void glBindBuffer(GLuint, GLuint);
void glBufferData(GLuint, GLsizei, void*, GLuint);
void glGenVertexArrays(GLuint, GLuint*);
void glBindVertexArray(GLuint);
void glEnableVertexAttribArray(GLuint);
void glVertexAttribPointer(GLuint, GLuint, GLuint, GLuint, GLuint, const void*);
GLuint glCreateShader(GLuint);
void glShaderSource(GLuint, GLCpuShader);
void glShaderSource(GLuint, GLuint, GLubyte*, void*);
void glCompileShader(GLuint);
GLuint glCreateProgram();
void glAttachShader(GLuint, GLuint);
void glLinkProgram(GLuint);
void glClearColor4f(float, float, float, float);
void glClear(GLuint);
void glUseProgram(GLuint);
void glBindVertexArray(GLuint);
void glDrawArrays(GLuint, GLuint, GLuint);

