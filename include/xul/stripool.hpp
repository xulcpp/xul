#ifndef _xul_stripool_hpp_
#define _xul_stripool_hpp_

#include <atomic>
#include <cstddef>
#include <cstdint>

namespace xul {

/// Memory pool that uses "strips" of memory that it cycles through when one
/// strip cannot provide the memory requested. Each strip only keeps track of:
/// - Number of active acquisitions
/// - Where the next acquisition can occur from
///
/// When acquisitions are released, only the active acquisition count is
/// decremented, and the head remains untouched. But, if the acquisition count
/// becomes 0, the head is reset, making the entirety of the strip available
/// again.
/// This scheme makes releases pretty much O(1), and acquisitions are at worst
/// O(n), where n is the number of strips, where the operation is an atomic check
/// of the strip's head to see if the acquistion will fit.
///
/// Concurrent acquisition and releasing is supported, and is lock free as long
/// as std::atomic_uint32_t is atomic.
///
/// The intended use case for this pool is to acquire memory for ephemeral
/// objects. The strip size will be determined by the largest object. The number
/// of strips is determined by how many objects are expected to be allocated
/// at once. This is typically determined through empirical measumrent. Failure
/// to acquire a resource is not an error.
///
/// # Limitations
///
/// An atomic uint32_t bitfield is used at the start of each strip to keep track
/// of the number of allocations in the strip, and where the next allocation in
/// the strip should occur. This current scheme imposes the following restrictions:
/// - The maximum strip size is 16'777'216 (2^24) - sizeof(std::max_align_t),
///   as the lower 24-bits are used to track where the next acquisition can
///   occur, and always acounts for bookkeeping block (the sizeof(std::max_align_t)).
/// - Maximum allocations per strip is 255 (2^8-1), as the upper 8 bits count
///   the number of allocations in the strip.
///
/// # Future Improvements
///
/// These are things that should probably be tackled:
/// - Provide more configuration:
///   - Atmoic type to use for bookkeeping
///   - Number of bits for counter / head, allowing more smaller allocations
///     per strip, or fewer larger allocations per strip.
struct Stripool
{
  [[nodiscard]] char* acquire(std::uint32_t requested) noexcept {

    // We start at the last accessed strip, and keep track of the number of
    // strips interrogated, giving up if we interrogate all strips.
    std::size_t stripIdx = currentStrip_.load(std::memory_order_relaxed);
    size_t interrogated = 0;

    requested += sizeof(StripPtr);

    // 1. Call helper to get strip* for stripIdx
    // 2. Load count and head
    // 3. If the _required_ will fit, attempt to exchange with updated count and head
    //   3a. If exchanged, return aligned pointer
    //   3b. If fail, check and try again
    // 4. If won't fit, advance to next strip
    //   4a. If all strips interrogated, return nullptr
    StripHdr* strip = stripAt(stripIdx % stripCount_);
    while ( true ) {
      // 1., 2.
      uint32_t countAndHead = strip->countAndHead.load(std::memory_order_relaxed);

      // Asking for too big an allocation is a logic error for the intended
      // use cases of this allocator, so we avoid a check up front. An impossible
      // to fill allocation will ultimately fail due to interrogated == stripCount_
      const auto head = countAndHead & head_mask_;

      if ( head + requested > stripSize_ ) {
        // 4.
        ++interrogated;
        if (interrogated >= stripCount_) {
          // 4.a
          return nullptr;
        }
        ++stripIdx;
        strip = stripAt(stripIdx % stripCount_);
      } else {
        // 3.
        const auto padding = alignof(StripPtr) - (requested % alignof(StripPtr));
        if ( head + padding < stripSize_ ) {
          requested += padding;
        }
        const uint32_t update = countAndHead + count_inc_ + requested;
        const bool exchanged = strip->countAndHead.compare_exchange_weak(
          countAndHead,
          update,
          std::memory_order_release,
          std::memory_order_relaxed);
        if ( exchanged ) {
          // 3a.
          char* ret = reinterpret_cast<char*>(strip) + head;
          reinterpret_cast<StripPtr*>(ret)->strip = strip;
          ret += sizeof(StripPtr);
          // Since we could allocate from this strip, we're pretty likely to be
          // allocating from it next time, so store it, not caring if another
          // concurrent allocation also wants to set it.
          currentStrip_.store(stripIdx, std::memory_order_relaxed);
          return ret;
        } else {
          // 3b. continue
        }
      }
    }
  }

