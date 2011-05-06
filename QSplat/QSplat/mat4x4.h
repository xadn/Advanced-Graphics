
#include <iostream>

using namespace std;

#if (!defined(__MAT4X4_H))
#define __MAT4X4_H

/* ------------------------------------------------------------------- */

template <class T>

class mat4x4 {

  // entries come in OpenGL order:
  //    0 4 8  12
  //    1 5 9  13
  //    2 6 10 14
  //    3 7 11 15
  T *e;

 public:

  mat4x4();
  mat4x4 ( const mat4x4<T> &m );
  ~mat4x4();

  mat4x4<T> & operator= ( mat4x4 m );

  // multiply (on the left and right) -- in the standard
  // linear algebra sense
  mat4x4<T> & lmul ( mat4x4<T> m );
  mat4x4<T> & rmul ( mat4x4<T> m );

  // access matrix entries
  T & operator() ( int i, int j );

  // return pointer to e
  T *pointer();
};

typedef mat4x4<double> mat4x4d;
typedef mat4x4<float> mat4x4f;

/* ------------------------------------------------------------------- */

// matrix multiplication
template <class T>
mat4x4<T> operator* ( mat4x4<T> m, mat4x4<T> n );

// translation matrix
template <class T>
mat4x4<T> Translation ( T a, T b, T c );

// identity
template <class T>
mat4x4<T> Identity ();

// rotation matrix; (x,y,z)=axis, alpha=angle
template <class T>
mat4x4<T> Rotation ( T x, T y, T z, double alpha );

// uniform scale matrix
template <class T>
mat4x4<T> Scale ( T a );

// nonunuform scale
template <class T>
mat4x4<T> Scale ( T a, T b, T c );

// print out a matrix
template <class T>
ostream & operator << ( ostream &o, mat4x4<T> m );

/* ------------------------------------------------------------------- */
/* ---------------- implementation of functions and methods ------------- */
/* ------------------------------------------------------------------- */

template <class T>
mat4x4<T>::mat4x4 ()
{
  e = new T[16];
}

template <class T>
mat4x4<T>::mat4x4 ( const mat4x4<T> &m )
{
  e = new T[16];
  for ( int i=0; i<16; i++ )
    e[i] = m.e[i];
}

template <class T>
mat4x4<T> & mat4x4<T>::operator= ( mat4x4<T> m )
{
  if (!e)
    e = new T[16];
  for ( int i=0; i<16; i++ )
    e[i] = m.e[i];
  return *this;
}

template <class T>
mat4x4<T>::~mat4x4<T>()
{
  delete[] e;
  e = NULL;
}

static int IX ( int i, int j )
{
  return j|(i<<2);
}

template <class T>
mat4x4<T> & mat4x4<T>::lmul ( mat4x4<T> m )
{
  int i,j,k;

  T *res = new T[16];

  for ( i=0; i<4; i++ )
    for ( j=0; j<4; j++ )
      {
	res[IX(i,j)]=0;
	for ( k=0; k<4; k++ )
	  res[IX(i,j)] += m.e[IX(i,k)]*e[IX(k,j)];
      }
  
  for ( i=0; i<16; i++ )
    e[i] = res[i];

  delete[] res;

  return *this;
}

template <class T>
mat4x4<T> & mat4x4<T>::rmul ( mat4x4<T> m )
{
  int i,j,k;

  T *res = new T[16];

  for ( i=0; i<4; i++ )
    for ( j=0; j<4; j++ )
      {
	res[IX(i,j)]=0;
	for ( k=0; k<4; k++ )
	  res[IX(i,j)] += e[IX(i,k)]*m.e[IX(k,j)];
      }
  
  for ( i=0; i<16; i++ )
    e[i] = res[i];

  delete[] res;

  return *this;
}

template <class T>
T & mat4x4<T>::operator() ( int i, int j )
{
  return e[IX(i,j)];
}

template <class T>
mat4x4<T> operator* ( mat4x4<T> m, mat4x4<T> n )
{
  int i,j,k;

  mat4x4<T> res;

  for ( i=0; i<4; i++ )
    for ( j=0; j<4; j++ )
      {
	res(i,j)=0;
	for ( k=0; k<4; k++ )
	  res(i,j) += m(i,k)*n(k,j);
      }

  return res;
}

template <class T>
T * mat4x4<T>::pointer()
{
  T *res = new T[16];
  for ( int i=0; i<16; i++ )
    res[i] = e[i];
  return res;
}

template <class T> 
mat4x4<T> Identity ()
{
  mat4x4<T> res;
  int i,j;
  for ( i=0; i<4; i++ )
    for ( j=0; j<4; j++ )
      res(i,j) = (i==j);
  return res;
}

template <class T>
mat4x4<T> Translation ( T a, T b, T c )
{
  mat4x4<T> res = Identity<T>();
  res(0,3) = a;
  res(1,3) = b;
  res(2,3) = c;
  return res;
}

template <class T>
mat4x4<T> Scale ( T a, T b, T c )
{
  mat4x4<T> res;
  int i,j;
  for ( i=0; i<4; i++ )
    for ( j=0; j<4; j++ )
      res(i,j) = 0;
  res(0,0) = a;
  res(0,0) = b;
  res(0,0) = c;
  return res; 
}

template <class T>
mat4x4<T> Scale ( T a )
{
  return Scale(a,a,a);
}

template <class T> 
ostream & operator << ( ostream &o, mat4x4<T> m )
{
  int i,j;
  for ( i=0; i<4; i++ )
    {
      for ( j=0; j<4; j++ )
	o << m(i,j) << " ";
      o << endl;
    }
  return o;
}

template <class T>
mat4x4<T> Rotation ( T x, T y, T z, double alpha )
{
  mat4x4<T> res;
  res(0,0) = 1+(1-cos(alpha))*(x*x-1);
  res(1,1) = 1+(1-cos(alpha))*(y*y-1);
  res(2,2) = 1+(1-cos(alpha))*(z*z-1);
  res(0,1) = -z*sin(alpha)+(1-cos(alpha))*x*y;
  res(0,2) = y*sin(alpha)+(1-cos(alpha))*x*z;
  res(1,0) = z*sin(alpha)+(1-cos(alpha))*x*y;
  res(1,2) = -x*sin(alpha)+(1-cos(alpha))*y*z;
  res(2,0) = -y*sin(alpha)+(1-cos(alpha))*x*z;
  res(2,1) = x*sin(alpha)+(1-cos(alpha))*y*z;
  res(3,3) = 1;
  res(0,3) = res(1,3) = res(2,3) = res(3,0) = res(3,1) = res(3,2) = 0;
  return res;
}

/* ------------------------------------------------------------------- */

#endif
