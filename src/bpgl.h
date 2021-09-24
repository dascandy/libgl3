#pragma once

#include <cstdint>

struct BpglWindow {};

BpglWindow* bpglCreateWindow(uint32_t w, uint32_t h, const char* title);
void bpglSwapBuffers(BpglWindow* window);

