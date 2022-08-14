#pragma once

/*************************************************************************
 * Copyright © 2016-2022 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cpphttpx.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

namespace uvbind::details {

template<typename self_type, typename base>
struct inner_handler : base {
	self_type* self;

	inner_handler(self_type* self) : self(self)
	{
		this->data = nullptr;
	}

	template<typename hndl_t>
	static self_type* try_extract_self(hndl_t* hndl)
	{
		return ((inner_handler*)hndl)->self;
	}

	static void allocate_uv_buf(uv_handle_t* handle, uv_buf_t& buf)
	{
		auto self = try_extract_self(handle);
		if(self) self->allocator.allocate_uv_buf(buf);
	}

	template<typename hndl_t, typename... args_t>
	static void call_read_cb(hndl_t* handle, args_t... args)
	{
		auto self = try_extract_self(handle);
		if(self) self->read_cb(std::forward<args_t>(args)...);
	}
};

} //namespace uvbind::details
