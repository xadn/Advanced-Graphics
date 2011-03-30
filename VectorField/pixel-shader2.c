
// this file is a good starting point

varying vec2 coords;
uniform sampler2D tex;   // this is the texture!!

void main()
{
  gl_FragColor = texture2D(tex,coords);   // just look up color; note the return value is of type vec4/RGBA
}


