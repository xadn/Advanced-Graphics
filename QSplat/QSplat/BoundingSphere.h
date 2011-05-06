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


typedef list<vec3dd*> vert_ls;
typedef vert_ls::iterator vert_it;


class BoundingSphere
{    
public:
    BoundingSphere(vert_ls verts);    
    list<vec3dd> recurseToDepth(int depth);
    
private:
    vec3dd findCenter(vert_ls verts);
    vert_ls* partitionMesh(vert_ls verts);
    vert_ls* partitionMesh2(vert_ls verts);
    
    bool leaf;
    double size;
    vec3dd center;
    
    BoundingSphere* leftSubTree;
    BoundingSphere* rightSubTree;
};