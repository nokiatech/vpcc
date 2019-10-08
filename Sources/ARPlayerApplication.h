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

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "GraphicsAPI.h"
#include "PCCRenderer.h"
#include "DebugTextRenderer.h"
#include "DebugTextRenderer.h"
#include "PassthroughCameraRenderer.h"
#include "QuadRenderer.h"
#include "DebugRenderer.h"
#include "Texture2D.h"
#include "FeaturePointRenderer.h"

#include "IApplication.h"
#include "ARPlayer.h"

#include "glm.h"

#if PLATFORM_ANDROID

    #include "arcore_c_api.h"

#elif PLATFORM_IOS || PLATFORM_MACOS

#endif

class ARPlayerApplication
: public IApplication
{
public:

    struct Settings
    {
        std::string filename = "";
        bool enableAR = false;
        bool enableDebugMode = false;
        bool enableDualLayerMode = false;
        bool enableManualVideoTextureUpload = false;
    };

public:

    ARPlayerApplication(Settings settings);
    virtual ~ARPlayerApplication();

    virtual void onPause();
    virtual void onResume(void* env, void* context, void* activity);
    virtual void onSurfaceCreated();
    virtual void onDisplayGeometryChanged(int display_rotation, int width, int height);
    virtual void onDrawFrame();

    virtual void onSingleTap(float x, float y);
    virtual void onDoubleTap(float x, float y);

    virtual void onDrag(float x0, float y0, float x1, float y1);

    virtual void onScale(float s);

private:

    struct DeviceInfo
    {
        std::string glVersion;
        std::string glslVersion;
        std::string vendor;
        std::string renderer;
    };

    DeviceInfo _deviceInfo;

    float _fps = 0;
    int64_t _frameDuration = 0;

    int64_t _frameCounter = 0;
    int64_t _previousFrameTime = 0;

    Settings _settings;

    bool _playbackPaused = false;

    float _rotation = 0.0f;
    float _scale = 1.0f;

    int32_t _screenWidth = 0;
    int32_t _screenHeight = 0;
    int32_t _displayRotation = 0;

    ARPlayer* _arPlayer = NULL;

    PCCRenderer _pccRenderer;

    DebugRenderer _debugDepthRenderer;
    DebugRenderer _debugColorRenderer;
    DebugRenderer _debugOccupancyRenderer;

    QuadRenderer _spriteRenderer;
    Texture2D _emptyCacheIcon;
    Texture2D _frameSyncIssueIcon;
    Texture2D _playbackPausedIcon;
    Texture2D _nokiaLogo;

    Texture2D _focusSquareDashed;
    Texture2D _focusSquareSolid;

    DebugTextRenderer _debugTextRenderer;

    PassthroughCameraRenderer _passthroughCameraRenderer;
    FeaturePointRenderer _featurePointRenderer;

private:

#if PLATFORM_ANDROID

    ArSession* _arSession = NULL;
    ArConfig* _arConfig = NULL;
    ArFrame* _arFrame = NULL;

    bool _installRequested = false;

    void arcoreUpdateCamera(glm::mat4& view, glm::mat4& projection);
    void arcoreRenderFeaturePoints(glm::mat4 model, glm::mat4 view, glm::mat4 projection);
    bool arcoreObjectHitTest(float x, float y, glm::mat4& model);
    bool arcoreGetMatrixFromAnchor(ArAnchor* anchor, glm::mat4& model);

    float normalizedDistanceToPlane(const ArSession& arSession, const ArPose& planePose, const ArPose& cameraPose);

    bool _objectEnabled = false;
    glm::mat4 _objectModel = glm::mat4(1.0f);

#endif
};
