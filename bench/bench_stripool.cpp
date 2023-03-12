#include <nanobench.h>

#include <xul/stripool.hpp>

namespace {

using namespace ankerl::nanobench;

const auto bench1 = Bench{}
.epochs(1'000)
.run("Stripool::acquire", []{
  xul::ArrayStripool<32, 4> pool;
  doNotOptimizeAway(pool.acquire(32));
});

auto bench2 = Bench{}
.epochs(1'000)
.run("Stripool::acquire-release", []{
  xul::ArrayStripool<32, 4> pool;
  auto mem = pool.acquire(32);
  pool.release(mem);
});

const auto bench3 = Bench{}
.epochs(1'000)
.run("Stripool::acquire all", []{
  xul::ArrayStripool<32, 4> pool;
  doNotOptimizeAway(pool.acquire(32));
  doNotOptimizeAway(pool.acquire(32));
  doNotOptimizeAway(pool.acquire(32));
  doNotOptimizeAway(pool.acquire(32));
});

const auto bench4 = Bench{}
.epochs(1'000)
.run("Stripool::acquire-release all", []{
  xul::ArrayStripool<32, 4> pool;
    auto mem1 = pool.acquire(32);
    auto mem2 = pool.acquire(32);
    auto mem3 = pool.acquire(32);
    auto mem4 = pool.acquire(32);
    pool.release(mem1);
    pool.release(mem2);
    pool.release(mem3);
    pool.release(mem4);
});

// @todo This benchmark assumes alignof(std::max_align_t) == 16
const auto bench5 = Bench{}
.epochs(1'000)
.run("Stripool::multiple acquire", []{
  xul::ArrayStripool<32, 4> pool;
  doNotOptimizeAway(pool.acquire(8));
  doNotOptimizeAway(pool.acquire(8));

  doNotOptimizeAway(pool.acquire(8));
  doNotOptimizeAway(pool.acquire(8));

  doNotOptimizeAway(pool.acquire(8));
  doNotOptimizeAway(pool.acquire(8));

  doNotOptimizeAway(pool.acquire(8));
  doNotOptimizeAway(pool.acquire(8));
});

const auto bench6 = Bench{}
.epochs(1'000)
.run("Stripool::multiple acquire-release", []{
  xul::ArrayStripool<32, 4> pool;
  auto mem1 = pool.acquire(8);
  auto mem2 = pool.acquire(8);
  auto mem3 = pool.acquire(8);
  auto mem4 = pool.acquire(8);
  auto mem5 = pool.acquire(8);
  auto mem6 = pool.acquire(8);
  auto mem7 = pool.acquire(8);
  auto mem8 = pool.acquire(8);

  pool.release(mem1);
  pool.release(mem2);
  pool.release(mem3);
  pool.release(mem4);
  pool.release(mem5);
  pool.release(mem6);
  pool.release(mem7);
  pool.release(mem8);
});

}
