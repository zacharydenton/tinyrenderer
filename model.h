#ifndef __MODEL_H__
#define __MODEL_H__

#include <string>
#include <vector>
#include "vec.h"

using namespace std;

class Model {
private:
	vector<vec3> verts_;
	vector<vector<int> > faces_;
public:
	Model(const string filename);
	~Model();
	int nverts();
	int nfaces();
	vec3 vert(int i);
	vector<int> face(int idx);
};

#endif //__MODEL_H__
