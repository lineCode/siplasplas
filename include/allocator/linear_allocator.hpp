#ifndef SIPLASPLAS_LINEAR_ALLOCATOR_HPP
#define SIPLASPLAS_LINEAR_ALLOCATOR_HPP

#include <memory>
#include <ctti/type_id.hpp>
#include <stdexcept>
#include <string>
#include <sstream>

#include "allocator_utils.hpp"
#include "allocator/intrusive_allocator.hpp"

namespace cpp
{
    class LinearAllocator : IntrusiveAllocator
    {
    public:
        LinearAllocator() = default;

	LinearAllocator(char* begin, char* end);

        /**
         * Allocates a memory block of size <size> aligned to a <alignment> boundary.
         * Optionally, the returned pointer has an offset of <offset> bytes to the beginning
         * of the allocated block (After alignment)
         */
        void* allocate(std::size_t size, std::size_t alignment, std::size_t offset = 0);

        std::string dump() const;

        void deallocate(void* ptr, std::size_t count, std::size_t offset = 0);
    };
}

#endif // SIPLASPLAS_LINEAR_ALLOCATOR_HPP
