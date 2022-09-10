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
    return false; // only call once
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
    bool timeout();

    static gboolean timer_func(gpointer data);

    int m_interval_ms { 0 };
    bool m_single_shot { false };
    bool m_active { false };
    guint m_source_active { 0xFFFFFFFF };
};

NonnullRefPtr<Web::Platform::Timer> EventLoopPluginGLib::create_timer()
{
    return TimerGLib::create();
}


NonnullRefPtr<TimerGLib> TimerGLib::create()
{
    return adopt_ref(*new TimerGLib);
}

TimerGLib::TimerGLib()
{
}

TimerGLib::~TimerGLib()
{
    if (m_active) {
        stop();
    }
}

// providing the correct signature is somehow a compilation error? great fucking job.
/* static */ gboolean TimerGLib::timer_func(gpointer data)
{
    TimerGLib *self = (TimerGLib *)data;
    return self->timeout();
}

bool TimerGLib::timeout()
{
    if (m_single_shot) {
        stop();
    }

    if (on_timeout)
        on_timeout();

    // maybe "false" could be used to stop one-shot timer? who knows?
    return true;
}


void TimerGLib::start()
{
    // the interal GTimeoutSource is _very_ similar to the public QtTimer with
    // single-shot and timeout fields.
    // but it is not included in headers so we cannot use it :(
    assert(!m_active);
    if (m_single_shot) {
        // only in glib 2.74 :(
        // m_source_active = g_timeout_add_once(m_interval_ms, timer_func, this);
        m_source_active = g_timeout_add(m_interval_ms, timer_func, this);
    } else {
        m_source_active = g_timeout_add(m_interval_ms, timer_func, this);
    }
}

void TimerGLib::start(int interval_ms)
{
    m_interval_ms = interval_ms;
    start();
}

void TimerGLib::restart()
{
    restart(interval());
}

void TimerGLib::restart(int interval_ms)
{
    if (is_active())
        stop();
    start(interval_ms);
}

void TimerGLib::stop()
{
    if (m_active) {
        g_source_remove(m_source_active);
        m_active = false;
    }
}

void TimerGLib::set_active(bool active)
{
    if (active)
        restart();
    else
        stop();
}

bool TimerGLib::is_active() const
{
    return m_active;
}

int TimerGLib::interval() const
{
    return m_interval_ms;
}

void TimerGLib::set_interval(int interval_ms)
{
    m_interval_ms = interval_ms;
}

bool TimerGLib::is_single_shot() const
{
    return m_single_shot;
}

void TimerGLib::set_single_shot(bool single_shot)
{
    m_single_shot = single_shot;
}

