
// Visualization of a Vector Field by Andy Niccolai on 3/30/2011
// for Advanced Graphics at the Colorado School of Mines

// dx = sin(15.0 * point.x + point.y);
// dy = cos(4.0 * point.x + 11.0 * point.y);

varying vec2 coords;
uniform sampler2D tex;   			// this is the texture!!

const float STEP_SIZE = 0.001;
const int NUM_STEPS = 10;			// each direction, half the number of total steps
uniform float u_time;

vec2 normalize(vec2 point)
{
	return point / sqrt(point.x*point.x+point.y*point.y);	
}

vec2 delta(vec2 point)
{
	vec2 d;
	
	d.x = sin(15.0 * point.x + point.y);
	d.y = cos(4.0 * point.x + 11.0 * point.y);
	d = normalize(d);
	
	return d * STEP_SIZE;	
}

void main()
{	
	vec4 color;
	vec2 point = coords;
	
	for(int i=0; i<NUM_STEPS; i++)
	{
		point = point + delta(point);		
		color += texture2D(tex, point);
	}	
	
	point = coords;
	
	for(int i=0; i<NUM_STEPS; i++)
	{
		point = point - delta(point);
		color += texture2D(tex, point);
	}
	
	float totalSteps = float(2*NUM_STEPS);
	
	color = color/totalSteps;
	
	gl_FragColor = color;
}


