#include <iostream>

#include "vfsm/vfsm.hpp"

using namespace std;

namespace Ev {
struct Tms { bool val{}; };
}

namespace Jtag {

// States
struct Reset        {};
struct Idle         {};
struct SelectDrScan {};
struct SelectIrScan {};
//
struct CaptureDr    {};
struct ShiftDr      {};
struct Exit1Dr      {};
struct PauseDr      {};
struct Exit2Dr      {};
struct UpdateDr     {};
//
struct CaptureIr    {};
struct ShiftIr      {};
struct Exit1Ir      {};
struct PauseIr      {};
struct Exit2Ir      {};
struct UpdateIr     {};


struct JtagContext
{
    // Jtag transition table
    constexpr auto operator()()
    {
        return vfsm::overload{
            [    ](Reset,        Ev::Tms ev) -> std::variant<Reset, Idle>             { if (ev.val) return {}; return Idle{}; },
            [    ](Idle,         Ev::Tms ev) -> std::variant<SelectDrScan, Idle>      { if (ev.val) return {}; return Idle{}; },
            // DR
            [    ](SelectDrScan, Ev::Tms ev) -> std::variant<SelectIrScan, CaptureDr> { if (ev.val) return {}; return CaptureDr{}; },
            [    ](CaptureDr,    Ev::Tms ev) -> std::variant<Exit1Dr, ShiftDr>        { if (ev.val) return {}; return ShiftDr{}; },
            [this](ShiftDr,      Ev::Tms ev) -> std::variant<Exit1Dr, ShiftDr>        { if (ev.val) return {}; feedDrBit(); return ShiftDr{}; },
            [    ](Exit1Dr,      Ev::Tms ev) -> std::variant<UpdateDr, PauseDr>       { if (ev.val) return {}; return PauseDr{}; },
            [    ](PauseDr,      Ev::Tms ev) -> std::variant<Exit2Dr, PauseDr>        { if (ev.val) return {}; return PauseDr{}; },
            [    ](Exit2Dr,      Ev::Tms ev) -> std::variant<UpdateDr, ShiftDr>       { if (ev.val) return {}; return ShiftDr{}; },
            [    ](UpdateDr,     Ev::Tms ev) -> std::variant<SelectDrScan, Idle>      { if (ev.val) return {}; return Idle{}; },
            // IR
            [    ](SelectIrScan, Ev::Tms ev) -> std::variant<Reset, CaptureIr>        { if (ev.val) return {}; return CaptureIr{}; },
            [    ](CaptureIr,    Ev::Tms ev) -> std::variant<Exit1Ir, ShiftIr>        { if (ev.val) return {}; return ShiftIr{}; },
            [this](ShiftIr,      Ev::Tms ev) -> std::variant<Exit1Ir, ShiftIr>        { if (ev.val) return {}; feedIrBit(); return ShiftIr{}; },
            [    ](Exit1Ir,      Ev::Tms ev) -> std::variant<UpdateIr, PauseIr>       { if (ev.val) return {}; return PauseIr{}; },
            [    ](PauseIr,      Ev::Tms ev) -> std::variant<Exit2Ir, PauseIr>        { if (ev.val) return {}; return PauseIr{}; },
            [    ](Exit2Ir,      Ev::Tms ev) -> std::variant<UpdateIr, ShiftIr>       { if (ev.val) return {}; return ShiftIr{}; },
            [    ](UpdateIr,     Ev::Tms ev) -> std::variant<SelectDrScan, Idle>      { if (ev.val) return {}; return Idle{}; }
        };
    }

    void feedDrBit()
    {}

    void feedIrBit()
    {}

    struct Data {};
    Data d;
};

using Fsm = vfsm::Fsm<JtagContext,
                      Reset, Idle,
                      SelectDrScan, CaptureDr, ShiftDr, Exit1Dr, PauseDr, Exit2Dr, UpdateDr,
                      SelectIrScan, CaptureIr, ShiftIr, Exit1Ir, PauseIr, Exit1Ir, UpdateIr>;

}

int main()
{
    return 0;
}
