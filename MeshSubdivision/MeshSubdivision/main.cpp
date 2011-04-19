//
//  main.cpp
//  MeshSubdivision
//
//  Modified by Andy Niccolai on 4/12/11.
//  Copyright 2011 Colorado School of Mines. All rights reserved.
//


#ifdef __APPLE__
#	include <OpenGL/gl.h>
#	include <OpenGL/glu.h>
#	include <GLUT/glut.h>
#else
#	include <GL/gl.h>
#	include <GL/glu.h>
#	include <GL/glut.h>
#endif
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include "vec3d.h"
#include "mat4x4.h"

using namespace std;


/* --------------------------------------------- */

const char* WINDOW_TITLE = "Mesh Subdivision Project - Andy Niccolai";

// mesh data

int triangles,vertices;
vec3dd *v;   // vertex table
vec3di *t;   // triangle table
vec3dd *n;   // triangle normals
vec3dd bbmin,bbmax;  // corners of the bounding box

int* vertex_degrees;            // degree of verticies
vector<int>* incidence_table;   // [vertex][triangle]   => triangles incident to every vertex
vector<int>* adjacency_table;   // [triangle][triangle] => 3 triangles adjacent to each triangle

typedef vector<int>::iterator int_it;

// reading a mesh

void read_mesh ( ifstream &ifs )
{
    int i;
    ifs >> triangles >> vertices;
    v = new vec3dd[vertices];
    t = new vec3di[triangles];
    for ( i=0; i<triangles; i++ )
        ifs >> t[i][0] >> t[i][1] >> t[i][2];
    ifs >> v[0][0] >> v[0][1] >> v[0][2];
    bbmin = bbmax = v[0];
    for ( i=1; i<vertices; i++ )
    {
        ifs >> v[i][0] >> v[i][1] >> v[i][2];
        bbmin &= v[i];
        bbmax |= v[i];
    }
    n = new vec3dd[triangles];
    for ( i=0; i<triangles; i++ )
        n[i] = (v[t[i][1]]-v[t[i][0]])^(v[t[i][2]]-v[t[i][0]]);
}


void find_vertex_degrees()
{
    vertex_degrees = new int[vertices];
    
    for (int i=0; i<triangles; i++) {
        for (int j=0; j<3; j++) {
            vertex_degrees[t[i][j]]++;
        }
    }
}


void find_incident_triangles()
{
    incidence_table = new vector<int>[vertices];
    
    for (int i=0; i<triangles; i++) {
        for (int j=0; j<3; j++) {
            incidence_table[t[i][j]].push_back(i);
        }
    }
}


vector<int> adjacent_to(int tri)
{
    map<int, bool> occurence;
    vector<int> adjacent_triangles;
    
    for (int i=0; i<3; i++) {
        for (int_it it=incidence_table[t[tri][i]].begin(); it != incidence_table[t[tri][i]].end(); it++) {            
            if ( (occurence[*it]) && (*it != tri) ) {
                adjacent_triangles.push_back(*it);
            }
            else {
                occurence[*it] = true;
            }
        }
    }
    
    return adjacent_triangles;
}


void find_adjacent_triangles()
{
    adjacency_table = new vector<int>[triangles];
    
    for (int i=0; i<triangles; i++) {
        adjacency_table[i] = adjacent_to(i);
    }   
}



int** edge_table;

int name_edge(int a, int b)
{
    if (a > b) {
        return a * 100 + b;
    }
    else {
        return b * 100 + a;
    }
}


void label_edges()
{
    edge_table = new int*[triangles];
    
    // Initalize the table to negative 1
    for (int i=0; i<triangles; i++) {
        edge_table[i] = new int[3];
        for (int j=0; j<3; j++) {
            edge_table[i][j] = -1;
        }        
    }
    
    int count = 0;
    
    for (int i=0; i<triangles; i++) {
        for (int j=0; j<3; j++) {
            if (edge_table[i][j] == -1) {
                
                edge_table[i][j] = count;
                
                int opp_tri = adjacency_table[i][j];
                
                for (int k=0; k<3; k++) {
                    if (adjacency_table[opp_tri][k] == i) {
                        edge_table[opp_tri][k] = count;
                    }
                }
                count++;
            }
        }
    }
    
    printf("\ncount: %i\n", count);
    printf("triangles: %i\n", triangles);
}


