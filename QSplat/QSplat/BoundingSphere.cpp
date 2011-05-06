//
//  BoundingSphere.cpp
//  QSplat
//
//  Created by aniccola on 5/4/11.
//  Copyright 2011 Colorado School of Mines. All rights reserved.
//

#include <math.h>
#include "BoundingSphere.h"


BoundingSphere::BoundingSphere(vert_ls verts)
{    
    static int count;
    count++;
    //cout << "node count: " << count << endl;
    
    vert_ls* partitions;
    
    switch (verts.size()) {
        case 0:
            cout << "error: shouldn't be empty\n";
            exit(EXIT_FAILURE);
            break;
            
        case 1:
            leaf = true;
            center = *verts.front();
            norm = verts.front()->normal;
            break;
            
        default:
            leaf = false;
            center = findCenter(verts);
            partitions = partitionMesh(verts);
            leftSubTree = new BoundingSphere(partitions[0]);
            rightSubTree = new BoundingSphere(partitions[1]);
            break;
    }
}


vec3dd BoundingSphere::findCenter(vert_ls verts)
{
    vec3dd average(0,0,0);
    
    for (vert_it it = verts.begin(); it != verts.end(); it++)
    {
        average += **it;
    }
    
    return (1.0/(double)verts.size())*average;
}


// Partition the mesh into equal parts
vert_ls* BoundingSphere::partitionMesh(vert_ls verts)
{
    vec3dd max(0,0,0);
    vec3dd min(0,0,0);
    vec3dd mid(0,0,0);
    vec3dd distance(0,0,0);
    int longest_axis;
    vert_ls* partitions = new vert_ls[2];    
    
    // Calculate the bounding box
    max = *verts.front();
    min = *verts.front();
    for(vert_it it = verts.begin(); it != verts.end(); it++)
    {
        max |= **it;
        min &= **it;
    }
    
    // Find the longest axis (x, y, z) to sort by
    double max_axis;
    distance = max - min;
    
    // Find the longest axis 
    for(int i=0; i<3; i++)
    {
        if (distance[i] > max_axis) {
            max_axis = distance[i];
            longest_axis = i;
        }
    }
    
    size = max_axis;
    
    // Find the midpoint of the bounding box
    mid = 0.5*(max+min);
    
    // Sort the vertices along the axis
    while ( !verts.empty() )
    {        
        if ( (*verts.front())[longest_axis] > mid[longest_axis] ) {
            partitions[0].push_back( verts.front() );
        }
        else {
            partitions[1].push_back( verts.front() );
        }

        verts.pop_front();
    }
    
    return partitions;
}


// Return a list of nodes
list<vec3dd> BoundingSphere::recurseToDepth(int depth)
{
    list<vec3dd> vertices;
    
    if (leaf || depth <= 0) {
        vertices.push_back(center);
    }
    else {
        depth--;
        vertices = leftSubTree->recurseToDepth(depth);        
        list<vec3dd> rst = rightSubTree->recurseToDepth(depth);    
        vertices.splice(vertices.end(), rst);
    }
    return vertices;
}
