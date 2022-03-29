uniform sampler2D tex;

varying vec3 vpos;
varying vec3 vnorm;
varying vec2 vtex;

#define MTL_KD		gl_FrontMaterial.diffuse.rgb
#define MTL_KS		gl_FrontMaterial.specular.rgb
#define MTL_SHIN	gl_FrontMaterial.shininess

void main()
{
#ifdef USE_TEX
	vec3 dcol = texture2D(tex, vtex).rgb;
#else
	vec3 dcol = MTL_KD;
#endif

	vec3 lpos = vec3(-0.5, 0.5, -1.5);
	vec3 l = normalize(lpos - vpos);
	vec3 n = normalize(vnorm);
	vec3 v = normalize(-vpos);
	vec3 h = normalize(v + l);

	float ndotl = max(dot(n, l), 0.0);
	float ndoth = max(dot(n, h), 0.0);

	vec3 diffuse = dcol * ndotl;
	vec3 specular = MTL_KS * pow(ndoth, 80.0);

	gl_FragColor.rgb = diffuse + specular;
	gl_FragColor.a = 1.0;
}
