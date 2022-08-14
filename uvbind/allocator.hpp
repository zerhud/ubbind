#pragma once

/*************************************************************************
 * Copyright © 2022 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cpphttpx.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <array>
#include <memory_resource>

namespace uvbind {

template<std::size_t asize = 4096>
class allocator_static {
	std::array<std::byte, asize> buffer;
	std::pmr::memory_resource* mem;
public:
	allocator_static(allocator_static&& other) : mem(other.mem) {}
	allocator_static& operator = (allocator_static&& other)
	{
		mem = other.mem;
		other.mem = nullptr;
		return *this;
	}

	allocator_static(const allocator_static& other) : mem(other.mem) {}
	allocator_static& operator = (const allocator_static& other)
	{
		mem = other.mem;
		return *this;
	}

	allocator_static(std::pmr::memory_resource* mem = std::pmr::get_default_resource())
	    : mem(mem)
	{}

	void allocate_uv_buf(uv_buf_t& buf) {
		buf.base = (char*)buffer.data();
		buf.len = buffer.size();
	}

	template<typename T, typename D, typename... Args>
	requires( requires(const D& d, T* p){ d(p); } )
	auto allocate(D d, Args&&... args)
	{
		struct holder {
			void* ptr = nullptr;
			std::pmr::memory_resource* mem = nullptr;
			D deleter;
			T* object = nullptr;

			holder(holder&&) =default ;
			holder& operator = (holder&&) =default ;

			holder(const holder&) =delete ;
			holder& operator = (const holder&) =delete ;

			holder() =default ;
			holder(std::pmr::memory_resource* mem, D d)
			    : ptr(mem->allocate(sizeof(T), alignof(T)))
			    , mem(mem)
			    , deleter(std::move(d))
			{}
			~holder()
			{
				if(object != nullptr) deleter(object);
				if(mem) mem->deallocate(ptr, sizeof(T), alignof(T));
			}

			T* get() { return object; }
			const T* get() const{ return object; }
			T* operator -> () { return object; }
		};

		holder h(mem, std::move(d));
		h.object = new (h.ptr) T(std::forward<Args>(args)...);
		return h;
	}
};

} // namespace uvbind
