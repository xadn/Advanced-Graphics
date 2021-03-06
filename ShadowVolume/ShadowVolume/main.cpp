//
//  main.cpp
//  ShadowVolume
//
//  Modified by Andy Niccolai on 2/10/11.
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
#include "vec3d.h"
#include "mat4x4.h"


using namespace std;


#define SWAP_ROWS_DOUBLE(a, b) { double *_tmp = a; (a)=(b); (b)=_tmp; }
#define SWAP_ROWS_FLOAT(a, b) { float *_tmp = a; (a)=(b); (b)=_tmp; }
#define MAT(m,r,c) (m)[(c)*4+(r)]


void MultiplyMatrixByVector4by4OpenGL_FLOAT(float *resultvector, const float *matrix, const float *pvector)
{
    resultvector[0]=matrix[0]*pvector[0]+matrix[4]*pvector[1]+matrix[8]*pvector[2]+matrix[12]*pvector[3];
    resultvector[1]=matrix[1]*pvector[0]+matrix[5]*pvector[1]+matrix[9]*pvector[2]+matrix[13]*pvector[3];
    resultvector[2]=matrix[2]*pvector[0]+matrix[6]*pvector[1]+matrix[10]*pvector[2]+matrix[14]*pvector[3];
    resultvector[3]=matrix[3]*pvector[0]+matrix[7]*pvector[1]+matrix[11]*pvector[2]+matrix[15]*pvector[3];
}


