
varying vec2 coords;  
uniform sampler2D tex;   

const float step = 1.0/1024.0;  // step size to access adjacent texture sample
// meaningful texture coordinates are in 0...1 and the texture is 1024x1024

void main()
{
  // a very poor implementation of a truncated Gaussian

  int i,j;
  float val = 0.0;
  float total_wt = 0.0;

  for ( i=-10; i<=10; i++ )
    for ( j=-10; j<=10; j++ )
      {
        float wt = exp(-(float(i*i+j*j))/30.0);
        total_wt += wt;
        val += wt*texture2D(tex,coords+step*vec2(float(i),float(j))).r;
      }
  val = val/total_wt;

  gl_FragColor = vec4(val,val,val,1.0);   
}

