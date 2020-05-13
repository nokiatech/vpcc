#ifdef GL_ES
precision highp float;
#endif

in vec2 v_tex_coord;

uniform samplerVideo u_texture_y;

float sample_depth(vec2 uv)
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
    yuv.g = yuv.r;
    yuv.b = yuv.r;

    vec3 rgb = kColorConversion601VideoRange * yuv;

    return rgb.r;

#elif defined(ENABLE_MEDIA_CODEC)

    vec3 yuv = texture(u_texture_y, uv).rgb;

    return yuv.r;

#endif
}

void main()
{
    float depth_sample = sample_depth(v_tex_coord);

    outputFragmentColor = vec4(depth_sample, depth_sample, depth_sample, 1.0);
}
