varying vec3 vnorm;

void main()
{
	gl_FragColor.rgb = vnorm * 0.5 + 0.5;
}
