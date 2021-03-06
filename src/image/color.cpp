// Distributed under the BSD License, see accompanying LICENSE.txt
// (C) Copyright 2010 John-John Tedro et al.
#include "image/color.hpp"

#include <sstream>
#include <string>

uint8_t alpha_over_c(uint8_t u, uint8_t o, uint8_t ua, uint8_t oa);

/**
 * Takes two color values and does an alpha over blending without using floats.
 */
inline uint8_t alpha_over_c(uint8_t ac, uint8_t aa, uint8_t bc, uint8_t ba) {
  return ((ac * aa) + ((0xff * (bc * ba) - aa * (bc * ba)) / 0xff)) / 0xff;
}

void color::blend(const color &other) {
  if (other.is_invisible()) return;

  if (other.is_opaque() || is_invisible()) {
    r = other.r;
    g = other.g;
    b = other.b;
    a = other.a;
    return;
  }
  
  r = alpha_over_c(other.r, other.a, r, a);
  g = alpha_over_c(other.g, other.a, g, a);
  b = alpha_over_c(other.b, other.a, b, a);
  a = a + (other.a * (0xff - a)) / 0xff;
  r = ((r * 0xff) / a);
  g = ((g * 0xff) / a);
  b = ((b * 0xff) / a);
}

// pull color down towards black
void color::darken(uint8_t f) {
  r = std::max((int)r - ((int)r * (int)f) / 0xff, 0x0);
  g = std::max((int)g - ((int)g * (int)f) / 0xff, 0x0);
  b = std::max((int)b - ((int)b * (int)f) / 0xff, 0x0);
}

void color::lighten(uint8_t f) {
  r = std::min((int)r + ((int)r * (int)f) / 0xff, 0xff);
  g = std::min((int)g + ((int)g * (int)f) / 0xff, 0xff);
  b = std::min((int)b + ((int)b * (int)f) / 0xff, 0xff);
}
