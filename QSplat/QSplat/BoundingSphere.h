//
//  BoundingSphere.h
//  QSplat
//
//  Created by aniccola on 5/4/11.
//  Copyright 2011 Colorado School of Mines. All rights reserved.
//
#include <list>
#include <vector>
#include "vec3d.h"

using namespace std;


class Point : public vec3dd
{
public:
    double size;
    vec3dd normal;
};

typedef list<Point*> vert_ls;
typedef vert_ls::iterator vert_it;


class BoundingSphere
{    
public:
    BoundingSphere(vert_ls verts);
    list<vec3dd> recurseToDepth(int depth);
    vec3dd normal() { return norm; }
    
private:
    vec3dd findCenter(vert_ls verts);
    vert_ls* partitionMesh(vert_ls verts);
    double length(vec3dd vert);
    void splat_vertices();
    
    bool leaf;
    double size;
    vec3dd center;
    vec3dd norm;
    vec3dd splat[4];
    
    BoundingSphere* leftSubTree;
    BoundingSphere* rightSubTree;
};