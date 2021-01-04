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

#include "VPCC/VPCCDatatypes110.h"

namespace pcc
{
    class PCCContext;
}

namespace VPCC110
{
    int decode(pcc::PCCContext* context, int32_t atlasIndex);

    // Needed in GPU rendering
    int32_t patchBlockToCanvasBlock(Patch& patch, const size_t blockU, const size_t blockV, size_t canvasStrideBlock, size_t canvasHeightBlock);
}
