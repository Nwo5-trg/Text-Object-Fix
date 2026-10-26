#pragma once

#include <nwo5.silly-api/include/cache/include.hpp>

using namespace nwo5::cache::prelude;

namespace settings {
    inline SettingCache<bool> enabled{"enabled"};
    inline SettingCache<bool> insertNewlines{"insert-newlines"};
    inline SettingCache<bool> inputBypass{"input-bypass"};
}