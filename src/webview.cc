#include <LibWeb/Bindings/MainThreadVM.h>
#include <LibWeb/Cookie/ParsedCookie.h>
#include <LibWeb/DOM/Document.h>
#include <LibWeb/Dump.h>
#include <LibWeb/HTML/BrowsingContext.h>
#include <LibWeb/HTML/Scripting/ClassicScript.h>
#include <LibWeb/HTML/Storage.h>
#include <LibWeb/HTML/Window.h>
#include <LibWeb/ImageDecoding.h>
#include <LibWeb/Layout/InitialContainingBlock.h>
#include <LibWeb/Loader/ResourceLoader.h>
#include <LibWeb/Page/Page.h>
#include <LibWeb/Painting/PaintableBox.h>
#include <LibWeb/Painting/StackingContext.h>
#include <LibWeb/WebSockets/WebSocket.h>
#include <LibCore/System.h>

#include "webview.h"

class HeadlessBrowserPageClient final : public Web::PageClient {
public:

    virtual Gfx::Palette palette() const override
    {
        return Gfx::Palette(*m_palette_impl);
    }

    virtual Gfx::IntRect screen_rect() const override
    {
        // FIXME: Return the actual screen rect.
        return m_viewport_rect;
    }

    virtual Web::CSS::PreferredColorScheme preferred_color_scheme() const override
    {
        return m_preferred_color_scheme;
    }

    void request_file(NonnullRefPtr<Web::FileRequest>& request) override
    {
        auto const file = Core::System::open(request->path(), O_RDONLY);
        request->on_file_request_finish(file);
    }

    RefPtr<Gfx::PaletteImpl> m_palette_impl;
    Gfx::IntRect m_viewport_rect { 0, 0, 800, 600 };
    Web::CSS::PreferredColorScheme m_preferred_color_scheme { Web::CSS::PreferredColorScheme::Auto };
};
