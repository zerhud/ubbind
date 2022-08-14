#pragma once

/*************************************************************************
 * Copyright © 2016 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cpphttpx.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <span>
#include <array>
#include <atomic>
#include <functional>
#include <uv.h>

#include "details/uv_cvt.hpp"

namespace uvbind {

class stream {
	template<std::size_t N,typename T>
	requires( 1 <= N )
	friend struct write_request;
public:
	typedef std::function<void (uv_write_t*,int)> write_cb_t;
	typedef std::function<void (uv_connect_t*,int)> connect_cb_t;
	typedef std::function<void (uv_shutdown_t*,int)> shutdown_cb_t;
	typedef std::function<void (uv_stream_t*,int)> connection_cb_t;

	stream(const stream&)=delete;
	stream& operator = (const stream&)=delete;

	stream(stream&& other)=default;
	stream& operator = (stream&& other)=default;

	stream() =default ;
	virtual ~stream () noexcept =default ;

	void shutdown(const shutdown_cb_t& cb);

	bool is_readable() const ;
	bool is_writable() const ;
	bool is_pending() const ; ///< cannot be moved then wait for some callback

	template<typename C, typename... Bufs>
	[[nodiscard]] auto start_write(C&& cb, Bufs&&... bufs)
	{
		using arr_t = std::array<uv_buf_t, sizeof...(Bufs)>;
		struct writer {
			C cb;
			arr_t uv_bufs;
			uv_write_t req;

			writer(const writer&) = delete ;
			writer& operator = (const writer&) = delete ;
			writer(writer&&) = delete ;
			writer& operator = (writer&&) = delete ;

			writer(C&& cb, arr_t b, uv_stream_t* s)
			    : cb(std::forward<C>(cb))
			    , uv_bufs(std::move(b))
			{
				req.data = this;
				int r = uv_write(&req, s, uv_bufs.data(), uv_bufs.size(),
				[](uv_write_t* req, int st){
				    writer* self = (writer*)req->data;
				    if(st < 0) details::throw_error(st);
				    assert(self != nullptr);
				    self->cb(); });
				if(r) details::throw_error(r);
			}
			~writer()
			{
			}
		};
		return writer{
			std::forward<C>(cb),
			arr_t{ details::make_buf(std::forward<Bufs>(bufs))... },
			get_stream()
		};
	}
protected:

	virtual uv_stream_t* get_stream() const =0 ;

	template<typename inner_t, typename allocator>
	auto start_read(allocator* alloc)
	{
		struct reader {
			allocator* alloc = nullptr;
			uv_stream_t* uvs = nullptr;

			reader() =default ;

			reader(const reader&) =delete ;
			reader& operator = (const reader&) =delete ;

			reader& operator = (reader&& other)
			{
				alloc = other.alloc;
				uvs = other.uvs;
				other.uvs = nullptr;
				return *this;
			}

			reader(reader&& other) noexcept
			    : alloc(other.alloc)
			    , uvs(other.uvs)
			{
				other.uvs = nullptr;
			}

			reader(allocator* a, uv_stream_t* s)
			    : alloc(a)
			    , uvs(s)
			{
				if(int r=uv_read_start(uvs, read_alloc_cb<inner_t>, read_cb<inner_t>);r)
					details::throw_error(r);
			}
			~reader() noexcept {
				if(uvs != nullptr) uv_read_stop(uvs);
				uvs = nullptr;
			}
		};

		return reader(alloc, get_stream());
	}

	template<typename inner_handler>
	static void read_alloc_cb(uv_handle_t* handle, size_t size, uv_buf_t* sbuf)
	{
		assert(sbuf);
		assert(handle);
		if(0 < size) inner_handler::allocate_uv_buf(handle, *sbuf);
		else {
			sbuf->base = nullptr;
			sbuf->len = 0;
		}
	}

	template<typename inner_handler>
	static void read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
	{
		assert(stream);
		auto* dt = (inner_handler*)(stream);
		if(0 <= nread) inner_handler::call_read_cb(stream, std::span<std::byte>((std::byte*)buf->base, nread));
	}
};

template<std::size_t N, typename T=std::string_view>
requires ( 1<= N )
struct write_request {
	typedef std::function<void()> finish_cb_t;
	typedef write_request<N, T> self_type;
private:
	finish_cb_t cb;
	uv_write_t req;
	std::array<uv_buf_t, N> uv_bufs;
	std::atomic<std::size_t> count=0;

	template<typename B>
	uv_buf_t make_buf(const B& data)
	{
		uv_buf_t ret;
		if constexpr (std::is_const_v<std::decay_t<B>>)
		    ret.base = (char*)const_cast<B*>(data.data());
		else
		    ret.base = (char*)data.data();
		ret.len = data.size();
		return ret;
	}
public:

	write_request(finish_cb_t cb) : write_request({}, std::move(cb)) {}

	template<typename con>
	write_request(con&& bufs, finish_cb_t cb)
	    : cb(std::move(cb))
	{
		req.data = this;
		for(std::size_t i=0;i<bufs.size();++i)
			if(!bufs[i].empty())
				uv_bufs[i] = make_buf(bufs[i]);
	}

	~write_request() noexcept
	{
		assert( count == 0 );
	}

	auto operator[](std::size_t i)
	{
		assert( i < N );
		struct proxy {
			self_type* self;
			std::size_t ind;
			proxy& operator = (T&& obj)
			{
				self->uv_bufs[ind] = self->make_buf(obj);
				return *this;
			}
		};

		return proxy{ this, i };
	}

	void operator()(stream& s)
	{
		++count;
		uv_write(&req, s.get_stream(), uv_bufs.data(), uv_bufs.size(),
		         [](uv_write_t* req, int st){
			write_request* self = (write_request*)req->data;
			assert(self->count != 0);
			--self->count;
			if(st < 0) details::throw_error(st);
			assert(self != nullptr);
			self->cb();
		});
	}
};

} // namespace uvbind
