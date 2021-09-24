#include "GL.h"
#include <cassert>
#include <fstream>
#include <string>
#include <algorithm>
#include <cmath>

static GlState _state;

struct BpglWindow {
  BpglWindow(uint32_t w, uint32_t h) {
    front = new GlTexture(w, h);
    back = new GlTexture(w, h);
    depth = new GlTexture(w, h);
  }
  GlTexture* front, *back, *depth;
};

BpglWindow* bpglCreateWindow(uint32_t w, uint32_t h, const char* title) {
  BpglWindow* win = new BpglWindow(w, h);
  _state.win = win;
  (void)title;
  return win;
}

struct TgaHeader {
   char  idlength;
   char  colourmaptype;
   char  datatypecode;
   uint16_t colourmaporigin;
   uint16_t colourmaplength;
   char  colourmapdepth;
   uint16_t x_origin;
   uint16_t y_origin;
   uint16_t width;
   uint16_t height;
   char  bitsperpixel;
   char  imagedescriptor;
} __attribute__((packed));

static_assert(sizeof(TgaHeader) == 18);

void bpglSwapBuffers(BpglWindow* window) {
  GlTexture* n = window->back;
  window->back = window->front;
  window->front = n;
  static int no = 0;
  no++;
  std::ofstream out(std::string("out") + std::to_string(no) + ".tga");
  TgaHeader header = { 0, 0, 2, 0, 0, 0, 0, 0, (uint16_t)n->width, (uint16_t)n->height, 32, 0x28 };
  out.write((const char*)&header, sizeof(header));
  out.write((const char*)n->buffer.data(), n->width * n->height * 4);
}

GLuint glGetError() {
  GLuint err = _state.error;
  _state.error = 0;
  return err;
}

void glSetError(GLuint err) {
  _state.error = err;
}

const GLubyte* glGetString(GLuint v) {
  switch(v) {
  case GL_RENDERER:
    return (const GLubyte*)"Bookish Potato Software Renderer";
  case GL_VERSION:
    return (const GLubyte*)"V0.1";
  default:
    glSetError(GL_INVALID_ENUM);
    return (const GLubyte*)"";
  }
}

void glEnable(GLuint v) {
  switch (v) {
  case GL_DEPTH_TEST:
    _state.depthTest = true;
    break;
  default:
    glSetError(GL_INVALID_ENUM);
    break;
  }
}

void glDepthFunc(GLuint f) {
  switch(f) {
  case GL_LESS:
    _state.depthFunc = f;
    break;
  default:
    glSetError(GL_INVALID_ENUM);
    break;
  }
}

void glGenBuffers(GLuint count, GLuint* buf) {
  size_t curTarget = 0;
  for (size_t index = 1; curTarget != count && index < _state.buffers.size(); index++) {
    if (_state.buffers[index] == nullptr) {
      _state.buffers[index] = new GlBuffer();
      buf[curTarget++] = index;
    }
  }
  while (curTarget < count) {
    buf[curTarget++] = _state.buffers.size();
    _state.buffers.push_back(new GlBuffer());
  }
}

void glBindBuffer(GLuint v, GLuint buffer) {
  switch(v) {
  case GL_ARRAY_BUFFER:
    _state.arrayBuffer = buffer;
    break;
  default:
    glSetError(GL_INVALID_ENUM);
    break;
  }
}

void glBufferData(GLuint v, GLsizei byteCount, void* data, GLuint usage) {
  (void)usage;
  switch(v) {
  case GL_ARRAY_BUFFER:
    assert(_state.buffers[_state.arrayBuffer] != nullptr);
    (*_state.buffers[_state.arrayBuffer]) = std::vector<uint8_t>((const uint8_t*)data, (const uint8_t*)data + byteCount);
    break;
  default:
    glSetError(GL_INVALID_ENUM);
    break;
  }
}

