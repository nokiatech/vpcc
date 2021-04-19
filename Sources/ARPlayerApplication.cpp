/**
* This file is part of Nokia VPCC implementation
*
* Copyright (c) 2019-2020 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
*
* Contact: VPCC.contact@nokia.com
*
* This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
* subsidiaries. All rights are reserved.
*
* Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
* written consent of Nokia.
*/

#include "ARPlayerApplication.h"

#include <array>

#include "FileSystem.h"
#include "Logger.h"
#include "HighResolutionTimer.h"

ARPlayerApplication::ARPlayerApplication(Settings settings)
{
    _settings = settings;

#if PLATFORM_MACOS

    // Always true since OpenGL Texture Cache (VideoToolbox) YUV texture upload
    // fails to compability error (kCVReturnPixelBufferNotOpenGLCompatible  = -6683).
    _settings.enableManualVideoTextureUpload = true;

#elif PLATFORM_WINDOWS

	// There's no HW video decoder + OpenGL interop for fast texture sharing.
    // Update has to done manually with OpenGL texture updates.
	_settings.enableManualVideoTextureUpload = true;

#elif PLATFORM_ANDROID

    // Frame rendering (AMediaCodec_releaseOutputBuffer / AMediaCodec_releaseOutputBufferAtTime)
    // and surface texture update (updateTextImage) mechanism has some problems that are tricky to overcome
    // -> manual texture upload is enabled even it decrease performance significantly.
    //
    // Known problems:
    //
    // 1.) Frames cannot be rendered from background thread since only one surface texture
    // can be bound to HW decoder, and this texture needs to be shared with PCC rendering.
    //
    // 2.) Calling frame rendering in a background thread and surface texture update in OpenGL ES thread
    // would lead to a situation where surface texture data is incorrect. Surface texture update always takes
    // latest frame data that is rendered. E.g., calling frame rendering twice and surface texture update would take
    // data from last rendered frame -> data from the first frame is skipped.
    //
    // 3.) Calling frame rendering and surface texture update in OpenGL ES thread also has issues.
    // For an unknown reason, an unknown amount of delay (~50ms) is needed between frame rendering call and
    // surface texture update call. If there's no delay surface texture update has data from previous frame or frame is completely black.
    // On Java side, there's a callback mechanism (SurfaceTexture.OnFrameAvailableListener) to prevent this issue this is not exposed to NDK.
    // Also OnFrameAvailableListener mechanism has a downside, it is not guaranteed that it won't skip frames. Based on some early prototyping
    // frame skipping was verified.
    _settings.enableManualVideoTextureUpload = true;

#endif
}

ARPlayerApplication::~ARPlayerApplication()
{
#if ENABLE_ARCORE_SUPPORT

    if (_settings.enableAR)
    {
        if (_arSession != NULL)
        {
            ArSession_destroy(_arSession);
            ArFrame_destroy(_arFrame);
        }
        
        _passthroughCameraRenderer.destroy();
        _featurePointRenderer.destroy();

        GL_CHECK_ERRORS();
    }

#endif

#if APPLICATION_MODE == APPLICATION_MODE_VPCC

    if (_vpccPlayer)
    {
        _vpccPlayer->stop();
        _vpccPlayer->shutdown();

        delete _vpccPlayer;
        _vpccPlayer = NULL;
    }

    _vpccRenderer.destroy();

#endif

    _debugDepthRenderer.destroy();
    _debugColorRenderer.destroy();
    _debugOccupancyRenderer.destroy();

    _debugTextRenderer.destroy();
    _spriteRenderer.destroy();

    freeTexture(_emptyCacheIcon);
    freeTexture(_frameSyncIssueIcon);
    freeTexture(_playbackPausedIcon);
    freeTexture(_nokiaLogo);

    freeTexture(_focusSquareDashed);
    freeTexture(_focusSquareSolid);
}

void ARPlayerApplication::onPause()
{
    LOG_D("onPause()");

#if ENABLE_ARCORE_SUPPORT

    if (_settings.enableAR)
    {
        if (_arSession != NULL)
        {
            ArSession_pause(_arSession);
        }
    }

#endif
}

