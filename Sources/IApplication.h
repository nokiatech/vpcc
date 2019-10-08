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

class IApplication
{
public:

    IApplication() {}
    virtual ~IApplication() {}

    virtual void onPause() = 0;
    virtual void onResume(void* env, void* context, void* activity) = 0;

    virtual void onSurfaceCreated() = 0;
    virtual void onDisplayGeometryChanged(int display_rotation, int width, int height) = 0;

    virtual void onDrawFrame() = 0;

    virtual void onSingleTap(float x, float y) = 0;
    virtual void onDoubleTap(float x, float y) = 0;

    virtual void onDrag(float x0, float y0, float x1, float y1) = 0;

    virtual void onScale(float s) = 0;
};
