//
//  main.cpp
//  ShadowVolume
//
//  Modified by Andy Niccolai on 2/10/11.
//  Copyright 2011 Colorado School of Mines. All rights reserved.
//

#include <cmath>
#include <cstdio>
#include <cstdlib>
#ifdef __APPLE__
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
    #include <GLUT/glut.h>
#else
    #include <GL/gl.h>
    #include <GL/glu.h>
    #include <GL/glut.h>
#endif
#include <iostream>
#include <fstream>
#include "vec3d.h"
#include "mat4x4.h"

using namespace std;

vec3df light_coord( -5.0, 2.0, 5.0 );
mat4x4f mv_mat;

class triangle
{   
public:
	
	// Normal vector
    vec3df normal;
	
	// Pointers to each vertex of the triangle
	vec3df *vert[3];
	
	// Shadow quad's verticies not in the triangle
	vec3df shadow_pair[3];
	
	// Shadow quad's normal vector
	vec3df shadow_normal[3];
	
	// Return a vertex
	vec3df get(int vertex) { return *vert[vertex]; }
	
	// Return a coordinate of a vertex
	float get(int vertex, int coord) { return (*vert[vertex])[coord]; }
	
	// Calculate the normal vector of the triangle
    void calc_normal() { normal = (get(1) - get(0))^(get(2) - get(0)); }
    
	// Determine if the triangle is lit
    bool lit()
    {
        if( (( light_coord - get(0) ) * normal) > 0 )
			return true;
		return false;
    }
	
	// Calculate the shadow quads and normals
	void calc_shadow()
	{
		for(int v=0; v<3; v++)
		{
			shadow_pair[v] = get(v) - light_coord;
			shadow_normal[v] = (get((v+1)%3) - get(v))^(shadow_pair[v] - get(v));
		}
	}
	
};

int num_triangles;
triangle *tri = NULL;
int num_points;
vec3df *v_list = NULL;

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

void draw_model()
{
    // Grabbing the modelview matrix isn't necessary yet
	
    //glGetFloatv(GL_MODELVIEW_MATRIX, mv_mat.pointer() );
    //cout << mv_mat << "\n";
    
    glBegin(GL_TRIANGLES);
    
	// For each triangle
    for(int i=0; i<num_triangles; i++)
    {
		// Set the normal
		glNormal3f( tri[i].normal[0], tri[i].normal[1], tri[i].normal[2] );
		
		// Set each vertex in the triangle
		for(int v=0; v<3; v++)
			glVertex3f( tri[i].get(v, 0), tri[i].get(v, 1), tri[i].get(v, 2) );
    }
    
    glEnd();
}

void draw_shadow_polygon()
{
	glBegin(GL_QUADS);
    
	// For each triangle
    for(int i=0; i<num_triangles; i++)
    {
		// If the triangle is in shadow 
		if( !tri[i].lit() )
        {
			// Tell the triangle to calculate it's shadow
			tri[i].calc_shadow();
			
			// For each face of the shadow
			for(int v=0; v<3; v++)
			{
				// Set the normal for each face of the shadow
				glNormal3f( tri[i].shadow_normal[v][0], tri[i].shadow_normal[v][1], tri[i].shadow_normal[v][2] );
				
				// First vertex (from the triangle)
				glVertex3f( tri[i].get(v, 0), tri[i].get(v, 1), tri[i].get(v, 2) );
				
				// Second vertex (next vertex from the triangle)
				glVertex3f( tri[i].get((v+1)%3, 0), tri[i].get((v+1)%3, 1), tri[i].get((v+1)%3, 2) );
				
				// Third vertex (calculated from the first vertex)
				glVertex4f(tri[i].shadow_pair[(v+1)%3][0], tri[i].shadow_pair[(v+1)%3][1], tri[i].shadow_pair[(v+1)%3][2], 0);
				
				// Forth vertex (calculated from the second vertex)
				glVertex4f(tri[i].shadow_pair[v][0], tri[i].shadow_pair[v][1], tri[i].shadow_pair[v][2], 0);
			}

		}
    }
    
    glEnd();	
}

/* --------------------------------------------- */

void draw_scene ( )
{
    // draw white/greyish
    //draw_cube();                          // cube
    
    // the rest of this procedure adds smaller
    // cubes of different colors centered at the corners of the white cube below;
    // note that the draw_cube function is used to draw each cube
    // however, modelview matrix and material is changed before each call,
    //  so the resulting cubes look different 
    
    set_material_properties(1,1,1);		// set color to GREY
	glPushMatrix();						// Save the current matrix
    draw_model();						// Draw the model 
    glPopMatrix();						// Restore the matrix
	
	set_material_properties(.5,.5,1);	// set color to BLUE
	glPushMatrix(); 
    draw_shadow_polygon();				// Draw the shadow polygons
    glPopMatrix(); 
	
}
/* --------------------------------------------- */

/* draw the scene */

GLvoid draw()
{
    // ensure we're drawing to the correct GLUT window 
    glutSetWindow(wid);
    
    // GLfloat *trans_matrix;
    // glGetFloatv(GL_MODELVIEW_MATRIX, trans_matrix);
    
    // clear the color buffers 
    // clearing stencil buffer is not really necessary for the sample code,
    // but you'll need it in the shadow volume implementation
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    {
        // in this block, we initialize light source #0
        GLfloat light_ambient[] = { .1, .1, .1, 1.0 };
        GLfloat light_diffuse[] = { .7, .7, .7, 1.0 };
        GLfloat light_specular[] = { 0, 0, 0, 1.0 };
        
        GLfloat light_position[] = { light_coord[0], light_coord[1], light_coord[2], 0 };
        
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
		
		// comment the identity matrix out to "fixate" the light position
       // glLoadIdentity();            // load identity to current matrix
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
    
    // clear color buffer to white 
    glClearColor(1.0, 1.0, 1.0, 1.0);
    
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
    
    glTranslatef(0,0,-20);   // move the cube `forward' with respect to the camera
    
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
    
    // double buffered, RGB color mode, use depth buffer and stencil buffer
    // stencil buffer is not really needed for the sample code, but will be for shadow volumes
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
    
    // create a GLUT window (not drawn until glutMainLoop() is entered)
    // wid is the window ID
    wid = glutCreateWindow("Graphics II: sample code");    
    
    // time to register callbacks 
    
    // window size changes 
    glutReshapeFunc(reshape);
    
    // keypress handling when the current window has input focus 
    glutKeyboardFunc(keyboard);
    
    // mouse event handling 
    glutMouseFunc(mouse_button);           // button press/release
    glutMotionFunc(button_motion);         // mouse motion w/ button down
    
    // Read in the model file specified on the command line
    //read_file(argv[1]);
	cout << "\n" << argv[1];
    read_file(argv[1]);
	
	
    // function to draw contents of our window -- 
    //  this is where most of your work will focus!
    glutDisplayFunc(draw);
    
    // this is the event loop entry:
    // take event off the queue, call the handler, repeat
    glutMainLoop();
    
    return 0;
}
