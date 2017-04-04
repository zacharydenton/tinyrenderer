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

void bresenham_line(Vec2i t0, Vec2i t1, TGAImage &image, TGAColor color)
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

#define ipart(X) ((int)(X))
#define fpart(X) (((double)(X))-(double)ipart(X))
#define rfpart(X) (1.0-fpart(X))

void wu_line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
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

  double dx = x1 - x0;
  double dy = y1 - y0;
  double gradient = (dx == 0) ? 1 : dy / dx;

  // Draw the first endpoint.
  auto xend = round(x0);
  auto yend = y0 + gradient * (xend - x0);
  auto xgap = rfpart(x0 + 0.5);
  auto xpxl1 = xend;
  auto ypxl1 = ipart(yend);
  auto intery = yend + gradient;
  if (steep) {
    image.set(ypxl1, xpxl1, color * rfpart(yend) * xgap);
    image.set(ypxl1 + 1, xpxl1,  color * fpart(yend) * xgap);
  } else {
    image.set(xpxl1, ypxl1, color * rfpart(yend) * xgap);
    image.set(xpxl1, ypxl1 + 1, color * fpart(yend) * xgap);
  }

  // Draw the other endpoint.
  xend = round(x1);
  yend = y1 + gradient * (xend - x1);
  xgap = fpart(x1 + 0.5);
  auto xpxl2 = xend;
  auto ypxl2 = ipart(yend);
  if (steep) {
    image.set(ypxl2, xpxl2, color * rfpart(yend) * xgap);
    image.set(ypxl2 + 1, xpxl2,  color * fpart(yend) * xgap);
  } else {
    image.set(xpxl2, ypxl2, color * rfpart(yend) * xgap);
    image.set(xpxl2, ypxl2 + 1, color * fpart(yend) * xgap);
  }

  // Main loop.
  if (steep) {
    for (auto x = xpxl1 + 1; x < xpxl2; x++) {
      image.set(ipart(intery), x, color * rfpart(intery));
      image.set(ipart(intery) + 1, x, color * fpart(intery));
      intery += gradient;
    }
  } else {
    for (auto x = xpxl1 + 1; x < xpxl2; x++) {
      image.set(x, ipart(intery), color * rfpart(intery));
      image.set(x, ipart(intery) + 1, color * fpart(intery));
      intery += gradient;
    }
  }
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color)
{ 
  bresenham_line(t0, t1, image, color); 
  bresenham_line(t1, t2, image, color); 
  bresenham_line(t2, t0, image, color); 
}

void line(Vec2i t0, Vec2i t1, TGAImage &image, TGAColor color)
{
  wu_line(t0.x, t0.y, t1.x, t1.y, image, color);
}

void draw_model(const string filename, TGAImage &image)
{
  Model model(filename);
  for (auto i = 0; i < model.nfaces(); i++) { 
    vector<int> face = model.face(i); 
    for (auto j = 0; j < 3; j++) { 
      Vec3f v0 = model.vert(face[j]); 
      Vec3f v1 = model.vert(face[(j+1)%3]); 
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
  draw_model("obj/african_head.obj", image);
  image.flip_vertically();
  image.write_tga_file("output.tga");
  return 0;
}
