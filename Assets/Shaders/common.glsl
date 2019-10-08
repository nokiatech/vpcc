#if defined(VERTEX_SHADER)
    #if (__VERSION__ < 130)
        #define in attribute
        #define out varying
    #endif
#endif

#if defined(FRAGMENT_SHADER)
    #if (__VERSION__ < 130)
        #define in varying
        #define outputFragmentColor gl_FragColor
    #else
        out highp vec4 outputFragmentColor;
    #endif
#endif

#if defined(ENABLE_SAMPLER_2D_RECT)
    #if (__VERSION__ < 150)
        #define texture texture2DRect
    #endif
#else
    #if (__VERSION__ < 150)
        #define texture texture2D
    #endif
#endif

#if defined(PLATFORM_ANDROID)
    #if defined(ENABLE_MANUAL_VIDEO_TEXTURE_UPLOAD)
        #define samplerVideo sampler2D
    #else
        #define samplerVideo __samplerExternal2DY2YEXT
    #endif
#else
    #define samplerVideo sampler2D
#endif

// Extra linebreak...