void glGenVertexArrays(GLuint count, GLuint* buf) {
  size_t curTarget = 0;
  for (size_t index = 1; curTarget != count && index < _state.vaos.size(); index++) {
    if (_state.vaos[index] == nullptr) {
      _state.vaos[index] = new GlVao();
      buf[curTarget++] = index;
    }
  }
  while (curTarget < count) {
    buf[curTarget++] = _state.vaos.size();
    _state.vaos.push_back(new GlVao());
  }
}

void glBindVertexArray(GLuint vao) {
  assert(_state.vaos[vao] != nullptr);
  _state.curVao = vao;
}

void glEnableVertexAttribArray(GLuint index) {
  auto& o = *_state.vaos[_state.curVao];
  if (o.size() <= index) o.resize(index+1);
  o[index] = new VaoEntry();
}
 
void glVertexAttribPointer(GLuint index, GLuint count, GLuint type, GLuint normalized, GLuint stride, const void* pointer) {
  assert(_state.vaos[_state.curVao] != nullptr);
  assert((*_state.vaos[_state.curVao])[index] != nullptr);
  auto& entry = *(*_state.vaos[_state.curVao])[index];
  entry.count = count;
  entry.type = type;
  entry.normalized = normalized;
  entry.stride = stride ? stride : count * 4;
  entry.offset = (uintptr_t)pointer;
}

GLuint glCreateShader(GLuint type) {
  size_t index = 1;
  while (index != _state.shaders.size() && _state.shaders[index] != nullptr) index++;
  if (index == _state.shaders.size()) _state.shaders.push_back(nullptr);
  _state.shaders[index] = new GlShader{type};
  return index;
}

void glShaderSource(GLuint index, GLCpuShader func) {
  _state.shaders[index]->func = func;
}

void glShaderSource(GLuint, GLuint, GLubyte*, void*) {
  assert(false);
}

void glCompileShader(GLuint) {
}

GLuint glCreateProgram() {
  size_t index = 1;
  while (index != _state.programs.size() && _state.programs[index] != nullptr) index++;
  if (index == _state.programs.size()) _state.programs.push_back(nullptr);
  _state.programs[index] = new GlShaderProgram();
  return index;
}

void glAttachShader(GLuint program, GLuint shader) {
  _state.programs[program]->add(_state.shaders[shader]);
}

void glLinkProgram(GLuint index) {
  _state.programs[index]->link();
}

void glClearColor4f(float r, float g, float b, float a) {
  uint8_t r8 = 255 * r, g8 = 255 * g, b8 = 255 * b, a8 = 255 * a;
  _state.clearColor = (a8 << 24) | (r8 << 16) | (g8 << 8) | (b8 << 0);
}

void glClear(GLuint bits) {
  if (bits & GL_COLOR_BUFFER_BIT) {
    _state.win->back->clear(_state.clearColor);
  }
  if (bits & GL_DEPTH_BUFFER_BIT) {
    _state.win->depth->clear(0);
  }
}

void glUseProgram(GLuint index) {
  _state.curShader = _state.programs[index];
}

void ReadVao(std::vector<VaoEntry*>& entries, GLuint v, GLvec4* buf) {
  uint8_t* buffer = _state.buffers[_state.arrayBuffer]->data();
  for (size_t index = 0; index < entries.size(); index++) {
    if (entries[index] == nullptr) continue;
    auto& e = *entries[index];
    switch(e.type) {
    case GL_FLOAT:
    {
      GLvec4 val = {0.0f, 0.0f, 0.0f, 1.0f};
      for (size_t n = 0; n < e.count; n++) {
        val[n] = *(float*)(buffer + v * e.stride + e.offset + sizeof(float) * n);
      }
      buf[index] = val;
    }
      break;
    }
  }
}

