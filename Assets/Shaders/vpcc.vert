#ifdef GL_ES
precision highp float;
#endif

// Attributes
in ivec2 a_block_uv;
in ivec2 a_patch_u0v0;
in ivec3 a_patch_u1v1d1;
in ivec3 a_projection;

// x = u0
// y = v0
// z = 0
// w = 0
in ivec4 a_patch_size_u0v0;

// x = orientation
// y = occupancy resolution
// z = depth projection mode
// w = 0
in ivec4 a_patch_properties;

// Uniforms
uniform samplerVideo u_depth_y;
uniform samplerVideo u_color_y;
uniform samplerVideo u_color_uv;
uniform samplerVideo u_occupancy_y;

uniform mat4 u_mvp;
uniform vec3 u_offset;
uniform float u_scale;

#define DEPTH_SCALE 255.0
#define OCCUPANCY_SCALE 255.0 * 2.0
#define BOUNDING_BOX_MAX 1024.0
#define POINT_SIZE_MAX 35.0

// Varyings
#if defined(ENABLE_VERIFICATION_LAYER)

out uvec4 v_transformFeedback;

#endif

out vec3 v_color;

// Functions
vec3 sample_color(ivec2 uv)
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
    yuv.r = texelFetch(u_color_y, uv, 0).r;
    yuv.gb = texelFetch(u_color_uv, uv / 2, 0).rg - vec2(0.5, 0.5);

    vec3 rgb = kColorConversion601VideoRange * yuv;

    return rgb;

#elif defined(ENABLE_MEDIA_CODEC)

    vec3 yuv = texelFetch(u_color_y, uv, 0).rgb;

    // itu_601
    // itu_601_full_range
    // itu_709
    return yuv_2_rgb(yuv, itu_709);

#endif
}

int sample_depth(ivec2 uv)
{
    float y = texelFetch(u_depth_y, uv, 0).r;

    return int(y * DEPTH_SCALE);
}

int sample_occupancy(ivec2 uv)
{
    float y = texelFetch(u_occupancy_y, uv, 0).r;

    return int(y * OCCUPANCY_SCALE);
}

ivec3 get_position(ivec2 position, ivec2 uv, ivec3 projection, int depth_projection_mode)
{
    int depth = 0;

    if (depth_projection_mode == 0) // 0: related to the min depth value
    {
        depth = (sample_depth(uv) + a_patch_u1v1d1.z);
    }
    else // 1: related to the max value
    {
        depth = a_patch_u1v1d1.z - sample_depth(uv);
    }

    ivec3 projected = ivec3(depth, position + a_patch_u1v1d1.xy);

    ivec3 proj_x = ivec3(0, 2, 1);
    ivec3 proj_y = ivec3(2, 0, 1);
    ivec3 proj_z = ivec3(1, 2, 0);

    ivec3 result;

    if (projection == proj_x)
    {
        result.xzy = projected;
    }
    else if (projection == proj_y)
    {
        result.zxy = projected;
    }
    else if (projection == proj_z)
    {
        result.yzx = projected;
    }

    return result;
}

