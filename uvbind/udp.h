#pragma once

/*************************************************************************
 * Copyright © 2022 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cpphttpx.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <span>
#include <array>
#include <string_view>

#include "ip.hpp"
#include "handle.h"
#include "loop.h"
#include "details/uv_cvt.hpp"
#include "details/inner_handler.hpp"

namespace uvbind {

struct udp_message {
	ip addr;
	std::span<const std::byte> data;
	bool is_chunk = false;

	std::string_view tosv() const
	{
		return std::string_view{ reinterpret_cast<const char*>(data.data()), data.size() };
	}
};

template<typename allocator_t, typename read_callback_t>
class udp : public handle {
	using self_type = udp<allocator_t, read_callback_t>;
	using inner_handler_t = details::inner_handler<self_type, uv_udp_t>;
	friend inner_handler_t;

	allocator_t allocator;
	read_callback_t read_cb;

	auto allocate_handler()
	{
		return allocator.template allocate<inner_handler_t>([](inner_handler_t*){}, this);
	}

	decltype( std::declval<self_type>().allocate_handler() ) hndl;

	uv_handle_t* get_handle() const override { return (uv_handle_t*)hndl.get(); }

	static void on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
	{
		assert(buf);
		assert(handle);
		if(0 < suggested_size) inner_handler_t::allocate_uv_buf(handle, *buf);
		else {
			buf->base = nullptr;
			buf->len = 0;
		}
	}

	static void on_recv(
	        uv_udp_t* handle,
	        ssize_t nread,
	        const uv_buf_t* buf,
	        const struct sockaddr* addr,
	        unsigned flags)
	{
		if(nread == 0) return;
		uvbind::ip from = addr == nullptr
		        ? uvbind::ip("0.0.0.0", 0)
		        : uvbind::ip(addr)
		        ;
		std::span<const std::byte> data((const std::byte*)buf->base, nread);
		udp_message msg{from, std::move(data), false};
		inner_handler_t::call_read_cb(handle, msg);
	}
public:
	udp(allocator_t alloc, loop_ptr loop, const ip& ip, read_callback_t rcb)
	    : handle(std::move(loop))
	    , allocator(std::move(alloc))
	    , read_cb(std::move(rcb))
	    , hndl(allocate_handler())
	{
		uv_udp_init(bound_loop(), hndl.get());
		uv_udp_bind(hndl.get(), ip, 0);
		uv_udp_recv_start(hndl.get(), on_alloc, on_recv);
	}

	~udp() noexcept
	{
		uv_close( (uv_handle_t*)hndl.get(), [](uv_handle_t*){} );
		hndl->self = nullptr;
	}

	template<typename callback_t, typename... data_t>
	requires( 0 < sizeof...(data_t) )
	[[nodiscard("the write process will be canceled")]]
	auto send(const uvbind::ip& dest, callback_t&& cb, data_t&&... data)
	{
		using arr_t = std::array<uv_buf_t, sizeof...(data_t)>;
		struct writer {
			callback_t cb;
			arr_t uv_bufs;
			uv_udp_send_t req;

			writer(writer&&) =delete ;
			writer(const writer&) =delete ;

			writer& operator = (writer&&) =delete ;
			writer& operator = (const writer&) =delete ;

			writer(callback_t cb, arr_t b, const uvbind::ip& dest, uv_udp_t* c)
			    : cb(std::forward<callback_t>(cb))
			    , uv_bufs(std::move(b))
			{
				req.data = this;
				int r=uv_udp_send(&req, c, uv_bufs.data(), uv_bufs.size(), dest, send_cb);
				throw_uv_error(r);
			}

			static void send_cb(uv_udp_send_t* req, int status)
			{
				if(!req) return;
				writer* self = (writer*)req->data;
				assert(self != nullptr);
				self->cb();
			}
		};

		return writer{
			std::forward<callback_t>(cb),
			arr_t{ details::make_buf(std::forward<data_t>(data))... },
			dest,
			hndl.get()
		};
	}
};

} // namespace uvbind
