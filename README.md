# VFSM

## Intro

Simple Finite State Machine engine implementation over [`std::variant`](https://en.cppreference.com/w/cpp/utility/variant).

Inspired by [SML](https://github.com/qlibs/sml). But with some changes:

- More stupid and direct state declaration for the SM. That allows more predictible state transition (that work grong in the SML)
- OnExit/OnEnter support for states (transition unsupported... just for now?)
  - OnExit/OnEnter designed to call only on the actual state changes. When state make a transition to inselft no actions will be called. You noted.
  - State types still are state-less
- Polling mode for the current state with ability to make a transition. Maybe useful for some cases.

It still:
- header-only
- with small footprint
- declarative (transition table declares)
- without run-time overhead for dispatching

Other useful sources:
- [Finite State Machine with std::variant](https://www.cppstories.com/2023/finite-state-machines-variant-cpp/)
  - [Code for article](https://github.com/fenbf/articles/tree/master/cpp20/stateMachine)
- [Implementing a Real-Time State Machine in Modern C++](https://honeytreelabs.com/posts/real-time-state-machine-in-cpp/)
  - [Code for article](https://gist.github.com/rpoisel/bada82555a1b08c98f41f6e72616e50a)

And mature projects / implementations:
- [boost.msm](https://github.com/boostorg/msm)
- [boost.statechart](https://github.com/boostorg/statechart)
- [boost-ext.sml](https://github.com/boost-ext/sml)
- [Qt State Machine Framework](https://doc.qt.io/qt-5/statemachine-api.html)

## Include to project

Just copy [vfsm.hpp](https://raw.githubusercontent.com/h4tr3d/fsm_variant/refs/heads/master/vfsm/vfsm.hpp) into project.

Or use [CMake.FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html):

```cmake
include(FetchContent)

FetchContent_Declare(
  htrd.vfsm
  GIT_REPOSITORY https://github.com/h4tr3d/fsm_variant
  GIT_TAG master # replace with fixed hash, if needed
)

FetchContent_MakeAvailable(htrd.vfsm)
```

## Nano Tutorial

Simple aspects:
- States
- Events
- Actions
- Transition table

Transition dispatchin based on the types and defines at the compile type. Work are based on C++ functions overloading
by the argument types.

**State** just an empty structure. Also known as Tag. May contains sume useful static entries: Id, Name that can be used
by the user defined code, for debug of logging purposes, for example.

```c++
struct Off {};
struct On {};
```

**Events** like states, but can conatains fields, like "Guards" that can be used for state transition decision.
```c++
struct EvToggle {};
```

**Action** user-defined code that calls as a **Event** reaction for the given state. OnEntry/OnExit action can be defined too.
In top-level of view **Actions** for now just a part of **Transition table** definition.

**Transition table** central thing of the VFSM. It combines **States**, **Events** and **Actions**. It can be simple
defined according UML nonations.

Simples way to define Transition way, declare structure witho defined `auto operator()()`:
```c++
struct FsmContext {
    auto operator()() {
        return vfsm::overload {
            // Current State     Handled Event     ->     Target State         Action
            [](Off,              EvToggle)         ->     On                   { gpio_write(1); return {}; },
            [](On,               EvToggle)         ->     Off                  { gpio_write(0); return {}; }
        };
    }
};
```

Note, well-known "overload pattern" are used ([one](https://www.cppstories.com/2019/02/2lines3featuresoverload.html/), [two](https://www.modernescpp.com/index.php/visiting-a-std-variant-with-the-overload-pattern/), 
[three (look into Example section)](https://en.cppreference.com/w/cpp/utility/variant/visit#Example)).
 
`FsmContext` can contains some "state" information. Yep, it is not handled by the state types. Maybe it will be changed in the future.

Next, more complex table, with OnEntry/OnExit handling:
```c++
struct FsmContext {
    auto operator()() {
        return vfsm::overload {
            // Current State     Handled Event     ->     Target State         Action
            // <...skipped...>
            // Current State    Source/Target State       Selector Tag      
            [](Off,             auto,                     vfsm::OnEnter)       { std::puts("Switch Off"); },
            [](On,              auto,                     vfsm::OnEnter)       { std::puts("Switch On"); }     
        };
    }
};
```

**Selector Tag** can be:
- `vfsm::OnEnter`: action will be called on the Action Entry
- `vsfm::OnExit`: action will be called on the Action Exit

**Source/Target State** field matches state from (for OnEnter)/to (for OnExit) transition was occurred.

For the Inital state OnEnter action will be called with the same **Current** and **Target** state types.

Now, compose it together. Simples way, just declare alias for the State Machine Engine:
```c++
using LampSwitchFsm = vfsm::Fsm<FsmContext, Off, On>;
```

First type - Context defined about  that provides `operator()()` and transition table. Next types just an enumeration
of the states handled by the State Machine.

Next, create instance and process it:
```c++
// Pass a context (init it) and Initial state
LampSwitchFsm sm{FsmContext{}, Off{}};

while (true) {
    std::this_thread::sleep_for(1s);
    sm.processEvent(EvToggle{});
}
```

It sample just toggle Lamp in the loop.

Look over main.cpp for additional samples.

