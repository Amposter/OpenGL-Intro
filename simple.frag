#version 330
out vec4 out_col;
uniform vec4 fcolor;
void main( void )
{    
	out_col = fcolor; //gl_FragColor is supposed to be depricated
}