void ARPlayerApplication::onResume(void* env, void* context, void* activity)
{
    LOG_D("onResume()");

#if ENABLE_ARCORE_SUPPORT

    if (_settings.enableAR)
    {
        if (_arSession == NULL)
        {
            ArInstallStatus install_status;
            bool user_requested_install = !_installRequested;

            assert(ArCoreApk_requestInstall(env, activity, user_requested_install, &install_status) == AR_SUCCESS);

            switch (install_status)
            {
                case AR_INSTALL_STATUS_INSTALLED:
                {
                    break;
                }

                case AR_INSTALL_STATUS_INSTALL_REQUESTED:
                {
                    _installRequested = true;
                    return;
                }
            }

            ArStatus status = ArSession_create(env, context, &_arSession);

            assert(status == AR_SUCCESS);
            assert(_arSession);

            ArConfig_create(_arSession, &_arConfig);
            ArConfig_setFocusMode(_arSession, _arConfig, AR_FOCUS_MODE_AUTO);
            ArConfig_setLightEstimationMode(_arSession, _arConfig, AR_LIGHT_ESTIMATION_MODE_DISABLED);
            ArConfig_setPlaneFindingMode(_arSession, _arConfig, AR_PLANE_FINDING_MODE_HORIZONTAL);
            ArConfig_setUpdateMode(_arSession, _arConfig, AR_UPDATE_MODE_LATEST_CAMERA_IMAGE);

            ArSession_configure(_arSession, _arConfig);

            ArFrame_create(_arSession, &_arFrame);
            assert(_arFrame);

            ArSession_setDisplayGeometry(_arSession, _displayRotation, _screenWidth, _screenHeight);
        }

        const ArStatus status = ArSession_resume(_arSession);
        assert(status == AR_SUCCESS);
    }

#endif
}

void ARPlayerApplication::onSurfaceCreated()
{
    LOG_D("onSurfaceCreated()");

    // Device info
    _deviceInfo.glVersion = (char*)glGetString(GL_VERSION);
    _deviceInfo.glslVersion = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
    _deviceInfo.vendor = (char*)glGetString(GL_VENDOR);
    _deviceInfo.renderer = (char*)glGetString(GL_RENDERER);

    LOG_I("---------- DEVICE INFO - BEGIN ----------");

    LOG_I("API version: %s", _deviceInfo.glVersion.c_str());
    LOG_I("Shader language version: %s", _deviceInfo.glslVersion.c_str());
    LOG_I("Vendor: %s", _deviceInfo.vendor.c_str());
    LOG_I("Renderer: %s", _deviceInfo.renderer.c_str());
    LOG_I("Display: %dx%d", _screenWidth, _screenHeight);

    LOG_I("---------- DEVICE INFO - END ----------");

#if ENABLE_ARCORE_SUPPORT

    // Initialize
    if (_settings.enableAR)
    {
        _passthroughCameraRenderer.create();
        _featurePointRenderer.create();

        GL_CHECK_ERRORS();
    }

#endif

    _debugDepthRenderer.create(DebugRenderer::Type::DEBUG_DEPTH, _settings.enableManualVideoTextureUpload);
    _debugColorRenderer.create(DebugRenderer::Type::DEBUG_COLOR, _settings.enableManualVideoTextureUpload);
    _debugOccupancyRenderer.create(DebugRenderer::Type::DEBUG_OCCUPANCY, _settings.enableManualVideoTextureUpload);

    _debugTextRenderer.create();
    _spriteRenderer.create(Texture2D::Type::TEXTURE_RGB, _settings.enableManualVideoTextureUpload);

    loadTexture("Assets/Graphics/empty_cache_icon.png", _emptyCacheIcon);
    loadTexture("Assets/Graphics/frame_sync_issue_icon.png", _frameSyncIssueIcon);
    loadTexture("Assets/Graphics/playback_paused_icon.png", _playbackPausedIcon);
    loadTexture("Assets/Graphics/nokia_logo.png", _nokiaLogo);

    loadTexture("Assets/Graphics/focus_square_dashed.png", _focusSquareDashed);
    loadTexture("Assets/Graphics/focus_square_solid.png", _focusSquareSolid);

    GL_CHECK_ERRORS();

    // Initialize player
#if APPLICATION_MODE == APPLICATION_MODE_VPCC

    _vpccRenderer.create(_settings.enableManualVideoTextureUpload);

    if (_vpccPlayer == NULL)
    {
        _vpccPlayer = new VPCCPlayer();

        VPCCPlayer::Config config;
        config.manualVideoTextureUpload = _settings.enableManualVideoTextureUpload;

		VPCCPlayer::Result::Enum result = VPCCPlayer::Result::RESULT_OK;

		result = _vpccPlayer->initialize(config);
        assert(result == VPCCPlayer::Result::RESULT_OK);

		result = _vpccPlayer->open(_settings.filename);
        assert(result == VPCCPlayer::Result::RESULT_OK);

		result = _vpccPlayer->play();
        assert(result == VPCCPlayer::Result::RESULT_OK);
    }

#endif

    GL_CHECK_ERRORS();
}

