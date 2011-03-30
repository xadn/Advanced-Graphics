
// this file is a good starting point

varying vec2 coords;
uniform sampler2D tex;   // this is the texture!!

//sin(15.0 * point.x + point.y);
//cos(4.0 * point.x + 11.0 * point.y);

const float STEP_SIZE = 0.01;
const int NUM_STEPS = 10;
const float DIV_STEPS = 0.1;

vec2 normalize(vec2 point)
{
	return point / sqrt(point.x*point.x+point.y*point.y);	
}

vec2 nextPoint(vec2 point)
{
	vec2 delta;
	
	delta.x = sin(15.0 * point.x + point.y);
	delta.y = cos(4.0 * point.x + 11.0 * point.y);
	delta = normalize(delta);
	
	point = point + STEP_SIZE * delta;
	
	return point;	
}

void main()
{	
	vec4 color;
	vec2 point = coords;
	
	
	
	for(int i=0; i<NUM_STEPS; i++)
	{
		point = nextPoint(point);		
		color += texture2D(tex, point);
	}	
	
	color = color*DIV_STEPS;
	
	gl_FragColor = color;   // just look up color; note the return value is of type vec4/RGBA
}