//This code comes directly from GLU except that it is for float
int glhInvertMatrixf2(float *m, float *out)
{
	float wtmp[4][8];
	float m0, m1, m2, m3, s;
	float *r0, *r1, *r2, *r3;
	r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];
	r0[0] = MAT(m, 0, 0), r0[1] = MAT(m, 0, 1),
	r0[2] = MAT(m, 0, 2), r0[3] = MAT(m, 0, 3),
	r0[4] = 1.0, r0[5] = r0[6] = r0[7] = 0.0,
	r1[0] = MAT(m, 1, 0), r1[1] = MAT(m, 1, 1),
	r1[2] = MAT(m, 1, 2), r1[3] = MAT(m, 1, 3),
	r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0,
	r2[0] = MAT(m, 2, 0), r2[1] = MAT(m, 2, 1),
	r2[2] = MAT(m, 2, 2), r2[3] = MAT(m, 2, 3),
	r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0,
	r3[0] = MAT(m, 3, 0), r3[1] = MAT(m, 3, 1),
	r3[2] = MAT(m, 3, 2), r3[3] = MAT(m, 3, 3),
	r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;
	/* choose pivot - or die */
	if (fabsf(r3[0]) > fabsf(r2[0]))
		SWAP_ROWS_FLOAT(r3, r2);
	if (fabsf(r2[0]) > fabsf(r1[0]))
		SWAP_ROWS_FLOAT(r2, r1);
	if (fabsf(r1[0]) > fabsf(r0[0]))
		SWAP_ROWS_FLOAT(r1, r0);
	if (0.0 == r0[0])
		return 0;
	/* eliminate first variable     */
	m1 = r1[0] / r0[0];
	m2 = r2[0] / r0[0];
	m3 = r3[0] / r0[0];
	s = r0[1];
	r1[1] -= m1 * s;
	r2[1] -= m2 * s;
	r3[1] -= m3 * s;
	s = r0[2];
	r1[2] -= m1 * s;
	r2[2] -= m2 * s;
	r3[2] -= m3 * s;
	s = r0[3];
	r1[3] -= m1 * s;
	r2[3] -= m2 * s;
	r3[3] -= m3 * s;
	s = r0[4];
	if (s != 0.0) {
		r1[4] -= m1 * s;
		r2[4] -= m2 * s;
		r3[4] -= m3 * s;
	}
	s = r0[5];
	if (s != 0.0) {
		r1[5] -= m1 * s;
		r2[5] -= m2 * s;
		r3[5] -= m3 * s;
	}
	s = r0[6];
	if (s != 0.0) {
		r1[6] -= m1 * s;
		r2[6] -= m2 * s;
		r3[6] -= m3 * s;
	}
	s = r0[7];
	if (s != 0.0) {
		r1[7] -= m1 * s;
		r2[7] -= m2 * s;
		r3[7] -= m3 * s;
	}
	/* choose pivot - or die */
	if (fabsf(r3[1]) > fabsf(r2[1]))
		SWAP_ROWS_FLOAT(r3, r2);
	if (fabsf(r2[1]) > fabsf(r1[1]))
		SWAP_ROWS_FLOAT(r2, r1);
	if (0.0 == r1[1])
		return 0;
	/* eliminate second variable */
	m2 = r2[1] / r1[1];
	m3 = r3[1] / r1[1];
	r2[2] -= m2 * r1[2];
	r3[2] -= m3 * r1[2];
	r2[3] -= m2 * r1[3];
	r3[3] -= m3 * r1[3];
	s = r1[4];
	if (0.0 != s) {
		r2[4] -= m2 * s;
		r3[4] -= m3 * s;
	}
	s = r1[5];
	if (0.0 != s) {
		r2[5] -= m2 * s;
		r3[5] -= m3 * s;
	}
	s = r1[6];
	if (0.0 != s) {
		r2[6] -= m2 * s;
		r3[6] -= m3 * s;
	}
	s = r1[7];
	if (0.0 != s) {
		r2[7] -= m2 * s;
		r3[7] -= m3 * s;
	}
	/* choose pivot - or die */
	if (fabsf(r3[2]) > fabsf(r2[2]))
		SWAP_ROWS_FLOAT(r3, r2);
	if (0.0 == r2[2])
		return 0;
	/* eliminate third variable */
	m3 = r3[2] / r2[2];
	r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4],
	r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6], r3[7] -= m3 * r2[7];
	/* last check */
	if (0.0 == r3[3])
		return 0;
	s = 1.0 / r3[3];		/* now back substitute row 3 */
	r3[4] *= s;
	r3[5] *= s;
	r3[6] *= s;
	r3[7] *= s;
	m2 = r2[3];			/* now back substitute row 2 */
	s = 1.0 / r2[2];
	r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),
	r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
	m1 = r1[3];
	r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1,
	r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
	m0 = r0[3];
	r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0,
	r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;
	m1 = r1[2];			/* now back substitute row 1 */
	s = 1.0 / r1[1];
	r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1),
	r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);
	m0 = r0[2];
	r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0,
	r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;
	m0 = r0[1];			/* now back substitute row 0 */
	s = 1.0 / r0[0];
	r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0),
	r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);
	MAT(out, 0, 0) = r0[4];
	MAT(out, 0, 1) = r0[5], MAT(out, 0, 2) = r0[6];
	MAT(out, 0, 3) = r0[7], MAT(out, 1, 0) = r1[4];
	MAT(out, 1, 1) = r1[5], MAT(out, 1, 2) = r1[6];
	MAT(out, 1, 3) = r1[7], MAT(out, 2, 0) = r2[4];
	MAT(out, 2, 1) = r2[5], MAT(out, 2, 2) = r2[6];
	MAT(out, 2, 3) = r2[7], MAT(out, 3, 0) = r3[4];
	MAT(out, 3, 1) = r3[5], MAT(out, 3, 2) = r3[6];
	MAT(out, 3, 3) = r3[7];
	return 1;
}


// Forward declaration of triangle class
class triangle;

// global variables -- used for communication between callback functions

bool firstEvah = true;

mat4x4f rotation_matrix;

vec3df original_light_position( -5.0, 2.0, 5.0 );
vec3df transformed_light_position = original_light_position;

int num_points;
int num_triangles;
vec3df *v_list = NULL;
triangle *tri = NULL;

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
bool update_fov = true;

mat4x4f get_rotation();  // get the rotation matrix to use


// Class to hold each triangle for the model
// Calculates the shadow polygon
class triangle
{   
public:
	
	// Normal vector
    vec3df normal;
	
	// Pointers to each vertex of the triangle
	vec3df *vert[3];
	
	// Shadow quad's verticies not in the triangle
	vec3df shadow_pair[3];
		
