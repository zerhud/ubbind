#pragma once

/*************************************************************************
 * Copyright © 2016-2022 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cpphttpx.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <optional>
#include <string_view>

#include <uv.h>

namespace uvbind {

class ip {
public:
	enum ip_ver { v4, v6 };

	ip(const sockaddr* sa);

	ip(std::string_view str, unsigned short port) : ip(v4, str, port) {}

	ip(ip_ver v, std::string_view str, unsigned short port);

	const sockaddr* addr() const;

	inline operator const sockaddr* () const
	{
		return addr();
	}

	unsigned short port() const;

	template<typename string, typename... args_t>
	requires( sizeof(typename string::value_type) == sizeof(char) )
	string to_str(args_t&&... args) const
	{
		string ret(std::forward<args_t>(args)...);
		ret.resize(128);
		uv_ip_name(&sock_addr_, ret.data(), ret.size());
		auto null_pos = ret.find((typename string::value_type)0x00);
		if(null_pos != string::npos)
			ret.resize(null_pos);
		return ret;
	}
private:
	std::optional<unsigned short> port_;
	sockaddr sock_addr_;
};

} // namespace uvbind
