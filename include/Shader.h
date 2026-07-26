namespace Shader {
    void TintScenegraph(RE::NiAVObject* a_obj, const RE::NiColorA& a_color_1, const RE::NiColorA& a_color_2);
    void RemoveTintScenegraph(RE::NiAVObject* a_obj);

    const RE::NiColorA pickableFillColor{0.0f, 0.5f, 1.0f, 0.3f};
    const RE::NiColorA pickableRimColor{0.0f, 0.5f, 1.0f, 0.5f};

    const RE::NiColorA hoverFillColor{0.0f, 0.3f, 2.0f, 0.3f};
    const RE::NiColorA hoverRimColor{0.0f, 0.3f, 2.0f, 0.8f};

    const RE::NiColorA selectedFillColor{0.15f, 0.85f, 0.25f, 0.3f};
    const RE::NiColorA selectedRimColor{0.25f, 1.0f, 0.35f, 0.8f};
    bool IsMovable(const RE::ObjectRefHandle& handle);
    void ApplyPickableHighlight(const RE::ObjectRefHandle& handle);
    void ApplyHoverHighlight(const RE::ObjectRefHandle& handle);
    void ApplySelectedHighlight(const RE::ObjectRefHandle& handle);
    void RefreshReferenceHighlight(const RE::ObjectRefHandle& handle);
    void QueueReferenceHighlight(
        const RE::ObjectRefHandle& handle,
        RE::NiAVObject* loaded3D);
    void QueueReferenceRelease(const RE::ObjectRefHandle& handle);
    void QueueClearAllReferenceHighlights();
    void ProcessPendingReferenceChanges();
    void ClearReferenceHighlight(const RE::ObjectRefHandle& handle);
    void ClearAllReferenceHighlights();
    std::unordered_map<RE::FormID, RE::ObjectRefHandle> GetHighlightedReferences();
}
