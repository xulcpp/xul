#include <xul/stripool.hpp>

#include <gtest/gtest.h>

#include <thread>
#include <cstdio>
#include <ranges>
#include <cstring>

TEST(Stripool, X)
{
  xul::ArrayStripool<16, 3> pool;
  static_assert(pool.raw_strip_size() % alignof(std::max_align_t) == 0);
  auto acq1 = pool.acquire(16);
  EXPECT_NE(acq1, nullptr);
  // The pointer isn't aligned as I'd expect. The StripHdr is 8-bytes not 16.
  // Probably better off just doing the correct alignment.
  EXPECT_EQ(acq1, (char*)pool.memory() + 8 + 16) << "Difference is: " << std::distance<char*>((char*)pool.memory(), acq1);
  std::fill_n(acq1, 16, 'a');

  auto acq2 = pool.acquire(16);
  EXPECT_NE(acq2, nullptr);
  std::fill_n(acq2, 16, 'b');

  auto acq3 = pool.acquire(16);
  EXPECT_NE(acq3, nullptr);
  std::fill_n(acq3, 16, 'c');

  auto acq4 = pool.acquire(16);
  EXPECT_EQ(acq4, nullptr);

  // Release acquisition 1, but don't clear the pointer. Since this was the
  // only acquisition from the first strip, that strip should be available again,
  // and therefore we get the same pointer back.
  pool.release(acq1);
  {
    auto acq = pool.acquire(16);
    EXPECT_EQ(acq, acq1);
  }
}

TEST(Stripool, MultipleAllocationsPerStrip)
{
  // Hold just under 3 maximally aligned objects per strip. Each allocation
  // has a max aligned pointer preceeding it, so this will only allow 2
  // per strip.
  xul::ArrayStripool<sizeof(std::max_align_t) * 3 - 1, 3> pool;
  for ( int goes = 0; goes < 10; ++goes ) {
    std::vector<char*> acqs;
    for ( int i = 0; i < 6; ++i ) {
      acqs.push_back(pool.acquire(sizeof(std::max_align_t)));
    }
    for ( auto acq : acqs ) {
      EXPECT_NE(nullptr, acq);
    }
    // Arbitrary amount of subsequent acquisitions should all fail.
    for ( int i = 0; i < 1000; ++i ) {
      ASSERT_EQ(nullptr, pool.acquire(1));
    }
    for ( auto acq : acqs ) {
      pool.release(acq);
    }
  }

}

bool fillbo_faggins(xul::Stripool& pool, int threadId)
{
  std::array<char, 8> expected{};
  expected.fill(threadId);

  for (int runs = 0; runs != 1'000'000; ++runs ) {
    char* acq = nullptr;
    while (!acq) {
      std::this_thread::yield();
      acq = pool.acquire(8);
    }
    // Repeatedly fill the acquisition with our thread ID
    for ( int fillAttempt = 0; fillAttempt < 1'000; ++fillAttempt ) {
      std::fill_n(acq, 8, threadId);
      for ( int checkAttempt = 0; checkAttempt < 1'000'000; ++checkAttempt ) {
        if ( std::memcmp(acq, expected.data(), 8) != 0 ) {
          ::printf("Overlap!\n");
          ::fflush(stdout);
          pool.release(acq);
          return true;
        }
      }
    }
    pool.release(acq);
  }
  return false;
}

// Not recommended to run this on debug. It takes forever.
TEST(Stripool, Swarm)
{
  xul::ArrayStripool<32, 12> pool;
  bool overlap{false};

  std::vector<std::thread> threads;
  for ( unsigned threadId = 0; threadId < 24; ++threadId ) {
    threads.push_back(std::thread{[threadId, &overlap, &pool]{
      overlap = overlap || fillbo_faggins(pool, threadId);
    }});
  }

  for ( auto& thread: threads ) {
    thread.join();
  }
}
