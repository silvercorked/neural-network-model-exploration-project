#pragma once

// https://stackoverflow.com/questions/58700542/overload-a-lambda-function#answer-58700664

// lambda's are functors, function objects, basically a struct with an defined () operator
// This struct takes in a collection of these functors and inherets their operator() definitions
// which, so long as each F in Fs is unique in signature, results in an object with many definitions
// to handle many cases, as if it was an Overloaded Lambda functor
// The first line does almost all of that, while the second is a deduction guide

template <class... Fs> struct OverloadedLambda : Fs... { using Fs::operator()...; };
template <class... Fs> OverloadedLambda(Fs...) -> OverloadedLambda<Fs...>;

/*
    Normal std::visit on std::variant<Fs...> 'v':
    std::visit([](auto&& a) {
        using T = std::decay_t<decltype(a)>;
        if constexpr (std::is_same_v<T, F>)
            // what to do for type F
        else if constexpr ...
        else {
            static_assert(false, "non-exhaustive visitor!")
        }
    }, v);
    With OverloadedLambda:
    std::visit(
        OverloadedLambda {
            [](F a) { // what to do for type of F // ...; }
            .... // no else case
        }
    , v);
*/
