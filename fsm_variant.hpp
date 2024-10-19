#pragma once

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
// Copyright 2024 Alexander Drozdov <adrozdoff@gmail.com>
// Author: Alexander Drozdov <adrozdoff@gmail.com>
//
// Distributed under MIT license.
//

#include <variant>

namespace fsm_variant {

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
template <class Table, class... States>
    requires (requires (Table t) { t(); })
class Fsm
{
public:
    ~Fsm() = default;

    using StateVariant = std::variant<States...>;

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


} // fsm_variant
