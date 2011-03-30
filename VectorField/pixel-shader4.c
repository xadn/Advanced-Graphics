
// this file is a good starting point

varying vec2 coords;
uniform sampler2D tex;   // this is the texture!!

void main()
{
	
	vec4 color;
	vec2 myCoord = coords;
	
	for(int i=0; i<10; i++)
	{
		myCoord.x = myCoord.x + 0.001;
		color += texture2D(tex,myCoord);
	}	
	color = color*0.1;
	
	gl_FragColor = color;   // just look up color; note the return value is of type vec4/RGBA
}


