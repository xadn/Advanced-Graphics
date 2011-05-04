//
//  main.cpp
//  MeshSubdivision
//
//  Modified by Andy Niccolai on 5/3/11.
//  2011 Colorado School of Mines.
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
#include "vec3d.h"
#include "mat4x4.h"
#include "Stopwatch.h"

using namespace std;


/* --------------------------------------------- */

const char* WINDOW_TITLE = "Mesh Subdivision Project - Andy Niccolai";

// Change this to true to display the vertices!!!
bool show_vertices = false;
//

int iteration_count;
Stopwatch timer;

// mesh data

typedef vector<int>::iterator int_it; // stl iterator for a vector of ints

int vertices;           // number of vertices
int triangles;          // number of triangles
int edges;              // number of edges
vec3dd *v;              // vertex table
vec3di *t;              // triangle table
vec3dd *n;              // triangle normals
vec3dd bbmin,bbmax;     // corners of the bounding box

vector<int>* incidence_table;   // triangles incident to every vertex, [vertex][triangle_index] = triangle
vec3di* adjacency_table;        // 3 triangles adjacent to each triangle, [triangle][opp_edge] = triangle 
vec3di* edge_table;             // edges with unique labels, [triangle][opp_edge] => edge
vec3dd** edge_list;             // stores the verticies of each edge to create new points


// reading a mesh

void calc_normals()
{
    n = new vec3dd[triangles];
    for (int i=0; i<triangles; i++ )
        n[i] = (v[t[i][1]]-v[t[i][0]])^(v[t[i][2]]-v[t[i][0]]);
}

