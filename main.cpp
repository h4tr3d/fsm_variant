#include <csignal>

#include <chrono>
#include <iostream>
#include <optional>
#include <thread>
#include <variant>

//
// References:
//
// - <https://www.cppstories.com/2023/finite-state-machines-variant-cpp/>
//   - <https://github.com/fenbf/articles/tree/master/cpp20/stateMachine>
//
// - <https://honeytreelabs.com/posts/real-time-state-machine-in-cpp/>
//   - <https://gist.github.com/rpoisel/bada82555a1b08c98f41f6e72616e50a>
//
// - <https://devblogs.microsoft.com/oldnewthing/20190620-00/?p=102604>
//

namespace helper {

template <class... Ts> struct overload : Ts...
{
    using Ts::operator()...;
};
// template<class... Ts> overload(Ts...)->overload<Ts...>; // no need in C++20, MSVC?
template<class... Ts> overload(Ts...) -> overload<Ts...>;
}



template <class Table, class... States>
    requires (requires (Table t) { t(); })
class Fsm
{
public:
    ~Fsm() = default;

    using StateVariant = std::variant<States...>;
    using OptionalStateVariant = std::optional<StateVariant>;

    constexpr Fsm(Table &&table, StateVariant &&initialState)
        : _table {std::move(table)},
          _state {std::move(initialState)}
    {
    }

    constexpr bool poll()
    {
        return std::visit([this](auto &&state) -> bool {
            if constexpr (requires { _state = _table()(state); }) {
                _state = _table()(state);
                return true;
            } else if constexpr (requires { std::visit([](auto&& x){}, _table()(state)); }) {
                std::visit([this](auto&& s) {
                    _state = std::move(s);
                }, _table()(state));
                return true;
            } else if constexpr (requires { _table()(state); }) {
                _table()(state);
                return true;
            } else {
                return false;
            }
        }, _state);
    }

    template <typename Event>
    constexpr bool processEvent(Event &&event)
    {
        return std::visit([this,&event](auto&& state) -> bool {
            if constexpr (requires { _state = _table()(state, event); }) {
                _state = _table()(state, event);
                return true;
            } else if constexpr (requires { std::visit([](auto&& x){},_table()(state, event)); }) {

                // iterate over resulting variants
                std::visit([this](auto&& s) {
                    _state = std::move(s);
                }, _table()(state, event));

                return true;
            } else if constexpr (requires { _table()(state, event); }) {
                _table()(state, event);
                return true;
            } else {
                return false;
            }
        }, _state);
    }

    constexpr auto visit(auto&& fn) const
    {
        return std::visit(fn, _state);
    }

private:
    Table _table;
    StateVariant _state;
};


auto main() noexcept -> int
{
    static bool is_fail = false;

    struct LocalSm
    {
        struct Init {};
        struct Run {};
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
            return helper::overload {
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
            };
        }

        Context ctx{};
    };

    using MyFsm = Fsm<LocalSm, LocalSm::Init, LocalSm::Run, LocalSm::Done, LocalSm::Fail, LocalSm::Wait>;

    MyFsm sm{LocalSm{}, LocalSm::Init{}};

    sm.processEvent(LocalSm::EvProcess{});
    sm.processEvent(LocalSm::EvProcess{});
    sm.processEvent(LocalSm::EvProcess{});

    sm.poll();
    sm.poll();
    sm.poll();

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
