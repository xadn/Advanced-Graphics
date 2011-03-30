
// this file is a good starting point

varying vec2 coords;
uniform sampler2D tex;   // this is the texture!!

void main()
{	
	vec4 color;
	vec2 point = coords;
	
	const float STEP_SIZE = 0.1;
	const int NUM_STEPS = 10;
	const float DIV_STEPS = 0.1;
	
	for(int i=0; i<NUM_STEPS; i++)
	{
		point.x = point.x + STEP_SIZE * sin(15.0 * point.x + point.y);
		point.y = point.y + STEP_SIZE * cos(4.0 * point.x + 11.0 * point.y);
		
		point = point / sqrt(point.x*point.x+point.y*point.y);
		
		color += texture2D(tex, point);
	}	
	
	color = color*DIV_STEPS;
	
	gl_FragColor = color;   // just look up color; note the return value is of type vec4/RGBA
}


