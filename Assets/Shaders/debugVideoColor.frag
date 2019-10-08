precision highp float;

in vec2 v_tex_coord;

uniform samplerVideo u_texture_y;
uniform samplerVideo u_texture_uv;

vec3 sample_color(vec2 uv)
{
#if defined(ENABLE_VIDEO_TOOLBOX) || defined(ENABLE_MANUAL_VIDEO_TEXTURE_UPLOAD)

    mat3 kColorConversion601VideoRange = mat3(1.164,  1.164, 1.164,
                                              0.0,   -0.392, 2.017,
                                              1.596, -0.813, 0.0);

    mat3 kColorConversion601FullRange = mat3(1.0,  1.0,   1.0,
                                             0.0, -0.343, 1.765,
                                             1.4, -0.711, 0.0);

    mat3 kColorConversion709 = mat3(1.164,  1.164, 1.164,
                                    0.0,   -0.213, 2.112,
                                    1.793, -0.533, 0.0);

    vec3 yuv;
    yuv.r = texture(u_texture_y, uv).r;
    yuv.gb = texture(u_texture_uv, uv).rg - vec2(0.5, 0.5);

    vec3 rgb = kColorConversion601VideoRange * yuv;

    return rgb;

#elif defined(ENABLE_MEDIA_CODEC)

    vec3 yuv = texture(u_texture_y, uv).rgb;

    // itu_601
    // itu_601_full_range
    // itu_709
    return yuv_2_rgb(yuv, itu_709);

#endif
}

void main()
{
    vec3 color_sample = sample_color(v_tex_coord);

    outputFragmentColor = vec4(color_sample.rgb, 1.0);
}
