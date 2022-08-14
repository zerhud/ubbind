#pragma once

/*************************************************************************
 * Copyright © 2016-2022 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cpphttpx.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <uv.h>

namespace uvbind::details {

void throw_error(int r);

template<typename B>
static uv_buf_t& make_buf(uv_buf_t& buf, const B& data)
{
	if constexpr (std::is_const_v<std::decay_t<B>>)
	    buf.base = (char*)const_cast<B*>(data.data());
	else
	    buf.base = (char*)data.data();
	buf.len = data.size();
	return buf;
}

template<typename B>
static uv_buf_t make_buf(const B& data)
{
	uv_buf_t ret;
	make_buf(ret, data);
	return ret;
}

} // namespace uvbind::details
