#pragma once

#include <coroutine>
#include <cstdlib>

template<typename T>
struct Generator {
    struct promise_type {
        T value;
        std::suspend_always yield_value(T v) { value = v; return {}; }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        Generator get_return_object() {
            return Generator{ std::coroutine_handle<promise_type>::from_promise(*this) };
        }
        void return_void() {}
        void unhandled_exception() { std::exit(1); }
    };

    std::coroutine_handle<promise_type> h;
    Generator(std::coroutine_handle<promise_type> h) : h(h) {}
    ~Generator() { if (h) h.destroy(); }

    struct iterator {
        std::coroutine_handle<promise_type> h;
        bool done;

        iterator(std::coroutine_handle<promise_type> h_, bool done_) : h(h_), done(done_) {}
        iterator& operator++() { h.resume(); done = h.done(); return *this; }
        T operator*() const { return h.promise().value; }
        bool operator!=(const iterator& other) const { return done != other.done; }
    };

    iterator begin() { h.resume(); return { h, h.done() }; }
    iterator end() { return { h, true }; }
};