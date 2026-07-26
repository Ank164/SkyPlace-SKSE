#pragma once

namespace ObjectGroup {
    struct Data;
}

namespace Hooks {
    void Install();
    bool CacheGroupModel(const ObjectGroup::Data& group);
    void RefreshLoadedReferenceHighlights();
    void RetireGroupModel(std::string_view meshPath);
    std::size_t GetGroupModelCacheSize();
    void VisitGroupModelCache(const std::function<void(std::string_view)>& visitor);
}
