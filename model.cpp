#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "model.h"

using namespace std;

Model::Model(const string filename) : verts_(), faces_() {
    ifstream in;
    in.open (filename, ifstream::in);
    if (in.fail()) return;
    string line;
    while (!in.eof()) {
        getline(in, line);
        istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            vec3 v;
            iss >> v.x;
            iss >> v.y;
            iss >> v.z;
            verts_.push_back(v);
        } else if (!line.compare(0, 2, "f ")) {
            vector<int> f;
            int itrash, idx;
            iss >> trash;
            while (iss >> idx >> trash >> itrash >> trash >> itrash) {
                idx--; // in wavefront obj all indices start at 1, not zero
                f.push_back(idx);
            }
            faces_.push_back(f);
        }
    }
    cerr << "# v# " << verts_.size() << " f# "  << faces_.size() << endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

vector<int> Model::face(int idx) {
    return faces_[idx];
}

vec3 Model::vert(int i) {
    return verts_[i];
}

