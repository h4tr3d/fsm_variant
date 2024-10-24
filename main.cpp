#include <cstdio>

#include "vfsm/vfsm.hpp"

auto main() noexcept -> int
{
    static bool is_fail = false;

    struct LocalSm
    {
        struct Init {};
        struct Run  {};
        struct Fail {};
        struct Done {};
        struct Wait {};

        // Events for Event Driven mode
        struct EvProcess {};
        struct EvReset   {};

        struct Context
        {
            int counter = 0;
        };

        // Transition table
        constexpr auto operator()()
        {
            return vfsm::overload {
                [this](Init, EvProcess) -> Run  { std::puts("init"); ctx = {}; return {}; },
                [this](Run,  EvProcess) -> std::variant<Run, Done, Fail> {
                std::puts("run");
                if (++ctx.counter == 5) {
                    if (is_fail) return Fail{};
                    return Done{};
                }
                return {};
            },
                [](Done, EvProcess) -> Done { std::puts("done"); return {}; },
                [](Fail, EvProcess) -> Fail { std::puts("fail"); return {}; },
                // Any State event processing
                [](auto, EvReset)   -> Init { return {}; },

                // Run State polling CB. Return value deternmine new state. Maybe void to keep state
                [](Run)                     { std::puts("Run State Poll"); },

                // OnEnter/OnExit cases
                [](Init,       vfsm::OnEnter) { std::puts("++ Init onEnter"); },
                [](Init, auto, vfsm::OnExit)  { std::puts("-- Init onExit"); },
                [](Run,  auto, vfsm::OnEnter) { std::puts("++ Run onEnter"); },
                [](Run,  auto, vfsm::OnExit)  { std::puts("-- Run onExit"); },
                [](auto, auto, vfsm::OnEnter) { std::puts("++ Generic onEnter"); },
                [](auto, auto, vfsm::OnExit)  { std::puts("-- Generic onExit"); }
            };
        }

        Context ctx{};
    };

    using MyFsm = vfsm::Fsm<LocalSm, LocalSm::Init, LocalSm::Run, LocalSm::Done, LocalSm::Fail, LocalSm::Wait>;

    MyFsm sm{LocalSm{}, LocalSm::Init{}};

    sm.processEvent(LocalSm::EvProcess{});
    sm.processEvent(LocalSm::EvProcess{});
    sm.processEvent(LocalSm::EvProcess{});

    // Poll only in the Run state
    sm.visit(vfsm::overload{
        [&](LocalSm::Run) { sm.poll(); },
        [](auto) {} // fallback
    });
    sm.visit([&](auto s) {
        if constexpr (std::is_same_v<LocalSm::Run, decltype(s)>) {
            sm.poll();
        }
    });

    sm.processEvent(LocalSm::EvProcess{});
    sm.processEvent(LocalSm::EvProcess{});
    sm.processEvent(LocalSm::EvProcess{});
    sm.processEvent(LocalSm::EvProcess{});
    sm.processEvent(LocalSm::EvProcess{});
    sm.processEvent(LocalSm::EvProcess{});

    is_fail = true;
    //sm = MyFsm{LocalSm{}, LocalSm::Init{}};
    sm.processEvent(LocalSm::EvReset{});

    sm.processEvent(LocalSm::EvProcess{});
    sm.processEvent(LocalSm::EvProcess{});
    sm.processEvent(LocalSm::EvProcess{});
    sm.processEvent(LocalSm::EvProcess{});
    sm.processEvent(LocalSm::EvProcess{});
    sm.processEvent(LocalSm::EvProcess{});
    sm.processEvent(LocalSm::EvProcess{});
    sm.processEvent(LocalSm::EvProcess{});
    sm.processEvent(LocalSm::EvProcess{});

    return 0;
}
