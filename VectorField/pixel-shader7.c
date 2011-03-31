
// Visualization of a Vector Field by Andy Niccolai on 3/30/2011
// for Advanced Graphics at the Colorado School of Mines

// f(x,y)=(0.4*sin(x'+10y')+y'
	    // ,0.4*sin(x'+10y')-x'),
// where x'=2x-1 and y'=2y-1. The range of x and y is 0...1

varying vec2 coords;
uniform sampler2D tex;   			// this is the texture!!

const float STEP_SIZE = 0.001;
const int NUM_STEPS = 30;			// each direction, half the number of total steps

vec2 normalize(vec2 point)
{
	return point / sqrt(point.x*point.x+point.y*point.y);	
}

vec2 delta(vec2 point)
{
	vec2 d;
	
	d.x = 0.4*sin(2.0*point.x-1.0 + 10.0*(2.0*point.y-1.0) ) + (2.0*point.y-1.0);
	d.y = 0.4*sin(2.0*point.x-1.0 + 10.0*(2.0*point.y-1.0) ) - (2.0*point.x-1.0);
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


