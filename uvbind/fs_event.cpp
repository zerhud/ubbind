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

#include "fs_event.h"
#include <limits.h>

uvbind::fs_event::fs_event(loop_ptr l, const callback_type& cb)
    : callbackable_handle(l,cb)
{
}

uvbind::fs_event::~fs_event() noexcept
{
}

void uvbind::fs_event::start(const std::filesystem::path& des_dir, uv_fs_event_flags flags)
{
	auto u8path = des_dir.generic_u8string();
	int r=uv_fs_event_start(holder_.get(),[](uv_fs_event_t* h, const char* file_name_, int events, int status){
	      auto& dt=uv_callback(h);
	      std::string file_name(file_name_);
	      dt.cb(dt.self,file_name,static_cast<uv_fs_event>(events), status);}
	  , (const char*)u8path.c_str()
	  , flags
	  );
	if(r) throw_uv_error(r);
}

void uvbind::fs_event::stop()
{
	int r=uv_fs_event_stop(holder_.get());
	if(r) throw_uv_error(r);
}

std::filesystem::path uvbind::fs_event::cur_path() const
{
	char buff[PATH_MAX];
	std::size_t size=PATH_MAX;
	int r=uv_fs_event_getpath(holder_.get(),&buff[0],&size);
	if(r) throw_uv_error(r);
	return std::filesystem::path( std::string_view(&buff[0],size) );
}
