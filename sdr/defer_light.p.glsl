uniform vec2 tex_scale;
uniform sampler2D gtex[4];
uniform vec3 light_pos, light_color;

void main()
{
	vec2 uv = gl_FragCoord.xy * tex_scale;

	vec3 pos = texture2D(gtex[0], uv).rgb;
	vec3 norm = normalize(texture2D(gtex[1], uv).rgb);
	vec3 color = texture2D(gtex[2], uv).rgb;

	vec3 ldir = light_pos - pos;
	float att = 1.0 / dot(ldir, ldir);

	ldir = normalize(ldir);
	float ndotl = max(dot(ldir, norm), 0.0);

	vec3 dcol = color * light_color * att * ndotl;

	gl_FragColor = vec4(dcol, 1.0);
}
