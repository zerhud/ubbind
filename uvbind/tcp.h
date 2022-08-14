#pragma once

/*************************************************************************
 * Copyright © 2016-2022 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cpphttpx.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <span>
#include <thread>
#include <chrono>
#include <optional>
#include <memory_resource>
#include <string_view>

#include "ip.hpp"
#include "stream.h"
#include "handle.h"
#include "details/inner_handler.hpp"
#include "details/uv_cvt.hpp"

namespace uvbind {

template<class,class>
class tcp_acceptor;

class tcp : public stream {
	template<class A,class B>
	friend class tcp_acceptor;
public:
	std::string peername() const ;
	std::string sockname() const ;
	void keepalive(bool enable, std::chrono::seconds delay);
protected:
	void manualy_init_tcp_socket(uv_loop_t* l) ;
	virtual uv_tcp_t* tcp_handle() const =0 ; //TODO: uv_tcp_t* must be to const object
};

class tcp_recive : public tcp,
        public callbackable_handle
            < tcp_recive
            , uv_tcp_t
            , uv_tcp_init
            ,std::function<void (tcp_recive*,int)>
            >
{
public:
	/// @param backlog indicates the number of connections the kernel might queue
	/// (same as http://linux.die.net/man/2/listen)
	tcp_recive(loop_ptr l, const ip& ip, int backlog, const callback_type& cb);
protected:
	uv_tcp_t* tcp_handle() const override ;
	uv_stream_t* get_stream() const override ;
};


template<typename buf_allocator, typename read_cb_t>
class tcp_acceptor  : public tcp, public handle
{
	using self_type = tcp_acceptor<buf_allocator, read_cb_t>;
	using inner_handler_t = details::inner_handler<self_type, uv_tcp_t>;
	using reader_t = decltype( std::declval<self_type>().template start_read<
	    inner_handler_t, buf_allocator>( nullptr ) );
	friend inner_handler_t;

	buf_allocator allocator;
	auto allocate_handler()
	{
		return allocator.template allocate<inner_handler_t>(
		            [](inner_handler_t* p){ /*uv_close((uv_handle_t*)p, [](uv_handle_t*){});*/ },
		            this
		            );
	}
	using handler_ptr = decltype( std::declval<self_type>().allocate_handler() );

	handler_ptr hndl;
	read_cb_t read_cb;
	std::optional<reader_t> reader;
public:
	tcp_acceptor(const uvbind::tcp_recive* server, buf_allocator alloc, read_cb_t cb)
	    : handle(server->owned_loop())
	    , allocator(std::move(alloc))
	    , hndl(allocate_handler())
	    , read_cb(std::move(cb))
	{
		uv_tcp_init( bound_loop(), hndl.get() );
		const tcp& srv = *server;
		if(int r=uv_accept((uv_stream_t*)srv.tcp_handle(), (uv_stream_t*)tcp_handle());r)
			throw_uv_error(r);
		reader.emplace(this->start_read<inner_handler_t>(&allocator));
	}
	~tcp_acceptor() noexcept
	{
		reader.reset();
		uv_close( (uv_handle_t*)hndl.get(), [](uv_handle_t*){} );
		hndl->self = nullptr;
	}
protected:
	uv_tcp_t* tcp_handle() const override { return (uv_tcp_t*)hndl.get(); }
	uv_stream_t* get_stream() const override { return (uv_stream_t*)hndl.get(); }
	uv_handle_t* get_handle() const override { return (uv_handle_t*)hndl.get(); }
};

template<typename buf_allocator, typename tcp_connect_cb_t, typename read_cb_t>
requires(
        requires(){ std::declval<tcp_connect_cb_t>()((int)1); }
     && requires(){ std::declval<read_cb_t>()(std::span<std::byte>{}); }
     && requires(buf_allocator& a, uv_buf_t& buf){ a.allocate_uv_buf(buf); }
        )
class tcp_connect : public tcp, public handle
{
	using self_type = tcp_connect<buf_allocator,tcp_connect_cb_t,read_cb_t>;
    using inner_handler_t = details::inner_handler<self_type, uv_tcp_t>;
    using reader_t = decltype( std::declval<self_type>().template start_read<
        inner_handler_t, buf_allocator>( nullptr ) );
    friend inner_handler_t;

	buf_allocator allocator;
	auto allocate_handler()
	{
		return allocator.template allocate<inner_handler_t>(
		            [](inner_handler_t* p){ /*uv_close((uv_handle_t*)p, [](uv_handle_t*){});*/ },
		            this
		);
	}

	using handler_ptr = decltype( std::declval<self_type>().allocate_handler() );

	read_cb_t read_cb;
	tcp_connect_cb_t connect_cb;
	handler_ptr own_handler;
	uv_connect_t req_;
	std::optional<reader_t> reader;
public:
	tcp_connect(const tcp_connect&) =delete;
	tcp_connect& operator = (const tcp_connect&) =delete;
	tcp_connect(tcp_connect&&) =delete;
	tcp_connect& operator = (tcp_connect&&) =delete;

	tcp_connect(
	            loop_ptr l,
	            const ip& ip,
	            tcp_connect_cb_t&& cb,
	            read_cb_t&& read_cb,
	            buf_allocator alloc
	            )
	    : handle(std::move(l))
	    , allocator(std::move(alloc))
	    , read_cb(std::move(read_cb))
	    , connect_cb(std::move(cb))
	{
		own_handler =  allocate_handler( );

		int r=uv_tcp_init(bound_loop(), static_cast<uv_tcp_t*>(own_handler.get()));
		if(r) throw_uv_error(r);

		using namespace std::literals;

		keepalive(true, 60s);

		r=uv_tcp_connect(&req_,static_cast<uv_tcp_t*>(own_handler.get()), ip, st_connect_cb);
		if(r) throw_uv_error(r);

		own_handler.get()->data = nullptr;
		reader.emplace(this->start_read<inner_handler_t>(&allocator));
	}

	~tcp_connect() noexcept
	{
		reader.reset();
		uv_close((uv_handle_t*)(own_handler.get()), [](uv_handle_t*){});
		own_handler->self = nullptr;
		std::this_thread::yield();
	}
protected:
	uv_handle_t* get_handle() const override
	{
		return (uv_handle_t*)own_handler.get();
	}

	uv_tcp_t* tcp_handle() const override
	{
		return (uv_tcp_t*)(own_handler.get());
	}

	uv_stream_t* get_stream() const override
	{
		return (uv_stream_t*)(own_handler.get());
	}
private:

	static void st_connect_cb(uv_connect_t* req, int status)
	{
		assert(req && req->handle);
		self_type* self = inner_handler_t::try_extract_self(req->handle);
		if(self) self->connect_cb(status);
	}
};

} // namespace uvbind
