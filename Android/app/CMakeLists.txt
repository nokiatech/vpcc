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

file(GLOB headers_tmc
        ../../Sources/External/mpeg-pcc-tmc2/dependencies/nanoflann/*.hpp
        ../../Sources/External/mpeg-pcc-tmc2/dependencies/nanoflann/*.h
        ../../Sources/External/mpeg-pcc-tmc2/dependencies/libmd5/*.h
        ../../Sources/External/mpeg-pcc-tmc2/source/lib/PccLibCommon/include/*.h
        ../../Sources/External/mpeg-pcc-tmc2/source/lib/PccLibBitstreamCommon/include/*.h
        ../../Sources/External/mpeg-pcc-tmc2/source/lib/PccLibBitstreamReader/include/*.h)

file(GLOB sources_tmc
        ../../Sources/External/mpeg-pcc-tmc2/dependencies/libmd5/*.c
        ../../Sources/External/mpeg-pcc-tmc2/source/lib/PccLibCommon/source/PCCFrameContext.cpp
        ../../Sources/External/mpeg-pcc-tmc2/source/lib/PccLibCommon/source/PCCContext.cpp
        ../../Sources/External/mpeg-pcc-tmc2/source/lib/PccLibBitstreamCommon/source/PCCHighLevelSyntax.cpp
        ../../Sources/External/mpeg-pcc-tmc2/source/lib/PccLibBitstreamCommon/source/PCCBitstream.cpp
        ../../Sources/External/mpeg-pcc-tmc2/source/lib/PccLibBitstreamReader/source/PCCBitstreamReader.cpp)

include_directories(include_tmc
        ../../Sources/External/mpeg-pcc-tmc2/dependencies/nanoflann
        ../../Sources/External/mpeg-pcc-tmc2/dependencies/libmd5
        ../../Sources/External/mpeg-pcc-tmc2/source/lib/PccLibCommon/include
        ../../Sources/External/mpeg-pcc-tmc2/source/lib/PccLibBitstreamCommon/include
        ../../Sources/External/mpeg-pcc-tmc2/source/lib/PccLibBitstreamReader/include)

# This is the main app library.
add_library(arplayer_native SHARED
        ${sources_tmc}
        ../../Sources/ARPlayerApplication.cpp
        ../../Sources/DebugRenderer.cpp
        ../../Sources/DebugTextRenderer.cpp
        ../../Sources/GraphicsAPI.cpp
        ../../Sources/HighResolutionTimer.cpp
        ../../Sources/HWVideoDecoder.cpp
        ../../Sources/PassthroughCameraRenderer.cpp
        ../../Sources/FeaturePointRenderer.cpp
        ../../Sources/TGA.cpp
        ../../Sources/Texture2D.cpp
        ../../Sources/QuadRenderer.cpp
        ../../Sources/HEVCBitstream.cpp
        ../../Sources/HEVC.cpp
        ../../Sources/VPCC/VPCCPlayer.cpp
        ../../Sources/VPCC/VPCCDecoder110.cpp
        ../../Sources/VPCC/VPCCParser110.cpp
        ../../Sources/VPCC/VPCCRenderer.cpp
        ../../Sources/Android/Logger.cpp
        ../../Sources/Android/FileSystem.cpp
        ../../Sources/Android/Surface.cpp
        ../../Sources/Android/SurfaceTexture.cpp
        ../../Sources/Android/JNIInterface.cpp
        ../../Sources/Android/HWVideoDecoderMediaCodec.cpp
        ../../Sources/Android/ARPlayerApplicationJNI.cpp)

target_include_directories(arplayer_native PRIVATE
        ${include_tmc}
        ../../Sources
        ../../Sources/External
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
