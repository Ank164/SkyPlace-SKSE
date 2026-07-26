#pragma once

namespace UI {
    void Register();
    namespace MainWindow {
        void __stdcall Render();
    }
    namespace ConfigWindow {
        void __stdcall Render();
    }
    namespace HealthWindow {
        void __stdcall RenderDynamicForms();
        void __stdcall RenderRetainedResources();
    }
};
