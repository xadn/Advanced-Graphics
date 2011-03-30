
varying vec2 coords;  // to be interpolated at rasterization stage	

void main()
{
  coords = gl_Vertex.xy;
  gl_Position = ftransform();
}							
