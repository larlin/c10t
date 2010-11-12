#ifndef ENGINE_BASE
#define ENGINE_BASE

#include <boost/shared_ptr.hpp>

#include "level.hpp"
#include "world.hpp"

class engine_base {
  protected:
    settings_t& s;
    const Cube part_c, pos_c, mpos_c;
    const world_info& world;
  public:
    engine_base(settings_t& s, world_info& world) :
      s(s),
      part_c(mc::MapX + 1, mc::MapY + 1, mc::MapZ + 1),
      pos_c((world.diff_x + 1) * mc::MapX, mc::MapY, (world.diff_z + 1) * mc::MapZ),
      mpos_c((world.diff_x + 1) * mc::MapX, mc::MapY, (world.diff_z + 1) * mc::MapZ),
      world(world)
    {
    }
    
    virtual void render(level_file& level, boost::shared_ptr<image_operations> oper) = 0;
    virtual void get_boundaries(size_t& width, size_t& height) = 0;
    virtual void get_level_boundaries(size_t& width, size_t& height) = 0;
    virtual void w2pt(int xPos, int zPos, size_t& x, size_t& y) = 0;
    virtual void wp2pt(int xPos, int yPos, int zPos, size_t& x, size_t& y) = 0;
};

class BlockRotation {
private:
  settings_t& s;
  nbt::ByteArray *byte_array;

  int x, z;

  void transform_xz(int& x, int& z) {
    int t = x;
    switch (s.rotation) {
      case 270:
        x = mc::MapX - z - 1;
        z = t;
        break;
      case 180:
        z = mc::MapZ - z - 1;
        x = mc::MapX - x - 1;
        break;
      case 90:
        x = z;
        z = mc::MapZ - t - 1;
        break;
    };
  }

public:
  BlockRotation(settings_t& s, nbt::ByteArray *byte_array)
    : s(s), byte_array(byte_array) {}

  void set_xz(int x, int z) {
    transform_xz(x, z);
    this->x = x;
    this->z = z;
  }
  
  /**
   * Blocks[ z + ( y * ChunkSizeY(=128) + ( x * ChunkSizeY(=128) * ChunkSizeZ(=16) ) ) ]; 
   */
  uint8_t get8(int y) {
    int p = y + (z * mc::MapY) + (x * mc::MapY * mc::MapZ);
    if (!(p >= 0 && p < byte_array->length)) return -1;
    return byte_array->values[p];
  }
  
  uint8_t get8() {
    int p = x + (z * mc::MapX);
    assert (p >= 0 && p < byte_array->length);
    return byte_array->values[p];
  }
  
  int get4(int y) {
    int p = (y + (z * mc::MapY) + (x * mc::MapY * mc::MapZ)) >> 1;
    if (!(p >= 0 && p < byte_array->length)) return -1;
    return ((byte_array->values[p]) >> ((y % 2) * 4)) & 0xf;
  }
};

inline void apply_shading(settings_t& s, int bl, int sl, int hm, int y, color &c) {
  // if night, darken all colors not emitting light
  
  if (bl == -1) bl = 0;
  
  if(s.night) {
    c.darken(0xa * (16 - bl));
  }
  else if (sl != -1 && y != s.top) {
    c.darken(0xa * (16 - std::max(sl, bl)));
  }
  
  //c.darken((mc::MapY - y));
  
  // in heightmap mode, brightness = height
  if (s.heightmap) {
    c.b = y*2;
    c.g = y*2;
    c.r = y*2;
    c.a = 0xff;
  }
  
  if (s.striped_terrain && y % 2 == 0) {
    c.darken(0xf);
  }
}

inline bool is_open(int bt) {
  if (bt == -1) {
    return false;
  }
  
  switch(bt) {
    case mc::Air: return true;
    case mc::Leaves: return true;
    default: return false;
  }
}

inline bool cave_ignore_block(settings_t& s, int y, int bt, BlockRotation& b_r, bool &cave_initial) {
  if (cave_initial) {
    if (!is_open(bt)) {
      cave_initial = false;
      return true;
    }
    
    return true;
  }
  
  if (!is_open(bt) && is_open(b_r.get8(y + 1))) {
    return false;
  }
  
  return true;
}

inline bool hell_ignore_block(settings_t& s, int y, int bt, BlockRotation& b_r, bool &hell_initial) {
  if (hell_initial) {
    if (is_open(bt)) {
      hell_initial = false;
      return false;
    }
    
    return true;
  }
  
  return false;
}

#endif /* ENGINE_BASE */
