#
# This file is part of Nokia VPCC implementation
#
# Copyright (c) 2019-2020 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
#
# Contact: VPCC.contact@nokia.com
#
# This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
# subsidiaries. All rights are reserved.
#
# Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
# written consent of Nokia.
#

cmake_minimum_required(VERSION 3.4.1)

# Import the ARCore library.
add_library(arcore SHARED IMPORTED)
set_target_properties(arcore PROPERTIES IMPORTED_LOCATION
                      ${ARCORE_LIBPATH}/${ANDROID_ABI}/libarcore_sdk_c.so)

# This is the main app library.
add_library(arplayer_native SHARED
           ${source_tmc}
           ../../Sources/ARPlayer.cpp
           ../../Sources/ARPlayerApplication.cpp
           ../../Sources/DebugRenderer.cpp
           ../../Sources/DebugTextRenderer.cpp
           ../../Sources/GraphicsAPI.cpp
           ../../Sources/HEVCBitstream.cpp
           ../../Sources/HEVC.cpp
           ../../Sources/HighResolutionTimer.cpp
           ../../Sources/HWVideoDecoder.cpp
           ../../Sources/PassthroughCameraRenderer.cpp
           ../../Sources/FeaturePointRenderer.cpp
           ../../Sources/PCCBitstream60.cpp
           ../../Sources/PCCParser60.cpp
           ../../Sources/PCCRenderer.cpp
           ../../Sources/QuadRenderer.cpp
           ../../Sources/TGA.cpp
           ../../Sources/Texture2D.cpp
           ../../Sources/Android/Logger.cpp
           ../../Sources/Android/FileSystem.cpp
           ../../Sources/Android/Surface.cpp
           ../../Sources/Android/SurfaceTexture.cpp
           ../../Sources/Android/JNIInterface.cpp
           ../../Sources/Android/HWVideoDecoderMediaCodec.cpp
           ../../Sources/Android/ARPlayerApplicationJNI.cpp)

target_include_directories(arplayer_native PRIVATE
                           ../../Sources
                           ../../Sources/External/glm
                           ../../Sources/External/STB
                           ../../Sources/External/ARCore/include)

target_link_libraries(arplayer_native
                      android
                      log
                      EGL
                      GLESv3
                      mediandk
                      arcore)