  static void release(char* mem) {
    // Preceeding the *mem* is a pointer the strip it was acquired from.
    StripPtr* ptr = reinterpret_cast<StripPtr*>(mem - sizeof(StripPtr));
    StripHdr* s = ptr->strip;
    std::uint32_t expectedCountAndHead = s->countAndHead.load(std::memory_order_relaxed);
    std::uint32_t desiredCountAndHead;
    do {
      desiredCountAndHead = expectedCountAndHead - count_inc_;
      if ( (desiredCountAndHead & count_mask_) == 0 ) {
        // If this exchange is successful, then count would be zero, therefore
        // all acquisitions have been released from the strip, so we get to
        // reset the strip back to pristine state.
        desiredCountAndHead = sizeof(StripHdr);
      }
    } while (!s->countAndHead.compare_exchange_weak(
        expectedCountAndHead,
        desiredCountAndHead,
        std::memory_order_release,
        std::memory_order_relaxed));
  }

private:
  /// Each strip consists of a header with the count and head atomic, and is
  /// padded out to the first acquisition. The header type is made available
  /// to subclasses, as they must ensure that the strip pointer provided at
  /// construction accounts for the the header size.
  struct alignas(std::atomic<std::uint32_t>) StripHdr {
    std::atomic_uint32_t countAndHead;
    // Alignof header, since the StripPtr is aligned as such, since that's its first member.
    [[no_unique_address]] char _pad[alignof(StripHdr*) - sizeof(countAndHead)];
  };

  // Every acquisition is prefixed with a pointer back to the strip, and is padded
  // out to max alignment.
  struct alignas(StripHdr*) StripPtr {
    StripHdr* strip;
    [[no_unique_address]] char _pad[alignof(std::max_align_t) - sizeof(strip)];
  };

  // Each strip uses a u32 bookkeeping bitfield to keep track of both the allocation
  // *count* and where the *head* of the strip is for the next acqusition.

  // The allocation count is stored in the upper 8 bits, so a mask and shift
  // are handy to have, as well as a constant for incrementing the count.
  static constexpr size_t count_mask_ = 0xFF000000;
  static constexpr size_t count_shift_ = 24; // Amount to shift count and head to get just the count
  static constexpr size_t count_inc_ = 0x01000000;

  // The head is stored in the lower 24 bits, and we only need a mask for our
  // operations on it.
  static constexpr size_t head_mask_ = 0x00FFFFFF;

  static_assert(sizeof(StripHdr) == alignof(StripHdr*));
  static_assert(sizeof(StripPtr) == alignof(std::max_align_t));

  StripHdr* stripAt(const std::size_t i) {
    return reinterpret_cast<StripHdr*>(stripMem_ + (i * stripSize_));
  }

  const std::size_t stripSize_;
  const std::size_t stripCount_;
  char* stripMem_;
  std::atomic<std::size_t> currentStrip_;

protected:
  /// Create a stripool that assumes the *stripMem* consists of *stripCount*
  /// strips, each of *rawStripSize* in length. This size must include any space
  /// reserved for a StripHdr and StripPtr.
  constexpr Stripool(std::size_t rawStripSize, std::size_t stripCount, char* stripMem) noexcept
    : stripSize_{rawStripSize}, stripCount_{stripCount} , stripMem_{stripMem}, currentStrip_{0}
  {
    // Strip heads are self-relative, so are initialised and reset to the size
    // of a `strip`, which is the per-strip management data.
    for ( std::size_t i = 0; i < stripCount; ++i ) {
      stripAt(i)->countAndHead = sizeof(StripHdr);
   }
  }

  static constexpr std::size_t striphdr_size = sizeof(StripHdr);
  static constexpr std::size_t stripptr_size = sizeof(StripPtr);
};



/// Stripool that is backed by a statically sized array that can hold
/// *strip_count_* strips, each capable of holding at least 1 *strip_size_*
/// byte allocation. The actual strip size will be sized larger to account
/// for mandatory padding and bookkeeping so that at least 1 acquisition of
/// *strip_size_* will always succed per strip.
template <std::size_t strip_size_, std::size_t strip_count_>
struct ArrayStripool : public Stripool
{
  ArrayStripool() : Stripool{raw_strip_size(), strip_count_, memory_} {}

  // For testing purposes
  const void* memory() const { return memory_; }

  static consteval std::size_t raw_strip_size() {
    std::size_t size = strip_size_ + striphdr_size + stripptr_size;
    size += size % alignof(std::max_align_t);
    return size;
  }

private:
  alignas(std::max_align_t) char memory_[raw_strip_size() * strip_count_];
};

}

#endif
