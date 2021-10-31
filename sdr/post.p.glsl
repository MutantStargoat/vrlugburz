uniform sampler2D tex;
uniform float exposure;
uniform vec2 tex_scale;

void main()
{
	vec3 texel = texture2D(tex, gl_TexCoord[0].xy).rgb;

	vec3 color = vec3(1.0) - exp(-texel * exposure);

#ifdef FB_NOT_SRGB
	gl_FragColor = vec4(pow(color, vec3(1.0 / 2.2)), 1.0);
#else
	gl_FragColor = vec4(color, 1.0);
#endif
}
