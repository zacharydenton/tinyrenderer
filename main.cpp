#include <vector>
#include <iostream>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

using namespace std;

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
Model *model = nullptr;
constexpr int width = 800;
constexpr int height = 800;

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
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

int main(int argc, char** argv)
{
  if (argc == 2) {
    model = new Model(argv[1]);
  } else {
    model = new Model("obj/african_head.obj");
  }

  TGAImage image(width, height, TGAImage::RGB);
  for (auto i = 0; i < model->nfaces(); i++) { 
    vector<int> face = model->face(i); 
    for (auto j = 0; j < 3; j++) { 
      Vec3f v0 = model->vert(face[j]); 
      Vec3f v1 = model->vert(face[(j+1)%3]); 
      int x0 = (v0.x+1.)*width/2.; 
      int y0 = (v0.y+1.)*height/2.; 
      int x1 = (v1.x+1.)*width/2.; 
      int y1 = (v1.y+1.)*height/2.; 
      line(x0, y0, x1, y1, image, white); 
    } 
  }

  image.flip_vertically();
  image.write_tga_file("output.tga");
  return 0;
}
