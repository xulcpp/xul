#ifndef _xul_stripool_pmr_hpp_
#define _xul_stripool_pmr_hpp_

#include "stripool.hpp"

#include <memory_resource>

namespace xul {

struct StripoolMemoryResource : public std::pmr::memory_resource
{
  StripoolMemoryResource(Stripool& pool) : pool_{pool} {}

  ~StripoolMemoryResource() override{}


private:
  void* do_allocate(std::size_t bytes, std::size_t /*alignment*/) override {
    return pool_.acquire(bytes);
  }

  void do_deallocate(void* p, std::size_t bytes, std::size_t alignment ) override {
    return pool_.release(static_cast<char*>(p));
  }

  bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
    /// @todo Any stripool knows how to deallocate any other stripool's allocations,
    /// so used a dynamic_cast check if RTTI is enabled.
    return &other == this;
  }

  Stripool& pool_;
};

}

#endif
