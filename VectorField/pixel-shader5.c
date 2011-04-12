
// Visualization of a Vector Field by Andy Niccolai on 3/30/2011
// for Advanced Graphics at the Colorado School of Mines

// dx = -y
// dy = x

varying vec2 coords;
uniform sampler2D tex;   			// this is the texture!!
uniform float u_time;

const float STEP_SIZE = 1.0/1024.0;
const int NUM_STEPS = 30;			// each direction, half the number of total steps

vec2 normalize(vec2 point)
{
	return point / sqrt(point.x*point.x+point.y*point.y);	
}

vec2 delta(vec2 point)
{
	vec2 d;
	
	d.x =-point.y;
	d.y =point.x;
	d = normalize(d);
	
	return d * STEP_SIZE;	
}

void main()
{	
	vec4 color;
	vec2 point = coords;
	
	int steps = NUM_STEPS;

	for(int i=0; i<steps; i++)
	{
		point = point + delta(point);		
		color += texture2D(tex, point);
	}	
	
	point = coords;
	
	for(int i=0; i<steps; i++)
	{
		point = point - delta(point);		
		color += texture2D(tex, point);
	}
	
	float totalSteps = float(2*steps);
	
	color = color/totalSteps;
	
	gl_FragColor = color;
}


