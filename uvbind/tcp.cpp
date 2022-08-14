/*******************************************************************************
 * Copyright © 2016 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 *
 * This file is part of cpphttpx
 *
 * CPPHTTPX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * CPPHTTPX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with cpphttpx (copying file in the root of this repository). If not,
 * see <http://www.gnu.org/licenses/>
 * *****************************************************************************/

#include <cassert>
#include <sstream>

#include "tcp.h"
#include "errors.h"

using namespace std::literals;

std::string uvbind::tcp::peername() const
{
	struct sockaddr_storage addr;
	int addr_len=sizeof(addr);

	uv_tcp_t* h = tcp_handle(); assert( h );
	int r=uv_tcp_getpeername(h,(struct sockaddr*)&addr,&addr_len);
	if(r) throw error(r);

	char buffer[INET6_ADDRSTRLEN];
	r=getnameinfo((struct sockaddr*)&addr,addr_len,buffer,sizeof(buffer),0,0,NI_NUMERICHOST);
	if(r) throw binder_error("cannor obtaine getnameinfo with error "+std::to_string(r));

	return std::string(&buffer[0],INET6_ADDRSTRLEN);
}

std::string uvbind::tcp::sockname() const
{
	struct sockaddr_storage addr;
	int addr_len=sizeof(addr);

	uv_tcp_t* h = tcp_handle(); assert( h );
	int r=uv_tcp_getsockname(h,(struct sockaddr*)&addr,&addr_len);
	if(r) throw error(r);

	char buffer[INET6_ADDRSTRLEN];
	r=getnameinfo((struct sockaddr*)&addr,addr_len,buffer,sizeof(buffer),0,0,NI_NUMERICHOST);
	if(r) throw binder_error("cannor obtaine getnameinfo with error "+std::to_string(r));

	return std::string(&buffer[0],INET6_ADDRSTRLEN);
}

void uvbind::tcp::keepalive(bool enable, std::chrono::seconds delay)
{
	uv_tcp_t* h = tcp_handle(); assert( h );
	auto r = uv_tcp_keepalive(h,enable,delay.count());
	if(r) throw error(r);
}

void uvbind::tcp::manualy_init_tcp_socket(uv_loop_t* l)
{
	uv_tcp_init(l, tcp_handle());
}

uvbind::tcp_recive::tcp_recive(
        uvbind::loop_ptr l,
        const uvbind::ip& ip,
        int backlog,
        const callback_type& cb)
    : callbackable_handle(l,cb)
{
	int r=uv_tcp_bind(holder_.get(), ip, 0);
	if(r) throw error(r);

	r=uv_listen((uv_stream_t*)holder_.get(),backlog,[](uv_stream_t* server, int status){
		auto& dt=uv_callback((uv_type*)server);
		uv_callback((uv_tcp_t*)server).cb(dt.self,status);
	});
	if(r) throw error(r);
}

uv_tcp_t* uvbind::tcp_recive::tcp_handle() const
{
	return holder_.get();
}

uv_stream_t* uvbind::tcp_recive::get_stream() const
{
	return (uv_stream_t*)holder_.get();
}