void init_mesh()
{
    find_vertex_degrees();
    find_incident_triangles();
    find_adjacent_triangles();
    label_edges();
}


/* --------------------------------------------- */



// global variables -- used for communication between callback functions

// window ID
int wid;

// default (initial) window size
#define VPD_DEFAULT 800

// size of the window 
int width = VPD_DEFAULT;
int height = VPD_DEFAULT;


bool is_left_mouse_button_down = false;
bool is_middle_mouse_button_down = false;

GLfloat zoom;   // this one controls the field of view

mat4x4d get_rotation();  // get the rotation matrix to use

/* --------------------------------------------- */

GLvoid set_material_properties ( GLfloat r, GLfloat g, GLfloat b )
{
    // this is a sample function that sets the material properties
    
    GLfloat mat_specular[4] = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat mat_ambient_and_diffuse[4] = { 0.5, 0.5, 0.5, 1.0 };
    GLfloat mat_shininess[1] = { 0.0 };
    
    mat_specular[0] = mat_ambient_and_diffuse[0] = r;
    mat_specular[1] = mat_ambient_and_diffuse[1] = g;
    mat_specular[2] = mat_ambient_and_diffuse[2] = b;
    
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_ambient_and_diffuse);
}

const int TARGET = 0;

/* --------------------------------------------- */
bool is_adj(int tri)
{    
    for (int_it it=adjacency_table[TARGET].begin(); it != adjacency_table[TARGET].end(); it++)
        if (*it == tri)
            return true;
           
    return false;
}

void draw_scene ( )
{
    int i;
    
    set_material_properties(.9,.9,.9);    // draw white/greyish
    
    // normalizing transform
    
    glMatrixMode(GL_MODELVIEW);
    double s = (bbmax-bbmin).max();
    vec3dd mc = -0.5*(bbmax+bbmin);
    glScalef(2/s,2/s,2/s);
    glTranslated(mc[0],mc[1],mc[2]);
    
    
    glBegin(GL_TRIANGLES);
    for ( i=0; i<triangles; i++ )
    {
        if (i == TARGET) {
            set_material_properties(0,0.5,1);   // Blue-green!
        }
        else if (is_adj(i)) {
            set_material_properties(0,0,1);     // Blue!
        }
        else {
            set_material_properties(1,1,1);     // White!
        }
            
        
        glNormal3f(n[i][0],n[i][1],n[i][2]);
        glVertex3f(v[t[i][0]][0],v[t[i][0]][1],v[t[i][0]][2]);
        glVertex3f(v[t[i][1]][0],v[t[i][1]][1],v[t[i][1]][2]);
        glVertex3f(v[t[i][2]][0],v[t[i][2]][1],v[t[i][2]][2]);
    }
    glEnd();
}

/* --------------------------------------------- */

/* draw the scene */