	// Return a vertex
	vec3df get(int vertex) 
	{ 
		return *vert[vertex]; 
	}
	
	// Return a coordinate of a vertex
	float get(int vertex, int coord) 
	{ 
		return (*vert[vertex])[coord]; 
	}
	
	// Calculate the normal vector of the triangle
    void calc_normal() 
	{ 
		normal = (get(1) - get(0))^(get(2) - get(0)); 
	}
	
    
	// Determine if the triangle is lit
    bool shadowed()
    {
        if( (( transformed_light_position - get(0) ) * normal) > 0 )
			return false;
		return true;
    }
	
	// Calculate the shadow quads and normals
	void calc_shadow()
	{
		for(int v=0; v<3; v++)
		{
			shadow_pair[v] = get(v) - transformed_light_position;
		}
	}
	
};


// Read a model in from a file
void read_file(char *filename) 
{
    std::ifstream file;
    file.open(filename);
    
    // Read in the array/loop sizes
    file >> num_triangles >> num_points;
    
    // Temp variable for triangles
    int t[num_triangles][3];
	
    // Initalize member to contain the triangles
    tri = new triangle[num_triangles];
	
    // Initalize member to contain the verticies
    v_list = new vec3df[num_points];
    
    // Read in the triangles
    for(int i=0; i<num_triangles; i++)
    {
        file >> t[i][0] >> t[i][1] >> t[i][2];
    }
    
    float tmax = 0;
    
    // Read in the verticies
    for(int i=0; i<num_points; i++)
    {
        float x, y, z;
        file >> x >> y >> z;
        v_list[i] = vec3df(x, y, z);
        
        // Store the farthest out coordinate
        if( v_list[i].max() > tmax )
            tmax = v_list[i].max();
    }
    
    // Multiply by the inverse since vec3d only overloads *
	tmax = 4/tmax;
    
    // Normalize each vertex
    for(int i=0; i<num_points; i++)
    {
        v_list[i] *= tmax;
    }
    
    // Initialize each triangle with pointers to it's verticies
    for(int i=0; i<num_triangles; i++)
    {
		for(int v=0; v<3; v++)
			tri[i].vert[v] = &v_list[ t[i][v] ];
		
        tri[i].calc_normal();
    }
    
    file.close();
}


// Set the material properties
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


void apply_transform()
{
    static GLdouble fov;
    
    // set the projection matrix 
    glMatrixMode(GL_PROJECTION);  // operate on projection matrix

    glLoadIdentity();
    // this places camera at the origin and directs it toward (0,0,-infinity)
    // see man gluPerspective for the meaning of the parameters
    // I have no idea how I came up with the field of view :)
    
    if (update_fov) {
        fov = 89.98*(atan(zoom-2)+M_PI/2)/M_PI+0.01;
    }

    gluPerspective(fov, 1.0, 5.0, 50.0);
    
    // build the modelview matrix
    glMatrixMode(GL_MODELVIEW);  // operate on modelview matrix
    
    glLoadIdentity();
    
    // note that each transformation call multiplies the current matrix [here:modelview]
    //  ON THE RIGHT
    //  at this point, modelview matrix = identity
    
    if(firstEvah) {
        glTranslatef(0,0,-20);   // Move the model 'forward' with respect to the camera
    }
    
    // at this point, modelview matrix = T [translation by (0,0,-20)]
   // mat4x4d R = get_rotation();
    glMultMatrixf(rotation_matrix.pointer());   // this applies a rotation R that is computed from the trackball UI
    
    // at this point, modelview matrix = T*R
    //  this means rotation is applied first to every vertex, then translation
    // note the order is opposite to the order of transformation calls in the code!	
}


// Draw the triangles of the models
void draw_model()
{    		
    glBegin(GL_TRIANGLES);
    
	// For each triangle
    for(int i=0; i<num_triangles; i++)
    {		
		// Set the normal
		glNormal3f( tri[i].normal[0], tri[i].normal[1], tri[i].normal[2] );
		
		// Set each vertex in the triangle
		for(int v=0; v<3; v++) 
        {
			glVertex3f( tri[i].get(v, 0), tri[i].get(v, 1), tri[i].get(v, 2) );
        }
    }
    
    glEnd();
}


