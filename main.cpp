#include <cstdio>
#include <typeinfo>
#include <format>

#include "vfsm/vfsm.hpp"

namespace Ev {
// Events for Event Driven mode
struct Process {};
struct Reset   {};
}

namespace Local {

// States
struct Init { };
struct Run  { };
struct Fail { };
struct Done { };
struct Wait { };


struct FsmContext
{
    struct Context
    {
        int counter = 0;
        bool is_fail = false;
    };

    // Transition table
    constexpr auto operator()()
    {
        return vfsm::overload {
            [this](Init, Ev::Process) -> Run  { std::puts("init"); return {}; },
            [this](Run,  Ev::Process) -> std::variant<Run, Done, Fail> {
            std::puts("run");
            if (++ctx.counter == 5) {
                if (ctx.is_fail) return Fail{};
                return Done{};
            }
            return {};
        },
            [](Done, Ev::Process) -> Done { std::puts("done"); return {}; },
            [](Fail, Ev::Process) -> Fail { std::puts("fail"); return {}; },
            // Any State event processing
            [](auto, Ev::Reset)   -> Init { return {}; },

            // Run State polling CB. Return value deternmine new state. Maybe void to keep state
            [](Run)                     { std::puts("== Run State Poll"); },

            // OnEnter/OnExit cases
            [this](Init, auto, vfsm::OnEnter) { std::puts("++ Init onEnter"); ctx = {}; },
            [    ](Init, auto, vfsm::OnExit)  { std::puts("-- Init onExit"); },
            [    ](Run,  auto, vfsm::OnEnter) { std::puts("++ Run onEnter"); },
            [    ](Run,  auto, vfsm::OnExit)  { std::puts("-- Run onExit"); },
            [    ](auto, auto, vfsm::OnEnter) { std::puts("++ Generic onEnter"); },
            [    ](auto, auto, vfsm::OnExit)  { std::puts("-- Generic onExit"); }
        };
    }

    Context ctx{};
};

// Declare machine
using Fsm = vfsm::Fsm<FsmContext, Init, Run, Fail, Done, Wait>;

};


auto main() noexcept -> int
{

    Local::Fsm sm{Local::FsmContext{}, Local::Init{}};

    sm.processEvent(Ev::Process{});
    sm.processEvent(Ev::Process{});
    sm.processEvent(Ev::Process{});

    // Poll only in the Run state
    sm.visit(vfsm::overload{
        [&](Local::Run) { sm.poll(); },
        [](auto) {} // fallback
    });

    sm.visit([&](auto& s) {
        if constexpr (std::is_same_v<Local::Run, std::remove_cvref_t<decltype(s)>>) {
            sm.poll();
        }
    });

    sm.processEvent(Ev::Process{});
    sm.processEvent(Ev::Process{});
    sm.processEvent(Ev::Process{});
    sm.processEvent(Ev::Process{});
    sm.processEvent(Ev::Process{});
    sm.processEvent(Ev::Process{});

    //
    // Two ways to reset state machine
    //

    std::puts("\nMake MachineReset 1: no current state OnExit()\n");

    sm = Local::Fsm{Local::FsmContext{}, Local::Init{}};
    sm.context().ctx.is_fail = true;

    sm.processEvent(Ev::Process{});
    sm.processEvent(Ev::Process{});
    sm.processEvent(Ev::Process{});
    sm.processEvent(Ev::Process{});
    sm.processEvent(Ev::Process{});
    sm.processEvent(Ev::Process{});
    sm.processEvent(Ev::Process{});
    sm.processEvent(Ev::Process{});
    sm.processEvent(Ev::Process{});


    std::puts("\nMake MachineReset 2 :: with current state OnExit()\n");

    sm.processEvent(Ev::Reset{});
    sm.context().ctx.is_fail = true;

    sm.processEvent(Ev::Process{});
    sm.processEvent(Ev::Process{});
    sm.processEvent(Ev::Process{});
    sm.processEvent(Ev::Process{});
    sm.processEvent(Ev::Process{});
    sm.processEvent(Ev::Process{});
    sm.processEvent(Ev::Process{});
    sm.processEvent(Ev::Process{});
    sm.processEvent(Ev::Process{});

    return 0;
}
