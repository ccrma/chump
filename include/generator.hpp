// // taken from
// // https://gist.github.com/cocoa-xu/553dafc8585f4583ecec3ccfc1aedad0

// #if __has_include(<coroutine>)
// #  include <coroutine>
// #  define has_coroutine 1
// #elif __has_include(<experimental/coroutine>)
// #  include <experimental/coroutine>
// #  define has_coroutine 1
// #  define has_experimental_coroutine 1
// #else
// #  error "no coroutine support found"
// #endif
// #include <concepts>
// #include <functional>
// #include <iostream>
// #include <map>
// #include <memory>
// #include <sstream>
// #include <thread>

// #pragma once

// // Based on https://www.scs.stanford.edu/~dm/blog/c++-coroutines.html#generic-generator-example
// // Added clean up functionality
// // T: Required;
// //   Type of the value yield to the user.
// // MIN_EVAL: Optional;
// //   Minimal number of evaluation times.
// //   Default 0.
// //   Usually set to 1 if you want to set up a cleanup function.
// template<typename T, size_t MIN_EVAL=0>
// struct generator {
//     struct promise_type;
//     struct iterator;
//     struct cleanup_callback {
//         std::function<void()> cleanup;
//     };
//     struct cancel_control {
//         std::shared_ptr<bool>& cancel;
//     };

// #ifdef has_coroutine
// #ifdef has_experimental_coroutine
//     using handle_type = std::experimental::coroutine_handle<promise_type>;
//     using suspend_always = std::experimental::suspend_always;
//     using suspend_never = std::experimental::suspend_never;
// #else
//     using handle_type = std::coroutine_handle<promise_type>;
//     using suspend_always = std::suspend_always;
//     using suspend_never = std::suspend_never;
// #endif
// #endif

//     struct promise_type {
//         T value_;
//         std::exception_ptr exception_;
//         // add a cleanup_ function
//         std::function<void()> cleanup_;
//         std::shared_ptr<bool> cancel_requested;
//         promise_type() {
//             cancel_requested = std::make_shared<bool>(false);
//         }

//         Generator get_return_object() {
//             return Generator(handle_type::from_promise(*this));
//         }
//         suspend_always initial_suspend() { return {}; }
//         suspend_always final_suspend() noexcept(true) { return {}; }
//         void unhandled_exception() { exception_ = std::current_exception(); }

//         template<typename From>
//         requires std::is_convertible_v<T, From>
//         suspend_always yield_value(From &&from) {
//             value_ = std::forward<From>(from);
//             return {};
//         }

//         // overload the yield_value function
//         // now we can pass a clean_up_callback to co_yield to set up cleanup callback
//         suspend_never yield_value(cleanup_callback&& token) {
//             cleanup_ = token.cleanup;
//             return {};
//         }

//         suspend_never yield_value(cancel_control&& cancellation) {
//             cancellation.cancel = cancel_requested;
//             return {};
//         }
//         void return_void() {};
//     };

//     struct iterator : std::iterator<std::input_iterator_tag, T> {
//         Generator* g_;
//         size_t index = -1;
//         iterator(Generator* g, size_t index = -1) {
//             this->index = index;
//             g_ = g;
//         }
//         iterator& operator++() {
//             if (g_ != nullptr && g_->h_ != nullptr) {
//                 g_->fill();
//                 if (g_->h_.done()) {
//                     index = -1;
//                 } else {
//                     index++;
//                 }
//             }
//             return *this;
//         }

//         T operator*() const {
//             return std::move(g_->operator()());
//         }

//         bool operator!=(const iterator& other) const {
//             return this->g_ != other.g_ || this->index != other.index;
//         }
//     };

//     iterator begin() {
//         if (discarded_ || (h_ && h_.done())) return end();
//         return iterator(this, 0);
//     }

//     iterator end() {
//         return iterator(this);
//     }

//     handle_type h_;
//     size_t num_evaluates = 0;

//     Generator(handle_type h) : h_(h) {}
//     ~Generator() {
//         if (num_evaluates < MIN_EVAL) {
//             std::ostringstream msg;
//             msg << "Generator[" << std::hex << this << "]: Minimal required number of evaluations is " << MIN_EVAL
//                 << " times, however, this generator has only been evaluated " << num_evaluates << " times.\n";
//             std::cerr << msg.str();
//         }
//         stop();
//     }
//     explicit operator bool() {
//         if (discarded_) return false;
//         fill();
//         return !h_.done();
//     }
//     T operator()() {
//         if (discarded_) {
//             throw std::runtime_error("Generator has been discarded.");
//         }
//         fill();
//         full_ = false;
//         return std::move(h_.promise().value_);
//     }

//     // This function forces the coroutine to stop (without notifying it)
//     void stop() {
//         if (!discarded_) {
//             // check if the clean up function was assigned or not
//             if (h_.promise().cleanup_) {
//                 // do clean up
//                 h_.promise().cleanup_();
//             }
//             h_.destroy();
//             discarded_ = true;
//         }
//     }

//     // This function only requests the coroutine to stop instead of force it to stop
//     void request_stop() {
//         if (!discarded_) {
//             *h_.promise().cancel_requested = true;
//         }
//     }

// private:
//     bool discarded_ = false;
//     bool full_ = false;

//     void fill() {
//         if (!full_ && !discarded_) {
//             num_evaluates++;
//             h_();
//             if (h_.promise().exception_) {
//                 std::rethrow_exception(h_.promise().exception_);
//             }
//             full_ = true;
//         }
//     }
// };

#include <concepts>
#include <coroutine>
#include <exception>
#include <iostream>
#include <utility>

#pragma once

template<typename T>
struct Generator {
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
    template<std::convertible_to<T> From> // C++20 concept
    std::suspend_always yield_value(From &&from) {
      value_ = std::forward<From>(from);
      return {};
    }
    // void return_void() {}

    template<std::convertible_to<T> From>
    void return_value(From &&from) {
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
      if (h_) h_.destroy();
      h_ = std::exchange(other.h_, {});
      full_ = other.full_;
    }
    return *this;
  }
  
  ~Generator() { if (h_) h_.destroy(); }
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
