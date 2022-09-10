#include "EventLoopPluginGLib.h"
#include "glib.h"
#include <AK/Function.h>
#include <LibWeb/Platform/Timer.h>
#include <AK/NonnullRefPtr.h>

EventLoopPluginGLib::EventLoopPluginGLib() = default;
EventLoopPluginGLib::~EventLoopPluginGLib() = default;

void EventLoopPluginGLib::spin_until(Function<bool()> goal_condition)
{
    GMainContext *c = g_main_context_default();
    while (!goal_condition()) {
        g_main_context_iteration(c, false);
    }
}

gboolean deferred_func(gpointer data)
{
    Function<void()> *allocated = (Function<void()> *)data;
    (*allocated)();
    delete allocated;
    return false;
}

void EventLoopPluginGLib::deferred_invoke(Function<void()> function)
{
    GMainContext *c = g_main_context_default();
    VERIFY(function);

    Function<void()> *allocated = new Function<void()>(move(function));

    GSource *source = g_idle_source_new();
    g_source_set_priority(source, G_PRIORITY_DEFAULT);
    g_source_set_callback(source, deferred_func, allocated, NULL);
    g_source_attach(source, c);
    g_source_unref(source);
}

class TimerGLib final : public Web::Platform::Timer {
public:
    static NonnullRefPtr<TimerGLib> create();

    virtual ~TimerGLib();

    virtual void start() override;
    virtual void start(int interval_ms) override;
    virtual void restart() override;
    virtual void restart(int interval_ms) override;
    virtual void stop() override;

    virtual void set_active(bool) override;

    virtual bool is_active() const override;
    virtual int interval() const override;
    virtual void set_interval(int interval_ms) override;

    virtual bool is_single_shot() const override;
    virtual void set_single_shot(bool) override;

private:
    TimerGLib();

    void* m_timer { nullptr };
};

NonnullRefPtr<Web::Platform::Timer> EventLoopPluginGLib::create_timer()
{
    return TimerGLib::create();
}