GLvoid draw()
{
    // ensure we're drawing to the correct GLUT window 
    glutSetWindow(wid);
    
    // clear color buffer to white 
    glClearColor(1.0, 1.0, 1.0, 1.0);
    
    // clear the buffers 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    {
        // in this block, we initialize light source #0
        GLfloat light_ambient[] = { .1, .1, .1, 1.0 };
        GLfloat light_diffuse[] = { .7, .7, .7, 1.0 };
        GLfloat light_specular[] = { 0, 0, 0, 1.0 };
        GLfloat light_position[] = { -5.0, 2.0, 0.0, 1.0 };
        
        // set light intensities and attenuation coefficients
        glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
        glLightf(GL_LIGHT0,GL_CONSTANT_ATTENUATION,1.0);  // no 
        glLightf(GL_LIGHT0,GL_LINEAR_ATTENUATION,0.0);    // attenuation
        glLightf(GL_LIGHT0,GL_QUADRATIC_ATTENUATION,0.0); // 1/(0*d^2+0*d+1)=1 attenuation factor
        
        // modelview matrix is applied to the light source location
        // we want to keep light position fixed here -- therefore, we 
        // set the modelview matrix to Identity before the glLight* call below
        
        glMatrixMode(GL_MODELVIEW);  // operate on modelview matrix
        glLoadIdentity();            // load identity to current matrix
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    }
    
    // turn back-face culling on 
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // enable illumination and light #0
    glEnable(GL_LIGHTING);  
    glEnable(GL_LIGHT0);
    
    // automatically scale normals to unit length after transformation
    glEnable(GL_NORMALIZE);
    
    // Enable depth test  
    glEnable(GL_DEPTH_TEST);
    
    // set the projection matrix 
    glMatrixMode(GL_PROJECTION);  // operate on projection matrix
    glLoadIdentity();
    // this places camera at the origin and directs it toward (0,0,-infinity)
    // see man gluPerspective for the meaning of the parameters
    // I have no idea how I came up with the field of view :)
    gluPerspective(89.98*(atan(zoom-2)+M_PI/2)/M_PI+0.01,1.0,15.0,25.0);
    
    // build the modelview matrix
    glMatrixMode(GL_MODELVIEW);  // operate on modelview matrix
    glLoadIdentity();
    
    // note that each transformation call multiplies the current matrix [here:modelview]
    //  ON THE RIGHT
    //  at this point, modelview matrix = identity
    
    glTranslatef(0,0,-20);   // move `forward' with respect to the camera
    
    // at this point, modelview matrix = T [translation by (0,0,-20)]
    
    mat4x4d R = get_rotation();
    glMultMatrixd(R.pointer());   // this applies a rotation R that is computed from the trackball UI
    
    // at this point, modelview matrix = T*R
    //  this means rotation is applied first to every vertex, then translation
    // note the order is opposite to the order of transformation calls in the code!
    
    // draw the scene
    draw_scene();
    
    // flush the pipeline -- make sure things will eventually get drawn
    glFlush();
    
    // swap the buffers
    // we really are drawing to an invisible buffer -- the call below is going to 
    // make it visible so that we don't see the drawing process and are in sync
    // with the monitor's refresh rate
    glutSwapBuffers();
}

/* --------------------------------------------- */


// variables used for computing rotation
vec3df point_on_trackball_below_cursor(0,0,0);   // tracks point on trackball below mouse cursor
vec3df last_down(0,0,0);                         // point on trackball where the button went down
mat4x4d finished_rotation = Identity<double>();  // superposition of all finished rotations
// finishing a rotation == releasing the left mouse button
// variable used to update zoom factor 
GLint mprev[2];   // coordinates of last [mouse movement with middle button down] event

/* ************ */

// rotation being specified by the user
// if left mouse button is down, the return value is Id

mat4x4d get_current_rotation()
{
    if (point_on_trackball_below_cursor==last_down)
        return Identity<double>();
    else
    {
        vec3d<GLfloat> axis = last_down^point_on_trackball_below_cursor;
        axis.normalize();
        return Rotation<double>(axis[0],axis[1],axis[2],-acos(last_down*point_on_trackball_below_cursor));
    }
}

/* ************ */

// rotation to be applied as a part of the modelview matrix

mat4x4d get_rotation()
{
    return finished_rotation*get_current_rotation();
}

/* ************ */

// computes the point on the trackball below the pixel (x,y)
// assumes trackball is a unit ball centered at the origin and that the 
// trackball is projected in parallel fashion

vec3d<GLfloat> scr23d ( GLint x, GLint y )
{
    float xx = 2*((float)x)/width-1;
    float yy = 1-2*((float)y)/height;
    float x2y2 = 1-xx*xx-yy*yy;
    if (x2y2<0)
    {
        double l = sqrt(xx*xx+yy*yy);
        xx = xx/l;
        yy = yy/l;
        x2y2 = 0;
        
    }
    if (x2y2<0)
        x2y2 = 0;
    float z = sqrt(x2y2);
    return vec3d<float>(xx,yy,z);      
}

