uniform sampler2D tex_color, tex_spec, tex_norm;

varying vec3 vpos, vnorm, vtang;
varying vec2 vtex;

void main()
{
	vec3 color = texture2D(tex_color, vtex).rgb;

	gl_FragData[0].xyz = vpos;
	gl_FragData[1].xyz = vnorm;
	gl_FragData[2].xyz = color;
}
