#pragma once

#include "json/types.h"

namespace json {

enum class PipeResult : u8 {
  kNoConsumption = 0,
  kSuccess = 1,
  kFailure = 2,
  kContinue = 3
};

template <typename UnaryPredicate>
class Pipe {
 public:
  constexpr explicit Pipe(int sz, UnaryPredicate&& up)
      : n_(0), fixed_(sz), up_(std::move(up)) {}

  constexpr explicit Pipe(int sz, const UnaryPredicate& up)
      : n_(0), fixed_(sz), up_(up) {}

  constexpr virtual PipeResult operator()(u8 data) {
    if (!this->up_(data)) {
      return PipeResult::kFailure;
    }

    if (++n_ == fixed_) {
      return PipeResult::kSuccess;
    }

    return PipeResult::kContinue;
  }

  // Reset the count.
  constexpr void reset() { n_ = 0; }

  constexpr virtual bool can_skip_consumption() const { return false; }

 private:
  int n_;
  const int fixed_;
  UnaryPredicate up_;
};

template <typename UnaryPredicate>
class PipeOne : public Pipe<UnaryPredicate> {
 public:
  using Base = Pipe<UnaryPredicate>;

  constexpr explicit PipeOne(UnaryPredicate&& up) : Base(1, std::move(up)) {}
  constexpr explicit PipeOne(const UnaryPredicate& up) : Base(1, up) {}

  constexpr PipeResult operator()(u8 data) override {
    return Base::operator()(data);
  }
};

template <typename UnaryPredicate>
class PipeZeroOrOne : public PipeOne<UnaryPredicate> {
 public:
  using Base = PipeOne<UnaryPredicate>;
  using Base::Base;

  constexpr PipeResult operator()(u8 data) override {
    if (Base::operator()(data) == PipeResult::kFailure) {
      return PipeResult::kNoConsumption;
    }

    return PipeResult::kSuccess;
  }

  constexpr bool can_skip_consumption() const override { return true; }
};

template <typename Ret, typename... Args>
Pipe(int, Ret(Args...)) -> Pipe<Ret (*)(Args...)>;

template <typename Ret, typename... Args>
PipeOne(Ret(Args...)) -> PipeOne<Ret (*)(Args...)>;

template <typename Ret, typename... Args>
PipeZeroOrOne(Ret(Args...)) -> PipeZeroOrOne<Ret (*)(Args...)>;

}  // namespace json