void ARPlayerApplication::onWindowResize(int displayRotation, int width, int height)
{
    LOG_D("onWindowResize(%d, %d)", width, height);

    glViewport(0, 0, width, height);

    _displayRotation = displayRotation;

    _screenWidth = width;
    _screenHeight = height;

#if ENABLE_ARCORE_SUPPORT

    if (_settings.enableAR)
    {
        if (_arSession != NULL)
        {
            ArSession_setDisplayGeometry(_arSession, _displayRotation, width, height);
        }
    }

#endif
}

void ARPlayerApplication::onDrawFrame()
{
#if APPLICATION_MODE == APPLICATION_MODE_VPCC

    drawVPCC();

#endif
}

void ARPlayerApplication::drawStats()
{
    // Update stats
    int64_t frameTime = HighResolutionTimer::getTimeMs();
    int64_t duration = frameTime - _previousFrameTime;
    float fps = 1000.0f / (float)duration;

    _frameCounter++;
    _previousFrameTime = frameTime;

    if (_frameCounter % 10 == 0)
    {
        _fps = fps;
        _frameDuration = duration;
    }

    // Draw stats
    if (_settings.enableDebugMode)
    {
        glm::vec4 textColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        glm::vec4 backgroundColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

        _debugTextRenderer.printFormat(glm::vec2(1.0f, 1.0f), textColor, backgroundColor, "API version: %s", _deviceInfo.glVersion.c_str());
        _debugTextRenderer.printFormat(glm::vec2(1.0f, 2.0f), textColor, backgroundColor, "Shader language version: %s", _deviceInfo.glslVersion.c_str());
        _debugTextRenderer.printFormat(glm::vec2(1.0f, 3.0f), textColor, backgroundColor, "Vendor: %s", _deviceInfo.vendor.c_str());
        _debugTextRenderer.printFormat(glm::vec2(1.0f, 4.0f), textColor, backgroundColor, "Renderer: %s", _deviceInfo.renderer.c_str());
        _debugTextRenderer.printFormat(glm::vec2(1.0f, 5.0f), textColor, backgroundColor, "Display: %dx%d", _screenWidth, _screenHeight);

        _debugTextRenderer.printFormat(glm::vec2(1.0f, 7.0f), textColor, backgroundColor, "Frame: %d", _frameCounter);
        _debugTextRenderer.printFormat(glm::vec2(1.0f, 8.0f), textColor, backgroundColor, "%f fps (rendering)", _fps);
        _debugTextRenderer.printFormat(glm::vec2(1.0f, 9.0f), textColor, backgroundColor, "%d ms / frame (rendering)", _frameDuration);
    }

    // Draw Logo
    glm::vec2 logoSize = glm::vec2(766.0f, 125.0f) * 0.5f;

    float logoPosX = _screenWidth - logoSize.x - 100.0f;
    float logoPosY = 100.0f;

    _spriteRenderer.draw(_nokiaLogo, glm::vec2(logoPosX, logoPosY), logoSize, 0.0f, glm::vec4(0.0f / 255.0f, 51.0f / 255.0f, 153.0f / 255.0f, 1.0f));
}

