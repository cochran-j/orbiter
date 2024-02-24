/* TextureImage.cpp
 *
 * Copyright (c) 2024 Jack Cochran
 * Licensed under the MIT license.
 */

#include "TextureImage.h"
#include "DDSTextureImage.h"

#include <memory>
#include <utility>
#include <filesystem>

namespace d3d9client {

std::unique_ptr<ITextureImage> chooseTextureImage
    (const std::filesystem::path& filepath) {

    {
        auto ptr = std::make_unique<DDSTextureImage>();
        if (ptr->isFileTypeHandled(filepath)) {
            return std::move(ptr);
        }
    }

    return nullptr;
}


}