void read_mesh ( ifstream &ifs )
{
    iteration_count = 0;
    
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
    calc_normals();
    
    printf("Input mesh:\n");
    printf("vertices:  %i\n", vertices);
    printf("triangles: %i\n\n", triangles);
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

int next_point(int i)
{
    return (i+1)%3;
}

vec3di adjacent_to(int tri)
{
    vec3di adjacent_triangles;
    
    // find the triangle opposite to each vertex
    for (int i=0; i<3; i++) {        
        vector<int> A = incidence_table[t[tri][next_point(i)]];
        vector<int> B = incidence_table[t[tri][next_point(i+1)]];
        map<int, bool> occurence;
        
        // Mark the triangles incident to A
        for (int_it it=A.begin(); it != A.end(); it++) {            
            occurence[*it] = true;
        }
        
        // Search triangles incident to B for a mark
        for (int_it it=B.begin(); it != B.end(); it++) {            
            if ( (occurence[*it]) && (*it != tri) ) {
                adjacent_triangles[i] = *it;
            }
        }        
    }    
    return adjacent_triangles;
}


// 3 triangles adjacent to each triangle
// [triangle][opp_edge] = triangle 
void find_adjacent_triangles()
{
    adjacency_table = new vec3di[triangles];
    
    for (int i=0; i<triangles; i++) {
        adjacency_table[i] = adjacent_to(i);
    }   
}


void label_edges()
{
    edges = 3*triangles/2;
    edge_table = new vec3di[triangles];
    edge_list = new vec3dd*[edges];
    
    // Initalize the table to negative 1
    for (int i=0; i<triangles; i++) {
            edge_table[i] = vec3di(-1, -1, -1);
    }
    
    // labels are generated serially
    // should have a range of [0, edges)
    int label = -1;
    
    // loop over each edge (actually the vertex opposite the edge) of each triangle
    for (int i=0; i<triangles; i++) {
        for (int j=0; j<3; j++) {
            if (edge_table[i][j] == -1)
            {                
                label++;
                
                // label this edge
                edge_table[i][j] = label;
                
                // find the other two vertices on the triangle
                int a = next_point(j);
                int b = next_point(j+1);                
                
                // put these vertices in a new array for later
                edge_list[label] = new vec3dd[4];
                edge_list[label][0] = v[t[i][a]];
                edge_list[label][1] = v[t[i][b]];
                edge_list[label][2] = v[t[i][j]];
                
                // find the adjacent triangle
                int opp_tri = adjacency_table[i][j];

                // find the vertex of the opposite triangle which is not on the edge
                for (int k=0; k<3; k++) {
                    if( ( t[opp_tri][k] != t[i][a] ) && ( t[opp_tri][k] != t[i][b] ) )
                    { 
                        // update it with the label
                        edge_table[opp_tri][k] = label;
                        
                        // add this the the array for later
                        edge_list[label][3] = v[t[opp_tri][k]];
                    }
                }
            } // end if
        } // end vertices
    } // end triangless
}


int s_vertices;     // number of vertices after subdivison
int s_triangles;    // number of triangles after subdivision
int old_vertices;   // number of vertices in the previous iteration
vec3dd* s_v;        // vertex table for subdivided surface
vec3di* s_t;        // triangle table for subdivided surface
vec3dd* s_n;        // normals
vec3dd* old_v;      // vertex table

const double D3_ORIG_WEIGHT     = 7.0/16.0;
const double D3_NEIGHBOR_WEIGHT = 3.0/16.0;
const double ORIG_WEIGHT        = 5.0/8.0;
const double NEIGHBOR_WEIGHT    = 3.0/8.0;

#include <list>
vec3dd move_point(int vert)
{    
    vec3dd p(0,0,0);
    vec3dd original = v[vert];
    int degree = 0;
    
    for (int_it it = incidence_table[vert].begin(); it != incidence_table[vert].end(); it++) {
        for (int i=0; i<3; i++) {
            if (t[*it][i] != vert) {
                p += v[t[*it][i]];
                degree++;
            }
        }
    }
    
    // divide by 2 since vertices are double counted
    if (degree/2 == 3) {
        original *= D3_ORIG_WEIGHT;
        p *= D3_NEIGHBOR_WEIGHT/2.0;
    }
    else {
        original *= ORIG_WEIGHT;
        p *= NEIGHBOR_WEIGHT/(double)degree; // dividing by degrees accounts for the double counting
    }
    p += original;

    return p;
}


const double EDGE_WEIGHT    = 3.0/8.0;
const double ADJ_WEIGHT     = 1.0/8.0; 

vec3dd create_point(int edge)
{
    vec3dd* e = edge_list[edge];  
    e[0] *= EDGE_WEIGHT;
    e[1] *= EDGE_WEIGHT;
    e[2] *= ADJ_WEIGHT;
    e[3] *= ADJ_WEIGHT;    
    return e[0]+e[1]+e[2]+e[3];
}


void subdivide()
{
    s_vertices = vertices+edges;    
    s_v = new vec3dd[s_vertices];
    
    // move current verticies
    for (int i=0; i<vertices; i++) {
        s_v[i] = move_point(i);
    }
    
    // add new edges
    for (int i=0; i<edges; i++) {
        s_v[vertices+i] = create_point(i);
        delete[] edge_list[i];
    }
    delete[] edge_list;
    
    printf("vertices: %i\n", s_vertices);
}




void associate_triangles()
{
    s_triangles = triangles*4;
    s_t = new vec3di[s_triangles];
    s_n = new vec3dd[s_triangles];
    
    int count = 0;
    
    for (int i=0; i<triangles; i++) {
        
        int middle_tri = count;
        count++;
        
        for (int j=0; j<3; j++) {            
            s_t[middle_tri][j] = vertices + edge_table[i][j];            
            s_t[count][2] = t[i][j];
            s_t[count][1] = vertices + edge_table[i][next_point(j)];
            s_t[count][0] = vertices + edge_table[i][next_point(j+1)];            
            count++;
        }
    }
    
    printf("triangles: %i\n", s_triangles);
    
}


void swap_mesh()
{
    delete[] old_v;
    old_v = v;
    old_vertices = vertices;
    
    delete[] t;
    delete[] n;
    
    vertices = s_vertices;
    triangles = s_triangles;
    v = s_v;
    t = s_t;
    n = s_n;
    
    calc_normals();
}


/* --------------------------------------------- */


void do_subdivision()
{
    iteration_count++;
    
    cout << "Iteration: " << iteration_count << endl;
    timer.start();
    
    find_incident_triangles();
    find_adjacent_triangles();
    label_edges();
    subdivide();
    associate_triangles();
    timer.stop();
    
    cout << "time: " << timer.time() << endl << endl;
    swap_mesh();
}

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


void render_triangles()
{
    set_material_properties(.9,.9,.9);    // white/greyish

    glBegin(GL_TRIANGLES);
    for (int i=0; i<triangles; i++ )
    {        
        glNormal3f(n[i][0],n[i][1],n[i][2]);
        glVertex3f(v[t[i][0]][0],v[t[i][0]][1],v[t[i][0]][2]);
        glVertex3f(v[t[i][1]][0],v[t[i][1]][1],v[t[i][1]][2]);
        glVertex3f(v[t[i][2]][0],v[t[i][2]][1],v[t[i][2]][2]);
    }
    glEnd();
}


void render_vertices()
{
    glPointSize(2.0);
    
    glBegin(GL_POINTS);
    
    set_material_properties(0,0,0);     // black! original vertices
    for(int i=0; i<old_vertices; i++) {
        glVertex3f(old_v[i][0], old_v[i][1], old_v[i][2]);        
    }
    
    glBegin(GL_POINTS);
    set_material_properties(0,0,1);     // blue! displaced origial vertices
    for(int i=0; i<old_vertices; i++) {
        glVertex3f(v[i][0], v[i][1], v[i][2]);        
    }
    
    
    set_material_properties(0,1,0);     // green! added verticies
    for(int i=old_vertices; i<vertices; i++) {
        glVertex3f(v[i][0], v[i][1], v[i][2]);        
    }
    
    glEnd();
}

void draw_scene ( )
{        
    // normalizing transform
    
    glMatrixMode(GL_MODELVIEW);
    double s = (bbmax-bbmin).max();
    vec3dd mc = -0.5*(bbmax+bbmin);
    glScalef(2/s,2/s,2/s);
    glTranslated(mc[0],mc[1],mc[2]);
    
    render_triangles();
    if (show_vertices) 
        render_vertices();
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

//glutCreateMenu(void (*)(int)):
//Creates an empty menu. The argument is the callback function, which is of the form void myfunc(int value), where value holds the index of the menu item which was selected.
//glutAddMenuEntry(char *name, int value):
//Adds a menu entry to the bottom of the current menu. The character string name is the text to be displayed in the menu entry, and the integer value is passed to your callback procedure to identify the selected item.
//glutAttachMenu(int button):
//Attaches the current menu to the specified mouse button, which is either GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON, or GLUT_RIGHT_BUTTON.

void menu_callback(int button)
{
    switch (button) {
        case 1:
            do_subdivision();
            break;
        case 2:
            show_vertices = !show_vertices;
            break;            
    }
    glutPostRedisplay();
}

void create_menu()
{
    const char* SUB_LABEL = "Subdivide";
    const char* VERT_LABEL = "Show vertices";
    
    glutCreateMenu(&menu_callback);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    glutAddMenuEntry(SUB_LABEL, 1);
    //glutAddMenuEntry(VERT_LABEL, 2);
    
}

/* --------------------------------------------- */

const char* NO_FILE_MESSAGE = "Please specify a file to display.\n";
const char* USAGE_INSTRUCTIONS = "Right-click options:\n\t1: subdivide the model\n";
const char* VERTICES_KEY = "Vertices color key:\n\tblack: vertices of previous iteration\n\tgreen: vertices of current iteration\n";

GLint main(int argc, char **argv)
{  
    if (argc < 1) {
        cout << NO_FILE_MESSAGE;
        exit(0);
    }
    
    cout << USAGE_INSTRUCTIONS << endl;
    //cout << VERTICES_KEY << endl;
    
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
    
    create_menu();
    
    // this is the event loop entry:
    // take event off the queue, call the handler, repeat
    glutMainLoop();
    
    return 0;
}