#if APPLICATION_MODE == APPLICATION_MODE_VPCC

void ARPlayerApplication::drawVPCC()
{
    // Draw ARCore stuff
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glViewport(0, 0, _screenWidth, _screenHeight);
    glScissor(0, 0, _screenWidth, _screenHeight);
    
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 arModel = glm::mat4(1.0f);
    glm::mat4 arView = glm::mat4(1.0f);
    glm::mat4 arProjection = glm::mat4(1.0f);

    bool renderPccModel = true;

#if ENABLE_ARCORE_SUPPORT

    if (_settings.enableAR)
    {
        arModel = _objectModel;
        renderPccModel = _objectEnabled;

        glm::fquat cameraOrientation = glm::fquat(0.0f, 0.0f, 0.0f, 1.0f);
        glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 0.0f);

        arcoreUpdateCamera(arView, arProjection, cameraOrientation, cameraPosition);

        _passthroughCameraRenderer.draw();

        arcoreRenderFeaturePoints(arModel, arView, arProjection);
    }

#endif

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    // Focus square rendering
    if (_settings.enableAR)
    {
        bool planeHit = false;

#if ENABLE_ARCORE_SUPPORT

        glm::mat4 tmp(1.0f);

        planeHit = arcoreObjectHitTest((float)_screenWidth * 0.5f, (float)_screenHeight * 0.5f, tmp);

#endif

        _spriteRenderer.draw(planeHit ? _focusSquareSolid : _focusSquareDashed, glm::vec2((float)_screenWidth * 0.5f - 200.0f, (float)_screenHeight * 0.5f - 200.0f), glm::vec2(400.0f, 400.0f), 0.0f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    glm::vec4 textColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    glm::vec4 backgroundColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    if (renderPccModel)
    {
        // Prepare transformations
        float centerX = 357.0f;
        float centerY = 512.0f;
        float centerZ = 244.0f;

        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(_scale, _scale, _scale));
        glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), _rotation, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 translate = glm::mat4(1.0f);

        if (_settings.filename.find("S26") != std::string::npos || _settings.filename.find("LONG") != std::string::npos)
        {
            centerX = 246.0f;
            centerY = 11.0f;
            centerZ = 157.0f;
        }
        else if (_settings.filename.find("S23") != std::string::npos || _settings.filename.find("LOOT") != std::string::npos)
        {
            centerX = 221.0f;
            centerY = 10.0f;
            centerZ = 226.0f;
        }
        else if (_settings.filename.find("S24") != std::string::npos || _settings.filename.find("RED") != std::string::npos)
        {
            centerX = 350.0f;
            centerY = 11.0f;
            centerZ = 266.0f;
        }
        else if (_settings.filename.find("S25") != std::string::npos || _settings.filename.find("SOLDIER") != std::string::npos)
        {
            centerX = 221.0f;
            centerY = 11.0f;
            centerZ = 193.0f;
        }
        else if (_settings.filename.find("S22") != std::string::npos || _settings.filename.find("QUEEN") != std::string::npos)
        {
            centerX = 100.0f;
            centerY = 450.0f;
            centerZ = 450.0f;

            glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), _rotation, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 rotZ = glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), glm::vec3(0.0f, 0.0f, 1.0f));

            rotate = rotY * rotZ;
        }

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);
        glm::mat4 mvp = glm::mat4(1.0f);

        glm::vec3 offset = glm::vec3(centerX, centerY, centerZ);

        if (_settings.enableAR)
        {
            model = arModel * rotate * scale;
            view = arView;
            projection = arProjection;
            mvp = projection * view * model;
        }
        else
        {
            glm::vec3 up = { 0.0f, 1.0f, 0.0f };
            glm::vec3 position = { -1.5f, 1.0f, -0.25f };
            glm::vec3 target = { 0.0f, 0.5f, 0.0f };

            model = scale * rotate * translate;
            view = glm::lookAt(position, target, up);
            projection = glm::perspective(glm::radians(45.0f), (float)_screenWidth / (float)_screenHeight, 0.1f, 100.0f);
            mvp = projection * view * model;
        }

        // Rendering
        VPCCRenderer::PresentationFrame presentationFrame;
        VPCCPlayer::Result::Enum result = _vpccPlayer->fetchPresentationFrame(presentationFrame);

        if (result == VPCCPlayer::Result::RESULT_NEW_FRAME || result == VPCCPlayer::Result::RESULT_OLD_FRAME)
        {
            _vpccRenderer.draw(presentationFrame, model, view, projection, mvp, offset, _scale);

            // Debug render video textures
            if (_settings.enableDebugMode)
            {
                glm::vec2 debugViewSize = glm::vec2(400, 400);

#if PLATFORM_ANDROID || PLATFORM_IOS

                glm::vec2 debugDepthViewPosition = glm::vec2(0, 800);
                glm::vec2 debugColorViewPosition = glm::vec2(0, 1200);
                glm::vec2 debugOccupancyViewPosition = glm::vec2(0, 1600);

#elif PLATFORM_MACOS || PLATFORM_WINDOWS

                glm::vec2 debugDepthViewPosition = glm::vec2(0, _screenHeight - debugViewSize.y);
                glm::vec2 debugColorViewPosition = glm::vec2(400, _screenHeight - debugViewSize.y);
                glm::vec2 debugOccupancyViewPosition = glm::vec2(800, _screenHeight - debugViewSize.y);

#endif

                // Layer #1
                Texture2D depthTextureY;
                depthTextureY.handle = presentationFrame.depth0->yTextureHandle;
                depthTextureY.type = (presentationFrame.depth0->target == GL_TEXTURE_EXTERNAL_OES) ? Texture2D::Type::VIDEO_TEXTURE : Texture2D::Type::TEXTURE_RGB;

                Texture2D depthTextureUV;
                depthTextureUV.handle = 0;
                depthTextureUV.type = Texture2D::Type::INVALID;

                _debugDepthRenderer.draw(depthTextureY, depthTextureUV, debugDepthViewPosition, debugViewSize);

                Texture2D colorTextureY;
                colorTextureY.handle = presentationFrame.color0->yTextureHandle;
                colorTextureY.type = (presentationFrame.color0->target == GL_TEXTURE_EXTERNAL_OES) ? Texture2D::Type::VIDEO_TEXTURE : Texture2D::Type::TEXTURE_RGB;

                Texture2D colorTextureUV;
                colorTextureUV.handle = presentationFrame.color0->uvTextureHandle;
                colorTextureUV.type = (presentationFrame.color0->target == GL_TEXTURE_EXTERNAL_OES) ? Texture2D::Type::VIDEO_TEXTURE : Texture2D::Type::TEXTURE_RGB;

                _debugColorRenderer.draw(colorTextureY, colorTextureUV, debugColorViewPosition, debugViewSize);

                Texture2D occupancyTextureY;
                occupancyTextureY.handle = presentationFrame.occupancy->yTextureHandle;
                occupancyTextureY.type = (presentationFrame.occupancy->target == GL_TEXTURE_EXTERNAL_OES) ? Texture2D::Type::VIDEO_TEXTURE : Texture2D::Type::TEXTURE_RGB;

                Texture2D occupancyTextureUV;
                occupancyTextureUV.handle = 0;
                occupancyTextureUV.type = Texture2D::Type::INVALID;

                _debugOccupancyRenderer.draw(occupancyTextureY, occupancyTextureUV, debugOccupancyViewPosition, debugViewSize);

                // Layer #2
                if (presentationFrame.depth1 != NULL && presentationFrame.color1 != NULL)
                {
                    Texture2D depthTextureY;
                    depthTextureY.handle = presentationFrame.depth1->yTextureHandle;
                    depthTextureY.type = (presentationFrame.depth1->target == GL_TEXTURE_EXTERNAL_OES) ? Texture2D::Type::VIDEO_TEXTURE : Texture2D::Type::TEXTURE_RGB;

                    Texture2D depthTextureUV;
                    depthTextureUV.handle = 0;
                    depthTextureUV.type = Texture2D::Type::INVALID;

                    _debugDepthRenderer.draw(depthTextureY, depthTextureUV, glm::vec2(400, 800), glm::vec2(400, 400));

                    Texture2D colorTextureY;
                    colorTextureY.handle = presentationFrame.color1->yTextureHandle;
                    colorTextureY.type = (presentationFrame.color1->target == GL_TEXTURE_EXTERNAL_OES) ? Texture2D::Type::VIDEO_TEXTURE : Texture2D::Type::TEXTURE_RGB;

                    Texture2D colorTextureUV;
                    colorTextureUV.handle = presentationFrame.color1->uvTextureHandle;
                    colorTextureUV.type = (presentationFrame.color1->target == GL_TEXTURE_EXTERNAL_OES) ? Texture2D::Type::VIDEO_TEXTURE : Texture2D::Type::TEXTURE_RGB;

                    _debugColorRenderer.draw(colorTextureY, colorTextureUV, glm::vec2(400, 1200), glm::vec2(400, 400));
                }
            }

            if (_playbackPaused)
            {
                _spriteRenderer.draw(_playbackPausedIcon, glm::vec2(0.0f, 2000.0f), glm::vec2(200.0f, 200.0f), 0.0f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            }
        }
        else if (result == VPCCPlayer::Result::RESULT_EOS)
        {
            _vpccPlayer->restart();
        }
        else if (result == VPCCPlayer::Result::RESULT_OUT_OF_SYNC_FRAME_AVAILABLE)
        {
            if (_settings.enableDebugMode)
            {
                _spriteRenderer.draw(_frameSyncIssueIcon, glm::vec2(0.0f, 2000.0f), glm::vec2(200.0f, 200.0f), 0.0f, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
            }
        }
        else // No frame available / other error
        {
            if (_settings.enableDebugMode)
            {
                _spriteRenderer.draw(_emptyCacheIcon, glm::vec2(0.0f, 2000.0f), glm::vec2(200.0f, 200.0f), 0.0f, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
            }

            glm::vec4 textColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            glm::vec4 backgroundColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
            
            _debugTextRenderer.printFormat(glm::vec2(1.0f, 11.0f), textColor, backgroundColor, "Loading & parsing...");
        }
    }

	drawStats();
}

#endif

void ARPlayerApplication::onSingleTap(float x, float y)
{
#if ENABLE_ARCORE_SUPPORT

    if (_settings.enableAR)
    {
        _objectEnabled = arcoreObjectHitTest((float)_screenWidth * 0.5f, (float)_screenHeight * 0.5f, _objectModel);
    }

#endif
}

void ARPlayerApplication::onDoubleTap(float x, float y)
{
    _playbackPaused = !_playbackPaused;

#if APPLICATION_MODE == APPLICATION_MODE_VPCC

    if (_playbackPaused)
    {
        _vpccPlayer->pause();
    }
    else
    {
        _vpccPlayer->resume();
    }

#endif
}

void ARPlayerApplication::onDrag(float x0, float y0, float x1, float y1)
{
    float dx = x1 - x0;
    float p = dx / (float)_screenWidth;

    _rotation += glm::two_pi<float>() * p;
}

void ARPlayerApplication::onScale(float s)
{
    _scale = s;
}

#if ENABLE_ARCORE_SUPPORT

void ARPlayerApplication::arcoreRenderFeaturePoints(glm::mat4 model, glm::mat4 view, glm::mat4 projection)
{
    if (_arSession == NULL) return;

    // Update and render point cloud.
    ArPointCloud* pointCloud = NULL;
    ArStatus status = ArFrame_acquirePointCloud(_arSession, _arFrame, &pointCloud);

    if (status == AR_SUCCESS)
    {
        int32_t numPoints = 0;
        ArPointCloud_getNumberOfPoints(_arSession, pointCloud, &numPoints);

        if (numPoints <= 0)
        {
            return;
        }

        std::vector<FeaturePoint> featurePoints;
        featurePoints.resize(numPoints);

        const float* pointCloudPtr = NULL;
        ArPointCloud_getData(_arSession, pointCloud, &pointCloudPtr);

       for (int32_t i = 0; i < numPoints; i += 4)
       {
            FeaturePoint featurePoint;
            featurePoint.x = pointCloudPtr[i + 0];
            featurePoint.y = pointCloudPtr[i + 1];
            featurePoint.z = pointCloudPtr[i + 2];
            featurePoint.w = pointCloudPtr[i + 3];

            featurePoints.push_back(featurePoint);
       }

        _featurePointRenderer.draw(projection * view, featurePoints);

        ArPointCloud_release(pointCloud);
        pointCloud = NULL;
    }
}

void ARPlayerApplication::arcoreUpdateCamera(glm::mat4& view, glm::mat4& projection, glm::fquat& orientation, glm::vec3& position)
{
    if (_arSession == NULL) return;

    // Set texture where camera renders background image
    ArSession_setCameraTextureName(_arSession, _passthroughCameraRenderer.getCameraTextureHandle());

    // Update session to get current frame and camera texture
    if (ArSession_update(_arSession, _arFrame) != AR_SUCCESS)
    {
        return;
    }

    // Fetch AR world camera view and projection matrices
    ArCamera* camera;
    ArFrame_acquireCamera(_arSession, _arFrame, &camera);

    ArCamera_getViewMatrix(_arSession, camera, glm::value_ptr(view));
    ArCamera_getProjectionMatrix(_arSession, camera, 0.1f, 100.f, glm::value_ptr(projection));

    // Fetch AR world camera pose
    ArPose* cameraPose = NULL;
    ArPose_create(_arSession, NULL, &cameraPose);

    ArCamera_getPose(_arSession, camera, cameraPose);

    float cameraPoseRaw[7] = { 0.0f };
    ArPose_getPoseRaw(_arSession, cameraPose, cameraPoseRaw);

    orientation = glm::fquat(cameraPoseRaw[0], cameraPoseRaw[1], cameraPoseRaw[2], cameraPoseRaw[3]);
    position = glm::vec3(cameraPoseRaw[4], cameraPoseRaw[5], cameraPoseRaw[6]);

    ArPose_destroy(cameraPose);
    cameraPose = NULL;

    ArCamera_release(camera);
    camera = NULL;
}

bool ARPlayerApplication::arcoreObjectHitTest(float x, float y, glm::mat4& model)
{
    if (_arFrame != NULL && _arSession != NULL)
    {
        ArHitResultList* hitResultList = NULL;
        ArHitResultList_create(_arSession, &hitResultList);

        ArFrame_hitTest(_arSession, _arFrame, x, y, hitResultList);

        int32_t hitResultListSize = 0;
        ArHitResultList_getSize(_arSession, hitResultList, &hitResultListSize);

        ArHitResult* finalHitResult = NULL;
        ArTrackableType trackableType = AR_TRACKABLE_NOT_VALID;

        for (int32_t i = 0; i < hitResultListSize; ++i)
        {
            ArHitResult* hitResult = NULL;
            ArHitResult_create(_arSession, &hitResult);
            ArHitResultList_getItem(_arSession, hitResultList, i, hitResult);

            if (hitResult == NULL)
            {
                return false;
            }

            ArTrackable* trackable = NULL;
            ArHitResult_acquireTrackable(_arSession, hitResult, &trackable);

            ArTrackable_getType(_arSession, trackable, &trackableType);

            if (AR_TRACKABLE_PLANE == trackableType)
            {
                ArPose* hitPose = NULL;
                ArPose_create(_arSession, NULL, &hitPose);

                ArHitResult_getHitPose(_arSession, hitResult, hitPose);

                int32_t isPoseInPolygon = 0;
                ArPlane* plane = ArAsPlane(trackable);
                ArPlane_isPoseInPolygon(_arSession, plane, hitPose, &isPoseInPolygon);

                // Use hit pose and camera pose to check if hittest is from the back of the plane
                ArCamera* camera;
                ArFrame_acquireCamera(_arSession, _arFrame, &camera);

                ArPose* cameraPose = NULL;
                ArPose_create(_arSession, NULL, &cameraPose);

                ArCamera_getPose(_arSession, camera, cameraPose);

                ArCamera_release(camera);
                camera = NULL;

                float distanceToPlane = normalizedDistanceToPlane(*_arSession, *hitPose, *cameraPose);

                ArPose_destroy(hitPose);
                hitPose = NULL;

                ArPose_destroy(cameraPose);
                cameraPose = NULL;

                if (!isPoseInPolygon || distanceToPlane < 0.0f)
                {
                    continue;
                }

                finalHitResult = hitResult;

                break;
            }
        }

        if (finalHitResult)
        {
            ArAnchor* anchor = NULL;

            if (ArHitResult_acquireNewAnchor(_arSession, finalHitResult, &anchor) != AR_SUCCESS)
            {
                return false;
            }

            ArTrackingState tracking_state = AR_TRACKING_STATE_STOPPED;
            ArAnchor_getTrackingState(_arSession, anchor, &tracking_state);

            if (tracking_state != AR_TRACKING_STATE_TRACKING)
            {
                ArAnchor_release(anchor);
                anchor = NULL;

                return false;
            }

            ArHitResult_destroy(finalHitResult);
            finalHitResult = NULL;

            ArHitResultList_destroy(hitResultList);
            hitResultList = NULL;

            // Get matrix from anchor
            ArPose* pose = NULL;
            ArPose_create(_arSession, NULL, &pose);

            ArAnchor_getPose(_arSession, anchor, pose);
            ArPose_getMatrix(_arSession, pose, glm::value_ptr(model));

            ArPose_destroy(pose);
            pose = NULL;

            ArAnchor_release(anchor);
            anchor = NULL;

            return true;
        }
    }

    return false;
}

bool ARPlayerApplication::arcoreGetMatrixFromAnchor(ArAnchor* anchor, glm::mat4& model)
{
    ArPose* pose = NULL;
    ArPose_create(_arSession, NULL, &pose);

    ArAnchor_getPose(_arSession, anchor, pose);
    ArPose_getMatrix(_arSession, pose, glm::value_ptr(model));

    ArPose_destroy(pose);
    pose = NULL;

    return true;
}

float ARPlayerApplication::normalizedDistanceToPlane(const ArSession& arSession, const ArPose& planePose, const ArPose& cameraPose)
{
    // Get plane position
    float planePoseRaw[7] = { 0.0f };
    ArPose_getPoseRaw(&arSession, &planePose, planePoseRaw);
    glm::vec3 planePosition(planePoseRaw[4], planePoseRaw[5], planePoseRaw[6]);

    // Get camera position
    float cameraPoseRaw[7] = { 0.0f };
    ArPose_getPoseRaw(&arSession, &cameraPose, cameraPoseRaw);

    // Calculate normal
    glm::vec3 cameraPosition(cameraPoseRaw[4], cameraPoseRaw[5], cameraPoseRaw[6]);
    glm::vec3 cameraToPlane(cameraPosition.x - planePosition.x, cameraPosition.y - planePosition.y, cameraPosition.z - planePosition.z);

    glm::quat planeQuaternion(planePoseRaw[3], planePoseRaw[0], planePoseRaw[1], planePoseRaw[2]);
    glm::vec3 normal = glm::rotate(planeQuaternion, glm::vec3(0.0f, 1.0f, 0.0f));

    return glm::dot(normal, cameraToPlane);
}

#endif
