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

#include "signal_handler.h"

uvbind::signal::signal(loop_ptr l, const callback_type& cb)
    : callbackable_handle(l,cb)
{
}

void uvbind::signal::start(int signum)
{
	int r = uv_signal_start(holder_.get(),[](uv_signal_t* h,int s){
		auto& dt=uv_callback(h);
		dt.cb(dt.self,s); },signum);
	throw_uv_error(r);
}

void uvbind::signal::stop()
{
	int r=uv_signal_stop(holder_.get());
	throw_uv_error(r);
}
