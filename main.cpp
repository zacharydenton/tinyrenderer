#include <string>
#include <vector>
#include <iostream>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

using namespace std;

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0, 255,   0,   255);
constexpr int width = 800;
constexpr int height = 800;

void line(Vec2i t0, Vec2i t1, TGAImage &image, TGAColor color)
{
  auto x0 = t0.x;
  auto y0 = t0.y;
  auto x1 = t1.x;
  auto y1 = t1.y;
  auto steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }
  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }
  auto dx = x1 - x0;
  auto dy = abs(y1 - y0);
  auto derr2 = abs(dy) * 2;
  auto err2 = 0;
  auto y = y0;
  auto ystep = y1 > y0 ? 1 : -1;
  for (int x = x0; x <= x1; x++) {
    if (steep) {
      image.set(y, x, color);
    } else {
      image.set(x, y, color);
    }
    err2 += derr2;
    if (err2 > dx) {
      y += ystep;
      err2 -= 2 * dx;
    }
  }
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color)
{ 
  line(t0, t1, image, color); 
  line(t1, t2, image, color); 
  line(t2, t0, image, color); 
}

void draw_model(const string filename, TGAImage &image)
{
  auto model = new Model(filename);
  for (auto i = 0; i < model->nfaces(); i++) { 
    vector<int> face = model->face(i); 
    for (auto j = 0; j < 3; j++) { 
      Vec3f v0 = model->vert(face[j]); 
      Vec3f v1 = model->vert(face[(j+1)%3]); 
      int x0 = (v0.x+1.)*width/2.; 
      int y0 = (v0.y+1.)*height/2.; 
      int x1 = (v1.x+1.)*width/2.; 
      int y1 = (v1.y+1.)*height/2.; 
      line(Vec2i(x0, y0), Vec2i(x1, y1), image, white); 
    } 
  }
}

int main(int argc, char** argv)
{
  TGAImage image(width, height, TGAImage::RGB);
  Vec2i t0[3] = {Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)}; 
  Vec2i t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)}; 
  Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)}; 
  triangle(t0[0], t0[1], t0[2], image, red); 
  triangle(t1[0], t1[1], t1[2], image, white); 
  triangle(t2[0], t2[1], t2[2], image, green);
  image.flip_vertically();
  image.write_tga_file("output.tga");
  return 0;
}
