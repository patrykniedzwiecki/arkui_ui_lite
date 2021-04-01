/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "compare_tools.h"
#include "dock/screen_device_proxy.h"
#include "draw/draw_utils.h"
#include "gfx_utils/file.h"
#include "gfx_utils/graphic_log.h"
#include "graphic_config.h"
#include "securec.h"

namespace OHOS {
void CompareTools::WaitSuspend()
{
#ifdef _WIN32
    Sleep(DEFAULT_WAIT_TIME_MS);
#else
    usleep(1000 * DEFAULT_WAIT_TIME_MS); // 1000: us to ms
#endif // _WIN32
}

bool CompareTools::StrnCatPath(char* filePath, size_t pathMax, const char* fileName, size_t count)
{
    if ((filePath == nullptr) || (pathMax > DEFAULT_FILE_NAME_MAX_LENGTH)) {
        return false;
    }
    char dest[DEFAULT_FILE_NAME_MAX_LENGTH] = UI_AUTO_TEST_RESOURCE_PATH;
    if (strncat_s(dest, DEFAULT_FILE_NAME_MAX_LENGTH, fileName, count) != EOK) {
        return false;
    }
    if (memcpy_s(static_cast<void *>(filePath), pathMax, dest, DEFAULT_FILE_NAME_MAX_LENGTH) != EOK) {
        return false;
    }
    return true;
}

bool CompareTools::CompareFile(const char* src, size_t length, uint8_t flag)
{
    switch (flag) {
        case COMPARE_BINARY:
            return CompareBinary(src, length);
        case COMPARE_IMAGE:
            // Unrealized : image for comparison
            break;
        default:
            break;
    }
    return false;
}

bool CompareTools::SaveFile(const char* src, size_t length, uint8_t flag)
{
    switch (flag) {
        case COMPARE_BINARY:
            return SaveFrameBuffToBinary(src, length);
        case COMPARE_IMAGE:
            // Unrealized : save frame buff as image
            break;
        default:
            break;
    }
    return false;
}

bool CompareTools::CompareBinary(const char* filePath, size_t length)
{
    if ((filePath == nullptr) || (length > DEFAULT_FILE_NAME_MAX_LENGTH)) {
        return false;
    }
    FILE* fd = fopen(filePath, "rb");
    if (fd == nullptr) {
        return false;
    }
    uint8_t* frameBuf = ScreenDeviceProxy::GetInstance()->GetBuffer();
    uint8_t sizeByColorMode = DrawUtils::GetByteSizeByColorMode(ScreenDeviceProxy::GetInstance()->GetBufferMode());
    uint32_t buffSize = HORIZONTAL_RESOLUTION * VERTICAL_RESOLUTION * sizeByColorMode;
    uint8_t* readBuf = reinterpret_cast<uint8_t*>(malloc(buffSize));
    if (fread(readBuf, buffSize, sizeof(uint8_t), fd) < 0) {
        fclose(fd);
        free(readBuf);
        return false;
    }
    for (int32_t i = 0; i < (buffSize / sizeof(uint8_t)); i++) {
        if (readBuf[i] != frameBuf[i]) {
            GRAPHIC_LOGE("[DIFF]:fileName=%s, read[%d]=%x, write[%d]=%x", filePath, i, readBuf[i], frameBuf[i]);
            fclose(fd);
            free(readBuf);
            return false;
        }
    }
    fclose(fd);
    free(readBuf);
    return true;
}

bool CompareTools::SaveFrameBuffToBinary(const char* filePath, size_t length)
{
    if ((filePath == nullptr) || (length > DEFAULT_FILE_NAME_MAX_LENGTH)) {
        return false;
    }
    FILE* fd = fopen(filePath, "wb+");
    if (fd == nullptr) {
        return false;
    }
    uint8_t* frameBuf = ScreenDeviceProxy::GetInstance()->GetBuffer();
    uint8_t sizeByColorMode = DrawUtils::GetByteSizeByColorMode(ScreenDeviceProxy::GetInstance()->GetBufferMode());
    uint32_t buffSize = HORIZONTAL_RESOLUTION * VERTICAL_RESOLUTION * sizeByColorMode;
    if (fwrite(frameBuf, buffSize, sizeof(uint8_t), fd) < 0) {
        fclose(fd);
        return false;
    }
    fclose(fd);
    return true;
}

bool CompareTools::CheckFileExist(const char* filePath, size_t length)
{
    if ((filePath == nullptr) || (length > DEFAULT_FILE_NAME_MAX_LENGTH)) {
        return false;
    }
    FILE* fd = fopen(filePath, "r");
    if (fd == nullptr) {
        return false;
    }
    fclose(fd);
    return true;
}
} // namespace OHOS