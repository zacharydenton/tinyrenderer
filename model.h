#ifndef __MODEL_H__
#define __MODEL_H__

#include <string>
#include <vector>
#include "geometry.h"

using namespace std;

class Model {
private:
	vector<Vec3f> verts_;
	vector<vector<int> > faces_;
public:
	Model(const string filename);
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	vector<int> face(int idx);
};

#endif //__MODEL_H__
