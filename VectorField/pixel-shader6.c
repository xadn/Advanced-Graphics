
// this file is a good starting point

varying vec2 coords;
uniform sampler2D tex;   // this is the texture!!

const float STEP_SIZE = 0.001;
const int NUM_STEPS = 30;
const float DIV_STEPS = 0.0333;

vec2 normalize(vec2 point)
{
	return point / sqrt(point.x*point.x+point.y*point.y);	
}

vec2 nextPoint(vec2 point)
{
	vec2 delta;
	
	delta.x = -point.y;
	delta.y = point.x;
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


