#include <string>
#include <vector>
#include <iostream>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

using namespace std;

TGAColor white = TGAColor(255, 255, 255, 255);
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

Vec3f barycentric(vector<Vec2i> pts, Vec2i P) { 
  Vec3f u = Vec3f(pts[2].x-pts[0].x, pts[1].x-pts[0].x, pts[0].x-P.x) ^ Vec3f(pts[2].y-pts[0].y, pts[1].y-pts[0].y, pts[0].y-P.y); 
  if (abs(u.z)<1) return Vec3f(-1,1,1); // triangle is degenerate, in this case return smth with negative coordinates 
  return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z); 
} 

void triangle(vector<Vec2i> pts, TGAImage &image, TGAColor color)
{ 
  Vec2i bboxmin(image.get_width() - 1, image.get_height() - 1);
  Vec2i bboxmax(0, 0);
  Vec2i clamp(image.get_width() - 1, image.get_height() - 1); 
  for (auto i=0; i<3; i++) { 
    bboxmin.x = std::max(0, std::min(bboxmin.x, pts[i].x)); 
    bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x)); 
    bboxmin.y = std::max(0, std::min(bboxmin.y, pts[i].y)); 
    bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y)); 
  } 
  Vec2i P; 
  for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) { 
    for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) { 
      Vec3f bc_screen  = barycentric(pts, P); 
      if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue; 
      image.set(P.x, P.y, color); 
    } 
  } 
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
    vector<Vec2i> screen_coords{3};
    vector<Vec3f> world_coords{3};
    for (int j=0; j<3; j++) { 
      Vec3f v = model.vert(face[j]); 
      screen_coords[j] = Vec2i((v.x+1.)*width/2., (v.y+1.)*height/2.); 
      world_coords[j] = v;
    } 
    Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
    n.normalize();
    auto light_dir = Vec3f(0, 0, -1);
    auto intensity = n * light_dir;
    if (intensity > 0) {
      triangle(screen_coords, image, white * intensity);
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
