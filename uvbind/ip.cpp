/*************************************************************************
 * Copyright © 2016-2022 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cpphttpx.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <cassert>

#include "ip.hpp"
#include "errors.h"

uvbind::ip::ip(const sockaddr* sa)
    : sock_addr_(*sa)
{
}

uvbind::ip::ip(ip_ver v, std::string_view str, unsigned short port)
    : port_(port)
{
	assert( !str.empty() );
	assert( v == v4 || v == v6 );
	assert( *({auto last=&str.back();++last;}) == 0x00 ); // uv_ip*_addr takes c string

	int r = 0;
	if(v == v4) r = uv_ip4_addr(&str.front(), port, (sockaddr_in*)&sock_addr_);
	else r = uv_ip6_addr(&str.front(), port, (sockaddr_in6*)&sock_addr_);
	if(r) throw error(r);
}

const sockaddr* uvbind::ip::addr() const
{
	return &sock_addr_;
}

unsigned short uvbind::ip::port() const
{
	if(port_) return *port_;
	return ntohs( ((const sockaddr_in*)&sock_addr_)->sin_port );
}
