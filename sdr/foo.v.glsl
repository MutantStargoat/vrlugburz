attribute vec3 apos;
attribute vec3 anorm;

varying vec3 vnorm;

void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * vec4(apos, 1.0);
	vnorm = gl_NormalMatrix * anorm;
}
