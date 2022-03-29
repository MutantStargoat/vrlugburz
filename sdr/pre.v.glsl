attribute vec4 apos;
attribute vec3 anorm;
attribute vec4 atex;

varying vec3 vpos;
varying vec3 vnorm;
varying vec2 vtex;

void main()
{
	vec4 pos = gl_ModelViewMatrix * apos;
	pos.z -= 1.5;

	gl_Position = gl_ProjectionMatrix * pos;

	vpos = pos.xyz;
	vnorm = gl_NormalMatrix * anorm;
	vtex = (gl_TextureMatrix[0] * atex).st;
}
