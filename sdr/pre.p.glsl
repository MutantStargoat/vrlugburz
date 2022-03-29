uniform sampler2D tex;

varying vec3 vpos;
varying vec3 vnorm;
varying vec2 vtex;

void main()
{
#ifdef USE_TEX
	vec3 dcol = texture2D(tex, vtex).rgb;
#else
	vec3 dcol = gl_FrontMaterial.diffuse.rgb;
#endif

	gl_FragColor.rgb = dcol;
	gl_FragColor.a = 1.0;
}