ivec2 patch_to_canvas(ivec2 uv, int patch_orientation, ivec2 patch_u0v0, ivec2 patch_size_u0v0, int occupancy_resolution)
{
    int u = uv.x;
    int v = uv.y;

    int patch_u0 = patch_u0v0.x;
    int patch_v0 = patch_u0v0.y;

    int patch_size_u0 = patch_size_u0v0.x;
    int patch_size_v0 = patch_size_u0v0.y;

    int x = 0;
    int y = 0;

    if (patch_orientation == 0) // DEFAULT
    {
        x = u + patch_u0 * occupancy_resolution;
        y = v + patch_v0 * occupancy_resolution;
    }
    else if (patch_orientation == 1) // SWAP
    {
        x = v + patch_u0 * occupancy_resolution;
        y = u + patch_v0 * occupancy_resolution;
    }
    else if (patch_orientation == 2) // ROT180
    {
        x = (patch_size_u0 * occupancy_resolution - 1 - u) + patch_u0 * occupancy_resolution;
        y = (patch_size_v0 * occupancy_resolution - 1 - v) + patch_v0 * occupancy_resolution;
    }
    else if (patch_orientation == 3) // ROT270
    {
        x = v + patch_u0 * occupancy_resolution;
        y = (patch_size_u0 * occupancy_resolution - 1 - u) + patch_v0 * occupancy_resolution;
    }
    else if (patch_orientation == 4) // MIRROR
    {
        x = (patch_size_u0 * occupancy_resolution - 1 - u) + patch_u0 * occupancy_resolution;
        y = v + patch_v0 * occupancy_resolution;
    }
    else if (patch_orientation == 5) // MROT90
    {
        x = (patch_size_v0 * occupancy_resolution - 1 - v) + patch_u0 * occupancy_resolution;
        y = (patch_size_u0 * occupancy_resolution - 1 - u) + patch_v0 * occupancy_resolution;
    }
    else if (patch_orientation == 6) // MROT180
    {
        x = u + patch_u0 * occupancy_resolution;
        y = (patch_size_v0 * occupancy_resolution - 1 - v) + patch_v0 * occupancy_resolution;
    }
    else if (patch_orientation == 7) // ROT90
    {
        x = (patch_size_v0 * occupancy_resolution - 1 - v) + patch_u0 * occupancy_resolution;
        y = u + patch_v0 * occupancy_resolution;
    }
    else if (patch_orientation == 8) // MROT270
    {
        x = v + patch_u0 * occupancy_resolution;
        y = u + patch_v0 * occupancy_resolution;
    }

    return ivec2(x, y);
}

void main()
{
    int patch_orientation = a_patch_properties.x;
    int occupancy_resolution = a_patch_properties.y;
    int patch_depth_projection_mode = a_patch_properties.z;

    ivec2 position = ivec2(gl_VertexID & 15, gl_VertexID >> 4) + ivec2(a_block_uv.x * occupancy_resolution, a_block_uv.y * occupancy_resolution);

    ivec2 position_in_block = ivec2(gl_VertexID & 15, gl_VertexID >> 4);
    ivec2 position_in_patch = position_in_block + ivec2(a_block_uv.x * occupancy_resolution, a_block_uv.y * occupancy_resolution);

    ivec2 uv = patch_to_canvas(position_in_patch, patch_orientation, a_patch_u0v0, a_patch_size_u0v0.xy, occupancy_resolution);

    // Scale occupancy UVs (since size can be different compared to depth & color)
    ivec2 texture_size = textureSize(u_depth_y, 0);

    ivec2 occupancy_size = textureSize(u_occupancy_y, 0);
    ivec2 occupancy_scale_factor = texture_size / occupancy_size;

    ivec2 occupancy_uv = uv / occupancy_scale_factor;
    int occupancy_sample = sample_occupancy(occupancy_uv);

    if (occupancy_sample > 0)
    {
        // Point
        ivec3 p = get_position(position, uv, a_projection, patch_depth_projection_mode);

        vec3 point = vec3(p.x, p.y, p.z);
        point = point - u_offset;
        point = point / BOUNDING_BOX_MAX;

        v_color = sample_color(uv);

        gl_Position = u_mvp * vec4(point, 1.0);

        // Transform feedback output
        uint r = uint(v_color.r * 255.0f);
        uint g = uint(v_color.g * 255.0f);
        uint b = uint(v_color.b * 255.0f);

        uint packed_rgb = (b << 16) + (g << 8) + r;

#if defined(ENABLE_VERIFICATION_LAYER)

        v_transformFeedback = uvec4(uint(p.x), uint(p.y), uint(p.z), packed_rgb);

#endif

        // Calculate point size

        // Use normalized device coordinates to calculate the PointSize of a vertex based on it's distance from the perspective camera.
        vec3 ndc = gl_Position.xyz / gl_Position.w;
        float z_distance = 1.0 - ndc.z;

        // 1 is close (right up in your face)
        // 0 is far (at the far plane)
        gl_PointSize = POINT_SIZE_MAX * z_distance * u_scale;
    }
    else
    {
#if defined(ENABLE_VERIFICATION_LAYER)

        // Transform feedback output
        v_transformFeedback = uvec4(0x7fff, 0x7fff, 0x7fff, 0xffffff);

#endif

        // Point
        v_color = vec3(0.0, 0.0, 0.0);
        
        gl_Position = vec4(-1.0, -1.0, -1.0, -1.0);
        gl_PointSize = 1.0;
    }
}
