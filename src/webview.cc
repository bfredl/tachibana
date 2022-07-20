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
#include <LibCore/EventLoop.h>
#include <LibCore/System.h>
#include <LibGfx/Bitmap.h>
#include <LibGfx/Font/FontDatabase.h>
#include <LibGfx/ImageDecoder.h>
#include <LibGfx/PNGWriter.h>
#include <LibGfx/Rect.h>

#include "webview.h"


String s_serenity_resource_root = [] {
    auto const* source_dir = getenv("SERENITY_SOURCE_DIR");
    if (source_dir) {
        return String::formatted("{}/Base", source_dir);
    }
    auto* home = getenv("XDG_CONFIG_HOME") ?: getenv("HOME");
    VERIFY(home);
    return String::formatted("{}/.lagom", home);
}();

// Taken from SerenityOS/ladybird sources
class HeadlessImageDecoderClient : public Web::ImageDecoding::Decoder {
public:
    static NonnullRefPtr<HeadlessImageDecoderClient> create()
    {
        return adopt_ref(*new HeadlessImageDecoderClient());
    }

    virtual ~HeadlessImageDecoderClient() override = default;

    virtual Optional<Web::ImageDecoding::DecodedImage> decode_image(ReadonlyBytes data) override
    {
        auto decoder = Gfx::ImageDecoder::try_create(data);

        if (!decoder)
            return Web::ImageDecoding::DecodedImage { false, 0, Vector<Web::ImageDecoding::Frame> {} };

        if (!decoder->frame_count())
            return Web::ImageDecoding::DecodedImage { false, 0, Vector<Web::ImageDecoding::Frame> {} };

        Vector<Web::ImageDecoding::Frame> frames;
        for (size_t i = 0; i < decoder->frame_count(); ++i) {
            auto frame_or_error = decoder->frame(i);
            if (frame_or_error.is_error()) {
                frames.append({ {}, 0 });
            } else {
                auto frame = frame_or_error.release_value();
                frames.append({ move(frame.image), static_cast<size_t>(frame.duration) });
            }
        }

        return Web::ImageDecoding::DecodedImage {
            decoder->is_animated(),
            static_cast<u32>(decoder->loop_count()),
            frames,
        };
    }

private:
    explicit HeadlessImageDecoderClient() = default;
};

class RequestManagerWebView : public Web::ResourceLoaderConnector {
public:
    static NonnullRefPtr<RequestManagerWebView> create()
    {
        return adopt_ref(*new RequestManagerWebView());
    }

    virtual ~RequestManagerWebView() override { }

    virtual void prefetch_dns(AK::URL const&) override { }
    virtual void preconnect(AK::URL const&) override { }

    virtual RefPtr<Web::ResourceLoaderConnectorRequest> start_request(String const& method, AK::URL const&, HashMap<String, String> const& request_headers, ReadonlyBytes request_body, Core::ProxyData const&) override;

private:
    RequestManagerWebView();

    class Request
        : public Web::ResourceLoaderConnectorRequest {
    public:
        static ErrorOr<NonnullRefPtr<Request>> create(String const& method, AK::URL const& url, HashMap<String, String> const& request_headers, ReadonlyBytes request_body, Core::ProxyData const&);

        virtual ~Request() override;

        virtual void set_should_buffer_all_input(bool) override { }
        virtual bool stop() override { return false; }
        virtual void stream_into(Core::Stream::Stream&) override { }

        void did_finish();
    };

    HashMap<void*, NonnullRefPtr<Request>> m_pending;
};

RequestManagerWebView::RequestManagerWebView()
{
}

RefPtr<Web::ResourceLoaderConnectorRequest> RequestManagerWebView::start_request(String const& method, AK::URL const& url, HashMap<String, String> const& request_headers, ReadonlyBytes request_body, Core::ProxyData const& proxy)
{
    auto proto = url.protocol();
    fprintf(stderr, "PROTO: %s\n", proto.characters());
    if (!url.protocol().is_one_of_ignoring_case("http"sv, "https"sv)) {
        return nullptr;
    }

    // TODO
    return nullptr;
}
// Taken from SerenityOS/ladybird sources
void initialize_web_engine()
{
    Web::ImageDecoding::Decoder::initialize(HeadlessImageDecoderClient::create());
    Web::ResourceLoader::initialize(RequestManagerWebView::create());
    // Web::WebSockets::WebSocketClientManager::initialize(HeadlessWebSocketClientManager::create());


    Web::FrameLoader::set_default_favicon_path(String::formatted("{}/res/icons/16x16/app-browser.png", s_serenity_resource_root));
    dbgln("Set favicon path to {}", String::formatted("{}/res/icons/16x16/app-browser.png", s_serenity_resource_root));

    Gfx::FontDatabase::set_default_fonts_lookup_path(String::formatted("{}/res/fonts", s_serenity_resource_root));

    Gfx::FontDatabase::set_default_font_query("Katica 10 400 0");
    Gfx::FontDatabase::set_fixed_width_font_query("Csilla 10 400 0");

    Web::FrameLoader::set_error_page_url(String::formatted("file://{}/res/html/error.html", s_serenity_resource_root));
}

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
        fprintf(stderr, "PTHAN: %s\n", request->path().characters());
        auto const file = Core::System::open(request->path(), O_RDONLY);
        fprintf(stderr, "YERROR: %d\n", file.is_error());
        fprintf(stderr, "YVALUE: %d\n", file.value());
        request->on_file_request_finish(file);
    }

    HeadlessBrowserPageClient()
       : m_page(make<Web::Page>(*this))
    {
    }

    void load(AK::URL const& url)
    {
        if (!url.is_valid())
            return;

         m_page->load(url);
    }


    NonnullOwnPtr<Web::Page> m_page;
    RefPtr<Gfx::PaletteImpl> m_palette_impl;
    Gfx::IntRect m_viewport_rect { 0, 0, 800, 600 };
    Web::CSS::PreferredColorScheme m_preferred_color_scheme { Web::CSS::PreferredColorScheme::Auto };
};

int main() {
  Core::EventLoop event_loop;
  initialize_web_engine();
  auto client = HeadlessBrowserPageClient();
  client.load(AK::URL("file:///home/bfredl/dev/zig/lib/docs/index.html"));
}
