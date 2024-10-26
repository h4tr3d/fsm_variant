//
// Distributed under MIT license.
//
// Copyright 2024 Alexander Drozdov <adrozdoff@gmail.com>
// Author: Alexander Drozdov <adrozdoff@gmail.com>
// Site:   https://github.com/h4tr3d/fsm_variant/
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

#pragma once

#include <variant>

namespace vfsm {

//
// Overload pattern
// Ref:
// - https://www.cppstories.com/2019/02/2lines3featuresoverload.html/
// - https://www.modernescpp.com/index.php/visiting-a-std-variant-with-the-overload-pattern/
//
template <class... Ts> struct overload : Ts...
{
    using Ts::operator()...;
};
// template<class... Ts> overload(Ts...)->overload<Ts...>; // no need in C++20, MSVC?
template<class... Ts> overload(Ts...) -> overload<Ts...>;

struct OnEnter {};
struct OnExit  {};

/**
 * Simple Finite State Machine implementation using C++20 features and std::variant
 *
 * Use declarative transition pattern and both polling and event driven behavior.
 *
 * Sample:
 * ```
 * bool run = true;
 *
 * struct Idle   {};
 * struct Run    {};
 * struct Finish {};
 *
 * struct EvStart {};
 * struct EvStop  {};
 * struct EvReset {};
 *
 * struct FsmTable {
 *   constexpr auto operator()() {
 *     return fsm_variant::overload{
 *       // Events processing
 *       [](Idle, EvStart) -> Run    { std::puts("Idle -> Run"); run = true; return {}; },
 *       [](Run,  EvStop)  -> Finish { std::puts("Run -> Finish"); return {}; },
 *       // Any state processing
 *       [](auto, EvReset) -> Idle   { std::puts("current -> Idle"); return {}; },
 *
 *       // Polling for the Run state
 *       [](Run) -> std::variant<Run, Finish> { if (!run) return Finish{}; return {}; }
 *     };
 *   }
 * };
 *
 * using MyFsm = fsm_variant::Fsm<FsmTable, Idle, Run, Finish>;
 *
 * MyFsm sm = {FsmTable{}, Idle{}};
 *
 * sm.processEvent(EvStart{}); // Run state now
 *
 * sm.visit([&](auto s) {
 *   if constexpr (std::is_same_v<Run, decltype(s)>) {
 *     sm.poll();
 *   }
 * });
 *
 * sm.processEvent(EvStop{});
 * sm.processEvent(EvReset{});
 *
 * ```
 *
 */
template <class TableContext, class... States>
    requires (requires (TableContext t) { t(); })
class Fsm
{
public:
    ~Fsm() = default;

    using StateVariant = std::variant<States...>;

    constexpr Fsm(TableContext &&table, StateVariant &&initialState)
        : _context {std::move(table)},
          _state {std::move(initialState)}
    {
        // Handle initalState onEnter here
        std::visit([this](auto&& s) {
            if constexpr (requires { _context()(s, s, OnEnter{}); }) {
                _context()(s, s, OnEnter{});
            } else if constexpr (requires { _context()(s, OnEnter{}); }) {
                _context()(s, OnEnter{});
            }
        }, _state);
    }

    constexpr bool poll()
    {
        return std::visit([this](auto&& state) -> bool {
            if constexpr (requires { _state = _context()(state); }) {
                auto newState = _context()(state);
                handleOnExitEnter(newState);
                _state = std::move(newState);
                return true;
            } else if constexpr (requires { std::visit([](auto&&){}, _context()(state)); }) {
                std::visit([this](auto&& newState) {
                    handleOnExitEnter(newState);
                    _state = std::move(newState);
                }, _context()(state));
                return true;
            } else if constexpr (requires { _context()(state); }) {
                _context()(state);
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
            if constexpr (requires { _state = _context()(state, event); }) {
                auto newState = _context()(state, std::forward<Event>(event));
                handleOnExitEnter(newState);
                _state = std::move(newState);
                return true;
            } else if constexpr (requires { std::visit([](auto&&){},_context()(state, event)); }) {
                // iterate over resulting variants
                std::visit([this](auto&& newState) {
                    handleOnExitEnter(newState);
                    _state = std::move(newState);
                }, _context()(state, std::forward<Event>(event)));

                return true;
            } else if constexpr (requires { _context()(state, event); }) {
                _context()(state, std::forward<Event>(event));
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

    constexpr auto visit(auto&& fn)
    {
        return std::visit(fn, _state);
    }

    auto const& context() const
    {
        return _context;
    }

    auto& context()
    {
        return _context;
    }

private:
    template<typename NewStateType>
    void handleOnExitEnter(NewStateType& newState)
    {
        // onExit/onEnter
        if (std::holds_alternative<NewStateType>(_state) == false) {
            std::visit([this,&newState](auto&& currentState) {
                // process current state onExit
                if constexpr (requires { _context()(currentState, newState, OnExit{}); }) {
                    _context()(currentState, newState, OnExit{});
                } else if constexpr (requires { _context()(currentState, OnExit{}); }) {
                    _context()(currentState, OnExit{});
                }

                // process new state onEnter
                if constexpr (requires { _context()(newState, currentState, OnEnter{}); }) {
                    _context()(newState, currentState, OnEnter{});
                } else if constexpr (requires { _context()(newState, OnEnter{}); }) {
                    _context()(newState, OnEnter{});
                }

            }, _state);
        }
    }

private:
    TableContext _context;
    StateVariant _state;
};


} // fsm_variant
