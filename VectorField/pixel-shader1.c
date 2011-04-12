
varying vec2 coords;

void main()
{
  gl_FragColor.r = sign(sin(20.0*coords.x));
  gl_FragColor.b = 1.0-cos(10.0*coords.y)/2.0;
  gl_FragColor.g = 0.5;
}

