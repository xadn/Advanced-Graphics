//
//  main.cpp
//  QSplat
//
//  Created by aniccola on 5/4/11.
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
#include <list>
#include <map>
#include "vec3d.h"
#include "mat4x4.h"
#include "Stopwatch.h"
#include "BoundingSphere.h"

using namespace std;


unsigned int recursion_depth = 12; // 12
unsigned int DEPTH_INCREMENT = 2;

enum RENDER_MODES {QSPLAT, LEAVES, SMOOTH, MESH};

RENDER_MODES render_mode = QSPLAT;


const double SCALE_FACTOR = 75;

double scale_factor;

/* --------------------------------------------- */

// mesh data

unsigned int triangles,vertices;
vector<Point> v;   // vertex table
vector<vec3di> t;
vector<vec3dd> n;   // triangle normals
vec3dd bbmin,bbmax;  // corners of the bounding box
BoundingSphere* sphere_tree;


// reading a mesh

double length(vec3dd vert)
{
    return sqrt(vert[0]*vert[0]+vert[1]*vert[1]+vert[2]*vert[2]);
}

void read_mesh ( ifstream &ifs )
{
    printf("reading mesh from file...\n");
    
    ifs >> triangles >> vertices;
    v.resize(vertices);
    t.resize(triangles);
    
    for (long i=0; i<triangles; i++ )
        ifs >> t[i][0] >> t[i][1] >> t[i][2];
    ifs >> v[0][0] >> v[0][1] >> v[0][2];
    bbmin = bbmax = v[0];
    for (long i=1; i<vertices; i++ )
    {
        ifs >> v[i][0] >> v[i][1] >> v[i][2];
        bbmin &= v[i];
        bbmax |= v[i];
    }
    
    printf("calculating triangle normals...\n");
    
    n.resize(triangles);
    for (long i=0; i<triangles; i++ )
        n[i] = (v[t[i][1]]-v[t[i][0]])^(v[t[i][2]]-v[t[i][0]]);
}


void calc_vertex_normals_and_size()
{
    vector<list<long> > incidence_table(vertices);    
    vector<double> area(triangles);
    
    // Build the table of triangles incident to each vertex
    // Calculate the area of each triangle at the same time
    // Area is 1/2(AB x AC)
    printf("building incidence table...\n");
    
    for (long i=0; i<triangles; i++) {
        for (int j=0; j<3; j++) {
            t[i][j];
            incidence_table[t[i][j]].push_back(i);
        }
        area[i] = 0.5*length(v[t[i][0]]^v[t[i][1]]);
    }
    
    // Calculate the normal for each vertex from its incident triangles
    // The normal is the area weighted sum of the normals of the incident triangles
    printf("calculating vertex normals...\n");
    
    for (long i=0; i<vertices; i++)
    {
        v[i].normal = vec3dd(0,0,0);
        for (list<long>::iterator it = incidence_table[i].begin(); it != incidence_table[i].end(); it++)
        {
            v[i].normal += area[*it] * n[*it];
        }
        //v[i].normal = n[incidence_table[i].front()];
        v[i].normal.normalize();
    }
    
    // Calculate the size of each leaf splat from its incident vertices
    printf("calculating size of leaf nodes...\n");
    
    for (long i=0; i<vertices; i++)
    {
        for (list<long>::iterator it = incidence_table[i].begin(); it != incidence_table[i].end(); it++)
        {
            // Iterate over each vertex in the triangle
            // Calculate the distance from it to the vertex in question
            // 1/2 the max distance is the size of the vertex
            double max = 0;
            for (int j=0; j<3; j++)
            {
                double len = length( v[t[*it][j]] - v[i] );
                if (len > max)
                    max = len;
            }
            v[i].size = 0.5*max;
        }
    }
}




