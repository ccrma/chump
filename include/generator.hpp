// taken from
// https://gist.github.com/cocoa-xu/553dafc8585f4583ecec3ccfc1aedad0
#include <concepts>
#include <coroutine>
#include <exception>
#include <iostream>
#include <utility>

#pragma once

template <typename T> struct Generator {
  struct promise_type;
  using handle_type = std::coroutine_handle<promise_type>;

  struct promise_type {
    T value_;
    std::exception_ptr exception_;

    Generator get_return_object() {
      return Generator(handle_type::from_promise(*this));
    }
    std::suspend_always initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void unhandled_exception() { exception_ = std::current_exception(); }
    template <std::convertible_to<T> From> // C++20 concept
    std::suspend_always yield_value(From &&from) {
      value_ = std::forward<From>(from);
      return {};
    }
    // void return_void() {}

    template <std::convertible_to<T> From> void return_value(From &&from) {
      value_ = std::forward<From>(from);
    }
  };

  handle_type h_;

  Generator(handle_type h) : h_(h) {}
  Generator(const Generator &) = delete;
  Generator &operator=(const Generator &) = delete;
  // move constructor
  Generator(Generator &&other) noexcept
      : h_(std::exchange(other.h_, {})), full_(other.full_) {}

  Generator &operator=(Generator &&other) noexcept {
    if (this != &other) {
      if (h_)
        h_.destroy();
      h_ = std::exchange(other.h_, {});
      full_ = other.full_;
    }
    return *this;
  }

  ~Generator() {
    if (h_)
      h_.destroy();
  }
  explicit operator bool() {
    fill();
    return !h_.done();
  }
  T operator()() {
    fill();
    full_ = false;
    return std::move(h_.promise().value_);
  }

private:
  bool full_ = false;

  void fill() {
    if (!full_) {
      h_();
      if (h_.promise().exception_)
        std::rethrow_exception(h_.promise().exception_);
      full_ = true;
    }
  }
};