// Draw a shadow polygon for each triangle in shadow
void draw_shadow()
{    	
	glBegin(GL_QUADS);
	
	// For each triangle
    for(int i=0; i<num_triangles; i++)
    {
		// If the triangle is in shadow 
		if( tri[i].shadowed() )
        {						
			// Tell the triangle to calculate it's shadow
            tri[i].calc_shadow();

			// For each face of the shadow
			for(int v=0; v<3; v++)
			{			
				// First vertex (from the triangle)
				glVertex3f( tri[i].get(v, 0), tri[i].get(v, 1), tri[i].get(v, 2) );
				
				// Second vertex (next vertex from the triangle)
				glVertex3f( tri[i].get((v+1)%3, 0), tri[i].get((v+1)%3, 1), tri[i].get((v+1)%3, 2) );
				
				// Third vertex (calculated from the second vertex)
				glVertex4f(tri[i].shadow_pair[(v+1)%3][0], tri[i].shadow_pair[(v+1)%3][1], tri[i].shadow_pair[(v+1)%3][2], 0);
				
				// Forth vertex (calculated from the first vertex)
				glVertex4f(tri[i].shadow_pair[v][0], tri[i].shadow_pair[v][1], tri[i].shadow_pair[v][2], 0);
			}
		}
    }
    
    glEnd();
}


// Transform the light coodinates
void transform_light()
{	
    float inverted_matrix[16];
    
	// Find the inverse transformation of the rotation matrix
	glhInvertMatrixf2(rotation_matrix.pointer(), inverted_matrix);
	
	// Apply the transformation to the light coordinates
	MultiplyMatrixByVector4by4OpenGL_FLOAT(transformed_light_position.pointer(), inverted_matrix, original_light_position.pointer());
}


// Draw the light source
void init_light()
{
	// Operate on modelview matrix
	glMatrixMode(GL_MODELVIEW);  
	
	// Comment out to "fixate" the light
	glLoadIdentity();            
	
	// Load the location of the light source
	glLightfv(GL_LIGHT0, GL_POSITION, original_light_position.pointer());	
	
	// Initialize light source
	GLfloat light_ambient[] =  { .1, .1, .1, 1.0 };
	GLfloat light_diffuse[] = { .7, .7, .7, 1.0 };
	GLfloat light_specular[] = { 0, 0, 0, 1.0 };	
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	
	// Attenuation coefficients 
	glLightf(GL_LIGHT0,GL_CONSTANT_ATTENUATION,1.0);  // no 
	glLightf(GL_LIGHT0,GL_LINEAR_ATTENUATION,0.0);    // attenuation
	glLightf(GL_LIGHT0,GL_QUADRATIC_ATTENUATION,0.0); // 1/(0*d^2+0*d+1)=1 attenuation factor		
}


void initalize()
{    
    // set model color to grey
    set_material_properties(1,1,1);	
    
    // set the lighting properties
    init_light();
    
    // clear color buffer to white 
    glClearColor(1.0, 1.0, 1.0, 1.0);
    
    glEnable(GL_LIGHTING); 
    glCullFace(GL_BACK);
    glEnable(GL_NORMALIZE);
    glEnable(GL_DEPTH_TEST);	
    glDepthFunc(GL_LEQUAL);
}


