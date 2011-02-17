
#include <iostream>
#include <list>
//#include "triangle.h"

using namespace std;


#if (!defined(__VEC3D_H))
#define __VEC3D_H

/* ------------------------------------------------------------------- */

template <class T>
class vec3d {

  T *coord;

 public:
	
	
  vec3d ();
  vec3d ( T x, T y, T z );
  vec3d ( const vec3d<T> & v );
  ~vec3d();

  vec3d<T> & operator += ( vec3d<T> v );
  vec3d<T> & operator -= ( vec3d<T> v );
  vec3d<T> & operator |= ( vec3d<T> v );
  vec3d<T> & operator &= ( vec3d<T> v );
  vec3d<T> & operator *= ( T c );
  vec3d<T> & operator = ( vec3d<T> v );

  // allocate and return pointer to coordinate vector
  T *pointer();

  T & operator[] ( int i );

  T max();
  T min();

  vec3d<T> & normalize();
};

typedef vec3d<double> vec3dd;
typedef vec3d<float> vec3df;

/* ------------------------------------------------------------------- */

// coordinatewise add
template <class T>
vec3d<T> operator+ ( vec3d<T> v, vec3d<T> w );

// coordinatewise subtract
template <class T>
vec3d<T> operator- ( vec3d<T> v, vec3d<T> w );

// scalar product
template <class T>
T operator* ( vec3d<T> v, vec3d<T> w );

// cross product
template <class T>
vec3d<T> operator^ ( vec3d<T> v, vec3d<T> w );

// coordinatewise max
template <class T>
vec3d<T> operator| ( vec3d<T> v, vec3d<T> w );

// coordinatewise min
template <class T>
vec3d<T> operator& ( vec3d<T> v, vec3d<T> w );

// scalar by vector multiplication
template <class T>
vec3d<T> operator* ( T c, vec3d<T> v );

template <class T>
ostream & operator<< ( ostream &o, vec3d<T> v );

// coordinatewise comparison
template <class T>
bool operator== ( vec3d<T> v, vec3d<T> w );

/* ------------------------------------------------------------------- */

/* ---------------- implementation of functions/methods ------------- */

/* ------------------------------------------------------------------- */

template <class T>
vec3d<T>::vec3d()
{
  coord = new T[3];
}

template <class T>
T vec3d<T>::min()
{
  if (coord[0]<coord[1] && coord[0]<coord[2])
    return coord[0];
  return coord[1]<coord[2] ? coord[1] : coord[2];
}

template <class T>
T vec3d<T>::max()
{
  if (coord[0]>coord[1] && coord[0]>coord[2])
    return coord[0];
  return coord[1]>coord[2] ? coord[1] : coord[2];
}

template <class T>
vec3d<T>::vec3d ( T x, T y, T z )
{
  coord = new T[3];
  coord[0] = x;
  coord[1] = y;
  coord[2] = z;
}

template <class T>
vec3d<T>::vec3d ( const vec3d<T>&v )
{
  coord = new T[3];
  coord[0] = v.coord[0];
  coord[1] = v.coord[1];
  coord[2] = v.coord[2];
}

template <class T>
vec3d<T>::~vec3d()
{
  delete[] coord;
  coord = 0;
}

template <class T>
vec3d<T> & vec3d<T>::operator += ( vec3d<T> v )
{
  coord[0] += v.coord[0];
  coord[1] += v.coord[1];
  coord[2] += v.coord[2];
  return *this;
}

template <class T>
vec3d<T> & vec3d<T>::operator -= ( vec3d<T> v )
{
  coord[0] -= v.coord[0];
  coord[1] -= v.coord[1];
  coord[2] -= v.coord[2];
  return *this;
}

template <class T>
vec3d<T> & vec3d<T>::operator = ( vec3d<T> v )
{
  if (!coord)
    coord = new T[3];
  coord[0] = v.coord[0];
  coord[1] = v.coord[1];
  coord[2] = v.coord[2];
  return *this;
}

template <class T>
vec3d<T> & vec3d<T>::operator*= ( T a )
{
  coord[0] *= a;
  coord[1] *= a;
  coord[2] *= a;
  return *this;
}

template <class T>
vec3d<T> & vec3d<T>::operator |= ( vec3d<T> v )
{
  if (v[0]>coord[0])
    coord[0] = v[0];
  if (v[1]>coord[1])
    coord[1] = v[1];
  if (v[2]>coord[2])
    coord[2] = v[2];
  return *this;
}

template <class T>
vec3d<T> & vec3d<T>::operator &= ( vec3d<T> v )
{
  if (v[0]<coord[0])
    coord[0] = v[0];
  if (v[1]<coord[1])
    coord[1] = v[1];
  if (v[2]<coord[2])
    coord[2] = v[2];
  return *this;
}

template <class T>
vec3d<T> & vec3d<T>::normalize ( )
{
  T l = sqrt(coord[0]*coord[0]+coord[1]*coord[1]+coord[2]*coord[2]);
  coord[0] = coord[0]/l;
  coord[1] = coord[1]/l;
  coord[2] = coord[2]/l;  
  return *this;
}

template <class T>
bool operator== ( vec3d<T> v, vec3d<T> w )
{
  return v[0]==w[0] && v[1]==w[1] && v[2]==w[2];
}

template <class T>
T * vec3d<T>::pointer()
{
  return coord;
}

template <class T>
T & vec3d<T>::operator[] ( int i )
{
  return coord[i];
}

template <class T>
vec3d<T> operator+ ( vec3d<T> v, vec3d<T> w )
{
  return vec3d<T>(v[0]+w[0],v[1]+w[1],v[2]+w[2]);
}

template <class T>
vec3d<T> operator- ( vec3d<T> v, vec3d<T> w )
{
  return vec3d<T>(v[0]-w[0],v[1]-w[1],v[2]-w[2]);
}

template <class T>
T operator* ( vec3d<T> v, vec3d<T> w )
{
  return v[0]*w[0]+v[1]*w[1]+v[2]*w[2];
}

template <class T>
vec3d<T> operator* ( T c, vec3d<T> v )
{
  return vec3d<T>(c*v[0],c*v[1],c*v[2]);
}

template <class T>
vec3d<T> operator^ ( vec3d<T> v, vec3d<T> w )
{
  return vec3d<T>(v[1]*w[2]-v[2]*w[1], v[2]*w[0]-v[0]*w[2], v[0]*w[1]-v[1]*w[0]);
}

template <class T>
vec3d<T> operator| ( vec3d<T> v, vec3d<T> w )
{
  return vec3d<T>(
		  v[0]>w[0] ? v[0] : w[0],
		  v[1]>w[1] ? v[1] : w[1],
		  v[2]>w[2] ? v[2] : w[2]
		  );
}

template <class T>
vec3d<T> operator& ( vec3d<T> v, vec3d<T> w )
{
  return vec3d<T>(
		  v[0]<w[0] ? v[0] : w[0],
		  v[1]<w[1] ? v[1] : w[1],
		  v[2]<w[2] ? v[2] : w[2]
		  );
}

template <class T>
ostream & operator<< ( ostream &o, vec3d<T> v )
{
  o << v[0] << " " << v[1] << " " << v[2];
  return o;
}


#endif


/* ------------------------------------------------------------------- */
