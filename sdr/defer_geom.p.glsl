varying vec3 vpos, vnorm, vtang;
varying vec2 vtex;

void main()
{
	gl_FragData[0].xyz = vpos;
	gl_FragData[1].xyz = vnorm * 0.5 + 0.5;
}