// Draw the scene
GLvoid draw()
{	
    // ensure we're drawing to the correct GLUT window 
    glutSetWindow(wid);
    
    // Get the trackball rotation for this pass 
    rotation_matrix = get_rotation();
    
    // Recalculate the position of the light for the shadow polygons
    transform_light();

    apply_transform();
    
    // clear the color buffers and stencil buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    
    // FIRST PASS, draw the model without light
    
	glDisable(GL_LIGHT0);
	
	draw_model();
	
    
    // SECOND PASS
    
	// disable buffers
	glColorMask(0, 0, 0, 0);
	glDepthMask(0);        
    
    // disable culling since both surfaces are needed to +/- the buffer
    glDisable(GL_CULL_FACE);
	
	// set the depth and stencil functions
	
	glStencilFunc(GL_ALWAYS, 0, 0);
    glEnable(GL_STENCIL_TEST);
    
	// increment the stencil buffer on the front faces
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
    
    // and decrement the stencil buffer on the back faces
    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR_WRAP);

    // draw the shadow polygon 
	draw_shadow();
    
    // switch culling back on to render the final scene
	glEnable(GL_CULL_FACE);
    
    // set the stencil buffer to clean up artifacts
	glStencilFunc(GL_EQUAL, 0, -1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	
    // turn on the lights
	glEnable(GL_LIGHT0);
    
    // enable buffers
	glColorMask(1, 1, 1, 1);
	glDepthMask(1);
    
	// draw the scene in light
    draw_model();
	//draw_shadow();
    
    // flush the pipeline -- make sure things will eventually get drawn
    glFlush();
    
    // swap the buffers
    glutSwapBuffers();
}


// variables used for computing rotation

vec3df point_on_trackball_below_cursor(0,0,0);   // tracks point on trackball below mouse cursor
vec3df last_down(0,0,0);                         // point on trackball where the button went down
mat4x4f finished_rotation = Identity<float>();  // superposition of all finished rotations

// finishing a rotation == releasing the left mouse button
// variable used to update zoom factor 
GLint mprev[2];   // coordinates of last [mouse movement with middle button down] event


// rotation being specified by the user
// if left mouse button is down, the return value is Id
mat4x4f get_current_rotation()
{
    if (point_on_trackball_below_cursor==last_down)
        return Identity<float>();
    else
    {
        vec3d<GLfloat> axis = last_down^point_on_trackball_below_cursor;
        axis.normalize();
        return Rotation<float>(axis[0],axis[1],axis[2],-acos(last_down*point_on_trackball_below_cursor));
    }
}


// rotation to be applied as a part of the modelview matrix
mat4x4f get_rotation()
{
    return finished_rotation*get_current_rotation();
}


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
                update_fov = true;
                break;
        }
            break;
            
    }
    
}


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
        update_fov = true;
        mprev[0] = x;
        mprev[1] = y;
    }
    glutPostRedisplay(); // add display event to queue 
    return;
}


// handle keyboard events; here, just exit if ESC is hit
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


// handle resizing the glut window
GLvoid reshape(GLint vpw, GLint vph)
{
    glutSetWindow(wid);
    width = vpw;
    height = vph;
    glViewport(0, 0, width, height);
    glutReshapeWindow(width, height);
    
    glutPostRedisplay();   // add display event to queue
}


// Pass functions to OpenGL's main loop
GLint main(int argc, char **argv)
{  
	if( argc < 2 )
	{
		cout << "Usage: ./main ModelFile.t\n";
		exit(EXIT_FAILURE);
	}
	
    // need this call to initialize glut/GL -- don't execute any OpenGL code before this call!
    glutInit(&argc,argv);
    
    // size and placement hints to the window system
    glutInitWindowSize(width,height);
    glutInitWindowPosition(10,10);
    
    // double buffered, RGB color mode, use depth buffer and stencil buffer
    // stencil buffer is not really needed for the sample code, but will be for shadow volumes
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
    
    // create a GLUT window (not drawn until glutMainLoop() is entered)
    // wid is the window ID
    wid = glutCreateWindow("Andy's Shadow Volume");    
    
    // time to register callbacks 
    
    // window size changes 
    glutReshapeFunc(reshape);
    
    // keypress handling when the current window has input focus 
    glutKeyboardFunc(keyboard);
    
    // mouse event handling 
    glutMouseFunc(mouse_button);           // button press/release
    glutMotionFunc(button_motion);         // mouse motion w/ button down
    
    // Read in the model file specified on the command line
    read_file(argv[1]);
	
    // function to draw contents of our window -- 
    //  this is where most of your work will focus!
    glutDisplayFunc(draw);
    
    initalize();
    
    // this is the event loop entry:
    // take event off the queue, call the handler, repeat
    glutMainLoop();
    
    return 0;
}
