
#include <GL/glew.h>
#ifdef __APPLE__
#	include <GLUT/glut.h>
#else
#	include <GL/glut.h>
#endif
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <cstring>

using namespace std;


const char* WINDOW_TITLE = "Andy's Vector Field Visualization";

// try the other combinations of shaders here:
// 1- a procedureal texture
// 2- simple texture lookup (equivalent to what the fixed functionality pipeline would do)
// 3- a (very bad) implementation of a Gaussian filter

string PS[] = {\
  "pixel-shader6.c",\
  "pixel-shader1.c",\
  "pixel-shader2.c",\
  "pixel-shader3.c",\
  "pixel-shader4.c",\
  "pixel-shader5.c",\
  "pixel-shader6.c",\
  "pixel-shader7.c"\
  };

string VS [] = {\
  "vertex-shader2.c",\
  "vertex-shader1.c",\
  "vertex-shader2.c",\
  "vertex-shader3.c"\
};

void showOptions()
{
  cout << "\nprogram arguments:\n\n";
  
  cout << "1 - pixel-shader1.c // pink and blue procedureal texture\n";
  cout << "2 - pixel-shader2.c // simple texture lookup\n";
  cout << "3 - pixel-shader3.c // gaussian filter\n";
  cout << "4 - pixel-shader4.c // horizontal averaging\n";
  cout << "5 - pixel-shader5.c // radial vector field\n";
  cout << "6 - pixel-shader6.c // source and sink field\n";
  cout << "7 - pixel-shader7.c // curvy one from the screenshot on blackboard\n\n";
}

/* ----------------------------------------------------------- */

// a few routines useful for reading shader programs from files
//  and printing out status of a shader (e.g. compilation/linking errors)



char *read_file ( const char *name )
{
  int size = 0;
  {  
    ifstream ifs(name);
    assert(ifs);
    do {
      size++;
      char c;
      c = ifs.get();
    }
    while(!ifs.eof());
  }
  char *res = new char[size];
  ifstream ifs(name);
  ifs.read(res,size-1);
  res[size-1] = 0;
  return res;
}

void printShaderInfoLog(GLuint obj)
{
  int infologLength = 0;
  int charsWritten  = 0;
  char *infoLog;
  
  glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

  if (infologLength > 0)
    {
      infoLog = new char[infologLength];
      glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
      cout << infoLog << endl;
      delete[] infoLog;
    }
}

void printProgramInfoLog(GLuint obj)
{
  int infologLength = 0;
  int charsWritten  = 0;
  char *infoLog;
  
  glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
  
  if (infologLength > 0)
    {
      infoLog = new char[infologLength];
      glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
      cout << infoLog << endl;
      delete[] infoLog;
    }
}

/* ----------------------------------------------------------- */

// global variables -- used for communication between callback functions

// window ID
int wid;

// default (initial) window size
#define VPD_DEFAULT 800

// size of the window 
int width = VPD_DEFAULT;
int height = VPD_DEFAULT;

// zoom interface related stuff
bool is_middle_mouse_button_down = false;
GLfloat zoom = 1;   // this one controls the field of view
GLint mprev[2];   // coordinates of last [mouse movement with middle button down] event

// IDs of noise texture and the GPU program
GLuint noise_texture;
GLuint p;

/* --------------------------------------------- */

/* draw the scene */

GLvoid draw()
{
  glutSetWindow(wid);
 
  glClear(GL_COLOR_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);  
  glLoadIdentity();
  glOrtho(-zoom,zoom,-zoom,zoom,-0.5,0.5);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // use the noise texture
  // since we are doing GPU programming YOU have to look up colors in one of the programs
  // etc. in order for texture to work
  glBindTexture(GL_TEXTURE_2D,noise_texture);   
  glEnable(GL_TEXTURE_2D);

  // no need for depth test
  glDisable(GL_DEPTH_TEST);

  // use program p
  glUseProgram(p);
  

  // just render a single quad with texture coords
  glBegin(GL_QUADS);
  glTexCoord2f(0,0);
  glVertex2f(-1,-1);
  glTexCoord2f(1,0);
  glVertex2f(1,-1);
  glTexCoord2f(1,1);
  glVertex2f(1,1);
  glTexCoord2f(0,1);
  glVertex2f(-1,1);
  glEnd();

  glFinish();

  // back to the old-fashioned pipeline
  glUseProgram(0);

  glutSwapBuffers();
}

/* --------------------------------------------- */

// function called when a mouse button is pressed or released

GLvoid mouse_button(GLint btn, GLint state, GLint x, GLint y)
{
    if (btn==GLUT_MIDDLE_BUTTON)
    {
        if (state==GLUT_DOWN)
        {
            is_middle_mouse_button_down = true;
            mprev[0] = x;
            mprev[1] = y;
        }
        if (state==GLUT_UP)
        {
            is_middle_mouse_button_down = false;
            zoom += 8*(y-mprev[1])/(float)height;
        }
    }
}

/* --------------------------------------------- */

// function called when mouse is moving with a button down

GLvoid button_motion(GLint x, GLint y)
{
  if (is_middle_mouse_button_down)
    {
      // middle button is down - update zoom factor (related to field of view - see draw()
      // also, save the 
      zoom *= 1+(y-mprev[1])/200.0;
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

int main(int argc, char **argv)
{  
  showOptions();
  
  glutInit(&argc,argv);

  glutInitWindowSize(width,height);
  glutInitWindowPosition(10,10);

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

  wid = glutCreateWindow(WINDOW_TITLE);    

  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse_button);          
  glutMotionFunc(button_motion);        
  glutDisplayFunc(draw);

  // initialize glew and check for OpenGL 2.0 support
  glewInit();
  if (glewIsSupported("GL_VERSION_2_0"))
    cout << "Ready for OpenGL 2.0" << endl;
  else 
    {
      cout << "OpenGL 2.0 not supported" << endl;;
      return 1;
    }
  
  // create the noise texture
  glGenTextures(1,&noise_texture);
  glBindTexture(GL_TEXTURE_2D,noise_texture);
  {
    float *arr = new float[VPD_DEFAULT*VPD_DEFAULT];
    for ( int i=0; i<VPD_DEFAULT*VPD_DEFAULT; i++ )
      arr[i] = drand48();
    glTexImage2D(GL_TEXTURE_2D,0,1,VPD_DEFAULT,VPD_DEFAULT,0,GL_LUMINANCE,GL_FLOAT,arr);
    delete[] arr;
  }
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

  // Let the user select a shader
  int shaderNumber = 0;
  if (argc > 1) {
    shaderNumber = atoi(argv[1]);
  }
  
  // Initialize shaders
  {
    const GLchar *vsh = read_file(VS[0].c_str());
    const GLchar *fsh = read_file(PS[shaderNumber].c_str());

    GLint vshid = glCreateShader(GL_VERTEX_SHADER);
    GLint fshid = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vshid,1,&vsh,NULL);
    glShaderSource(fshid,1,&fsh,NULL);
    printShaderInfoLog(vshid);
    printShaderInfoLog(fshid);
    glCompileShader(vshid);
    glCompileShader(fshid);
    printShaderInfoLog(vshid);
    printShaderInfoLog(fshid);
    p = glCreateProgram();
    glAttachShader(p,vshid);
    glAttachShader(p,fshid);
    printProgramInfoLog(p);
    glLinkProgram(p);
    printProgramInfoLog(p);

  }
  

  glutMainLoop();

  return 0;
}
