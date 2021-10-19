attribute vec4 apos;
attribute vec3 anorm;
attribute vec3 atang;
attribute vec4 atex;

varying vec3 vpos, vnorm, vtang;
varying vec2 vtex;

void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * apos;
	vpos = (gl_ModelViewMatrix * apos).xyz;
	vnorm = gl_NormalMatrix * anorm;
	vtang = gl_NormalMatrix * atang;
	vtex = (gl_TextureMatrix[0] * atex).st;
}
