
#include <LibWeb/Platform/EventLoopPlugin.h>

class EventLoopPluginGLib final : public Web::Platform::EventLoopPlugin {
public:
    EventLoopPluginGLib();
    virtual ~EventLoopPluginGLib() override;

    virtual void spin_until(Function<bool()> goal_condition) override;
    virtual void deferred_invoke(Function<void()>) override;
    virtual NonnullRefPtr<Web::Platform::Timer> create_timer() override;
};
