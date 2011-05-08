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
    vert_ls recurseToDepth(int depth);
    
private:
    vert_ls* partitionMesh(vert_ls verts);
    double length(vec3dd vert);
    
    bool leaf;
    Point center;
    
    BoundingSphere* leftSubTree;
    BoundingSphere* rightSubTree;
};