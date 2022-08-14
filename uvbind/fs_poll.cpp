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

#include "fs_poll.h"

#include <string>
#include <limits.h>

uvbind::fs_poll::fs_poll(loop_ptr l, const callback_type& cb)
    : callbackable_handle(l,cb)
{
}

uvbind::fs_poll::~fs_poll() noexcept
{
}

void uvbind::fs_poll::start(const std::string& path)
{
	int r=uv_fs_poll_start(holder_.get(),[](uv_fs_poll_t* h, int status, const uv_stat_t* prev, const uv_stat_t* curr){
	      auto& dt=uv_callback(h);
	      dt.cb(dt.self,status,prev,curr);} ,path.c_str(),100);
	if(r) throw_uv_error(r);
}

void uvbind::fs_poll::stop()
{
	int r=uv_fs_poll_stop(holder_.get());
	if(r) throw_uv_error(r);
}

std::string uvbind::fs_poll::cur_path() const
{
	char buff[PATH_MAX];
	std::size_t size=PATH_MAX;
	int r=uv_fs_poll_getpath(holder_.get(),&buff[0],&size);
	if(r) throw_uv_error(r);
	return std::string(&buff[0],size);
}