void build_sphere_tree()
{
    calc_vertex_normals_and_size();
    
    vert_ls verts;
    
    printf("creating pointers array...\n");
    for (int i=0; i<vertices; i++)
    {
        verts.push_back(&v[i]);
    }    
    
    printf("building bounding spheres...\n");
    sphere_tree = new BoundingSphere(verts);
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

/* --------------------------------------------- */

vector<Point> fastSplats;
double maxSplatSize;

typedef vector<Point>::iterator point_it;

vector<point_it> splatParts;
vector<double> splatSizes;

bool testSize(Point* a, Point* b)
{
    if (a->size > b->size)
        return true;
    return false;
}

void partitionSizes(point_it start, point_it end, int depth)
{
    if (start != end)
    {
        depth--;
        double mid = (start->size + (end-1)->size) / 2.0;
        
        if (depth == 0)
        {
            splatParts.push_back(start);
            splatSizes.push_back(mid * scale_factor);
        }
        else
        {
            point_it midpoint = start;           
            while (midpoint->size > mid) midpoint++;
            partitionSizes(start, midpoint, depth);
            partitionSizes(midpoint, end, depth);
        }
    }        
}

void walk_tree()
{
    static Stopwatch timer;
    timer.start();
    
    scale_factor = sqrt(2.0*pow(((double)width/(bbmax-bbmin).max()), 2.0));
    
    vert_ls splats = sphere_tree->recurseToDepth(recursion_depth);
    
    splats.sort(testSize);
    
    maxSplatSize = scale_factor * (splats.front()->size - splats.back()-> size)/2.0;
    
    fastSplats.clear();    
    fastSplats.reserve(splats.size());
    
    for (vert_it it = splats.begin(); it != splats.end(); it++)
    {
        fastSplats.push_back(**it);
    }

    splatParts.clear();
    splatSizes.clear();
    
    partitionSizes(fastSplats.begin(), fastSplats.end(), 5);
    
    splatParts.push_back(fastSplats.end());
    
    cout << splatSizes.size() << endl;
    
    timer.stop();
    printf("Recursed to depth %u in %f seconds\n", recursion_depth, timer.time());
    printf("Displaying %lu splats, with %lu sizes\n", splats.size(), splatSizes.size() );
    timer.reset();
}


/* --------------------------------------------- */


void draw_mesh()
{
    set_material_properties(.9,.9,.9);
    
    glBegin(GL_TRIANGLES);
    for (int i=0; i<triangles; i++ )
    {
        glNormal3f(n[i][0],n[i][1],n[i][2]);
        for (int j=0; j<3; j++)
        {
            glVertex3f(v[t[i][j]][0],v[t[i][j]][1],v[t[i][j]][2]); 
        }
    }
    glEnd();
}

void draw_mesh_smooth()
{
    set_material_properties(.9,.9,.9);
    
    glBegin(GL_TRIANGLES);
    for (int i=0; i<triangles; i++ )
    {
        for (int j=0; j<3; j++)
        {            
            glNormal3f(v[t[i][j]].normal[0], v[t[i][j]].normal[1], v[t[i][j]].normal[2]);
            glVertex3f(v[t[i][j]][0],v[t[i][j]][1],v[t[i][j]][2]);            
        }
    }
    
    glEnd();
}


void draw_splats()
{
    set_material_properties(.9,.9,.9);
    
    for (int i=0; i < splatParts.size()-1; i++)
    {
        glPointSize(splatSizes[i]);
        glBegin(GL_POINTS);
        for (point_it it=splatParts[i]; it != splatParts[i+1]; it++)
        {            
            glNormal3f(it->normal[0], it->normal[1], it->normal[2]);
            glVertex3f((*it)[0], (*it)[1], (*it)[2]);
        }
        glEnd();
    }
}

void draw_splats_colors()
{
    set_material_properties(.9,.9,.9);
    
    for (int i=0; i < splatParts.size()-1; i++)
    {
        glPointSize(splatSizes[i]);
        glBegin(GL_POINTS);
        for (point_it it=splatParts[i]; it != splatParts[i+1]; it++)
        {
            set_material_properties(it->color[0], it->color[1], it->color[2]);
            glNormal3f(it->normal[0], it->normal[1], it->normal[2]);
            glVertex3f((*it)[0], (*it)[1], (*it)[2]);
        }
        glEnd();
    }
}


void draw_scene()
{        
    // normalizing transform
    
    glMatrixMode(GL_MODELVIEW);
    double s = (bbmax-bbmin).max();
    
    scale_factor = sqrt(2.0*pow(((double)width/s), 2.0));
    
    vec3dd mc = -0.5*(bbmax+bbmin);
    glScalef(2/s,2/s,2/s);
    glTranslated(mc[0],mc[1],mc[2]);
    
    switch (render_mode) {
        case QSPLAT:
            draw_splats();
            break;
        case LEAVES:
            draw_splats_colors();
            break;
        case SMOOTH:
            draw_mesh_smooth();
            break;
        case MESH:
            draw_mesh();
            break;
    }
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
            
        case 'x':
            recursion_depth += DEPTH_INCREMENT;
            break;
            
        case 'z':
            recursion_depth -= DEPTH_INCREMENT;            
            break;
            
        case '=':
            zoom -= 1;
            break;
            
        case '-':
            zoom += 1;
            break;
            
        default:  
            break;
    }
    walk_tree();
    glutPostRedisplay();
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

void menu_callback(int button)
{
    switch (button) {
        case 1:
            render_mode = QSPLAT;
            break;
        case 2:
            render_mode = LEAVES;
            break;         
        case 3:
            render_mode = SMOOTH;
            break;
        case 4:
            render_mode = MESH;
            break;
    }
    glutPostRedisplay();
}


void create_menu()
{
    
    glutCreateMenu(menu_callback);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    glutAddMenuEntry("Regular splats", 1);
    glutAddMenuEntry("Highlight leaves", 2);
    glutAddMenuEntry("Smooth shaded", 3);
    glutAddMenuEntry("Flat shaded", 4);
    
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
    wid = glutCreateWindow("QSplat Implementation");    
    
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
    
    Stopwatch timer;
    
    timer.start();
    
    read_mesh(ifs);    
    build_sphere_tree();
    
    timer.stop();
    
    printf("\nProcessed the mesh in %f seconds\n\n", timer.time() );
    
    walk_tree();
    
    create_menu();
    // this is the event loop entry:
    // take event off the queue, call the handler, repeat
    glutMainLoop();
    
    return 0;
}
