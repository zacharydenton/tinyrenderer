#include <limits>
#include <string>
#include <vector>
#include <iostream>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "vec.h"

using namespace std;

TGAColor white = TGAColor(255, 255, 255, 255);
TGAColor red   = TGAColor(255, 0,   0,   255);
TGAColor green = TGAColor(0, 255,   0,   255);
constexpr int width = 800;
constexpr int height = 800;

void bresenham_line(vec2i start, vec2i end, TGAImage &image, TGAColor color)
{
  auto steep = abs(end.y - start.y) > abs(end.x - start.x);
  if (steep) {
    swap(start.x, start.y);
    swap(end.x, end.y);
  }
  if (start.x > start.y) {
    swap(start, end);
  }
  auto dx = end.x - start.x;
  auto dy = abs(end.y - start.y);
  auto derr2 = abs(dy) * 2;
  auto err2 = 0;
  auto y = start.y;
  auto ystep = end.y > start.y ? 1 : -1;
  for (int x = start.x; x <= end.x; x++) {
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

void wu_line(vec2 start, vec2 end, TGAImage &image, TGAColor color)
{
  auto steep = abs(end.y - start.y) > abs(end.x - start.x);
  if (steep) {
    swap(start.x, start.y);
    swap(end.x, end.y);
  }
  if (start.x > start.y) {
    swap(start, end);
  }

  auto dx = end.x - start.x;
  auto dy = end.y - start.y;
  auto gradient = (dx == 0) ? 1.0 : dy / dx;

  // Draw the first endpoint.
  auto xend = round(start.x);
  auto yend = start.y + gradient * (xend - start.x);
  auto xgap = rfpart(start.x + 0.5);
  auto xpxl1 = xend;
  auto ypxl1 = ipart(yend);
  auto intery = yend + gradient;
  if (steep) {
    image.set(ypxl1, xpxl1, color * rfpart(yend) * xgap);
    image.set(ypxl1 + 1, xpxl1, color * fpart(yend) * xgap);
  } else {
    image.set(xpxl1, ypxl1, color * rfpart(yend) * xgap);
    image.set(xpxl1, ypxl1 + 1, color * fpart(yend) * xgap);
  }

  // Draw the other endpoint.
  xend = round(end.x);
  yend = end.y + gradient * (xend - end.x);
  xgap = fpart(end.x + 0.5);
  auto xpxl2 = xend;
  auto ypxl2 = ipart(yend);
  if (steep) {
    image.set(ypxl2, xpxl2, color * rfpart(yend) * xgap);
    image.set(ypxl2 + 1, xpxl2, color * fpart(yend) * xgap);
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

vec3 barycentric(vector<vec2i> pts, vec2i P) {
  vec3 u = vec3(pts[2].x - pts[0].x,
                pts[1].x - pts[0].x,
                pts[0].x  -P.x) ^ vec3(pts[2].y - pts[0].y,
                                       pts[1].y - pts[0].y,
                                       pts[0].y - P.y);

  if (abs(u.z) < 1) {
    // Triangle is degenerate.
    return vec3(-1, 1, 1);
  }

  return vec3(1 - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

vec3 barycentric(vec3 A, vec3 B, vec3 C, vec3 P) {
    vec3 s[2];
    for (int i=2; i--; ) {
        s[i].x = C[i]-A[i];
        s[i].y = B[i]-A[i];
        s[i].z = A[i]-P[i];
    }
    vec3 u = s[0] ^ s[1];
    if (abs(u[2]) > 1e-2) {
        return vec3(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
    }
    return vec3(-1,1,1);
}

vec3 world2screen(const vec3 &v) {
  return vec3(int((v.x+1.)*width/2.+.5), int((v.y+1.)*height/2.+.5), v.z);
}

void triangle(vector<vec3> pts, vector<double> &zbuffer, TGAImage &image, TGAColor color)
{
  vec2 bboxmin(numeric_limits<double>::max(), numeric_limits<double>::max());
  vec2 bboxmax(-numeric_limits<double>::max(), -numeric_limits<double>::max());
  vec2 clamp(image.get_width() - 1, image.get_height() - 1);
  for (auto i = 0; i < 3; i++) {
    bboxmin.x = max(0., min(bboxmin.x, pts[i].x));
    bboxmax.x = min(clamp.x, max(bboxmax.x, pts[i].x));
    bboxmin.y = max(0., min(bboxmin.y, pts[i].y));
    bboxmax.y = min(clamp.y, max(bboxmax.y, pts[i].y));
  }
  vec3 P;
  for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
    for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
      vec3 bc_screen = barycentric(pts[0], pts[1], pts[2], P);
      if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
      P.z = 0;
      for (auto i = 0; i < 3; i++) {
        P.z += pts[i].z * bc_screen[i];
      }
      auto z_idx = int(P.y * image.get_width() + P.x);
      if (zbuffer[z_idx] < P.z) {
        zbuffer[z_idx] = P.z;
        image.set(P.x, P.y, color);
      }
    }
  }
}

void line(vec2 start, vec2 end, TGAImage &image, TGAColor color)
{
  wu_line(start, end, image, color);
}

void draw_model(const string &filename, TGAImage &image)
{
  Model model(filename);
  vector<double> zbuffer(image.get_width() * image.get_height());
  fill(zbuffer.begin(), zbuffer.end(), -numeric_limits<double>::max());
  auto light_dir = vec3(0, 0, -1);
  for (auto i = 0; i < model.nfaces(); i++) {
    auto face = model.face(i);
    vector<vec3> screen_coords{3};
    vector<vec3> world_coords{3};
    for (auto j = 0; j < 3; j++) {
      auto v = model.vert(face[j]);
      screen_coords[j] = world2screen(v);
      world_coords[j] = v;
    }
    vec3 n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
    n.normalize();
    auto intensity = n * light_dir;
    if (intensity > 0) {
      triangle(screen_coords, zbuffer, image, red * intensity);
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