void GetVertex(GLuint v, GLvec4* buf) {
  GLvec4* uniform = nullptr;
  GLvec4 buffer[16];
  ReadVao(*_state.vaos[_state.curVao], v, buffer);

  _state.curShader->vs->func(uniform, buffer, buf, _state);

  float w = buf[0][3];
  buf[0] /= w;
  buf[0][3] = 1/w;
  for (size_t n = 1; n < 1; n++) {
    buf[n] /= w;
  }
}

float edgeFunction(const GLvec4 &a, const GLvec4 &b, const GLvec4 &c) 
{ 
  float ef = (c[0] - a[0]) * (b[1] - a[1]) - (c[1] - a[1]) * (b[0] - a[0]); 
  if (ef != 0) return ef;
  GLvec4 d = b - a;
  return (d[1] > 0 || (d[1] == 0 && d[0] < 0));
}

void RenderTriangle(GLuint a, GLuint b, GLuint c) {
  GLvec4 va[16], vb[16], vc[16];
  GetVertex(a, va);
  GetVertex(b, vb);
  GetVertex(c, vc);

  GlTexture* target = _state.win->back;
//  GlTexture* depth = _state.win->depth;
  GLvec4 screensize = GLvec4{(float)target->width, (float)target->height, 0.0f, 0.0f};
  GLvec4 v0 = ((va[0] + 1.0f) / 2.0f) * screensize;
  GLvec4 v1 = ((vb[0] + 1.0f) / 2.0f) * screensize;
  GLvec4 v2 = ((vc[0] + 1.0f) / 2.0f) * screensize;
  float area = edgeFunction(v0, v1, v2);
  if (area <= 0) return; // filters out degenerate and backface polygons

  int32_t xmin = (int32_t)std::floor(std::min({v0[0], v1[0], v2[0]}) + 0.5f), ymin = std::floor(std::min({v0[1], v1[1], v2[1]}) + 0.5f);
  int32_t xmax = (int32_t)std::floor(std::max({v0[0], v1[0], v2[0]}) + 0.5f), ymax = std::floor(std::max({v0[1], v1[1], v2[1]}) + 0.5f);
  if (xmin >= (int32_t)target->width || ymin >= (int32_t)target->height || xmax < 0 || ymax < 0) return; // out of window

  if (xmin < 0) xmin = 0;
  if (xmax > (int32_t)target->width) xmax = target->width;
  if (ymin < 0) ymin = 0;
  if (ymax > (int32_t)target->height) ymax = target->height;
//  GLvec4 out[16];
  for (uint32_t j = ymin; j < (uint32_t)ymax; ++j) { 
    for (uint32_t i = xmin; i < (uint32_t)xmax; ++i) { 
      GLvec4 p = {i + 0.5f, j + 0.5f, 0, 0}; 
      float w0 = edgeFunction(v1, v2, p) / area; 
      float w1 = edgeFunction(v2, v0, p) / area; 
      float w2 = edgeFunction(v0, v1, p) / area; 
      if (w0 >= 0 && w1 >= 0 && w2 >= 0) { 
        target->buffer[j*target->width+i] = 0xff00ffff;
/*
        float r = w0 * c0[0] + w1 * c1[0] + w2 * c2[0]; 
        float g = w0 * c0[1] + w1 * c1[1] + w2 * c2[1]; 
        float b = w0 * c0[2] + w1 * c1[2] + w2 * c2[2]; 
        framebuffer[j * w + i][0] = (unsigned char)(r * 255); 
        framebuffer[j * w + i][1] = (unsigned char)(g * 255); 
        framebuffer[j * w + i][2] = (unsigned char)(b * 255); 
*/
      } 
    } 
  }   
}

void glDrawArrays(GLuint type, GLuint start, GLuint count) {
  switch(type) {
  case GL_TRIANGLES:
    for (size_t n = start; n < count; n += 3) {
      RenderTriangle(n, n+1, n+2);
    }
    break;
  default:
    glSetError(GL_INVALID_ENUM);
    break;
  }
}