/* ************ */

// function called when a mouse button is pressed or released

GLvoid mouse_button(GLint btn, GLint state, GLint x, GLint y)
{
    switch (btn)
    {
            
        case GLUT_LEFT_BUTTON:
            // left button goes down or up
            switch (state)
        {
            case GLUT_DOWN:
                is_left_mouse_button_down = true;
                last_down = scr23d(x,y);
                break;
            case GLUT_UP:
                is_left_mouse_button_down = false;
                point_on_trackball_below_cursor = scr23d(x,y);
                finished_rotation = get_rotation();
                last_down = point_on_trackball_below_cursor = vec3d<GLfloat>(0,0,0);
                glutPostRedisplay();
                break;
        }
            break;
            
        case GLUT_MIDDLE_BUTTON:
            // middle button goes down or up
            switch (state)
        {
            case GLUT_DOWN:
                is_middle_mouse_button_down = true;
                mprev[0] = x;
                mprev[1] = y;
                break;
            case GLUT_UP:
                is_middle_mouse_button_down = false;
                zoom += 8*(y-mprev[1])/(float)height;
                break;
        }
            break;
            
    }
    
}

/* --------------------------------------------- */

// function called when mouse is moving with a button down

GLvoid button_motion(GLint x, GLint y)
{
    if (is_left_mouse_button_down)
    {
        // left button is down -- update the point on trackball `below' the mouse cursor
        point_on_trackball_below_cursor = scr23d(x,y);
    }
    
    if (is_middle_mouse_button_down)
    {
        // middle button is down - update zoom factor (related to field of view - see draw()
        // also, save the 
        zoom += 8*(y-mprev[1])/(float)height;
        mprev[0] = x;
        mprev[1] = y;
    }
    glutPostRedisplay(); // add display event to queue 
    return;
}

/* --------------------------------------------- */

/* handle keyboard events; here, just exit if ESC is hit */

GLvoid keyboard(GLubyte key, GLint x, GLint y)
{
    switch(key)
    {
        case 27:  /* ESC */
            exit(0);
            break;
            
        default:  
            break;
    }
}

/* --------------------------------------------- */

/* handle resizing the glut window */

GLvoid reshape(GLint vpw, GLint vph)
{
    glutSetWindow(wid);
    width = vpw;
    height = vph;
    glViewport(0, 0, width, height);
    glutReshapeWindow(width, height);
    
    glutPostRedisplay();   // add display event to queue
}

/* --------------------------------------------- */

GLint main(int argc, char **argv)
{  
    // need this call to initialize glut/GL -- don't execute any OpenGL code before this call!
    glutInit(&argc,argv);
    
    // size and placement hints to the window system
    glutInitWindowSize(width,height);
    glutInitWindowPosition(10,10);
    
    // double buffered, RGB color mode, use depth buffer 
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    
    // create a GLUT window (not drawn until glutMainLoop() is entered)
    // wid is the window ID
    wid = glutCreateWindow(WINDOW_TITLE);    
    
    // time to register callbacks 
    
    // window size changes 
    glutReshapeFunc(reshape);
    
    // keypress handling when the current window has input focus 
    glutKeyboardFunc(keyboard);
    
    // mouse event handling 
    glutMouseFunc(mouse_button);           // button press/release
    glutMotionFunc(button_motion);         // mouse motion w/ button down
    
    // function to draw contents of our window -- 
    //  this is where most of your work will focus!
    glutDisplayFunc(draw);
    
    // read the input mesh
    if (argc<2)
    {
        cout << "Use mesh name as the command line argument" << endl;
        return 0;
    }
    ifstream ifs(argv[1]);
    if (!ifs)
    {
        cout << "Can't open " << argv[1] << endl;
        return 0;
    }
    read_mesh(ifs);
    
    init_mesh();
    
    // this is the event loop entry:
    // take event off the queue, call the handler, repeat
    glutMainLoop();
    
    return 0;
}
