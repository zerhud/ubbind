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

#include "timer.h"


uvbind::timer::timer(loop_ptr l, const callback_type& cb)
    : callbackable_handle(l,cb)
{
}

uvbind::timer::~timer() noexcept
{
}

void uvbind::timer::start()
{
	assert( holder_ );

	uint64_t repeat = uv_timer_get_repeat(holder_.get());

	int r = uv_timer_start(holder_.get(),[](uv_timer_t* h){
		auto& dt=uv_callback(h);
		dt.cb(dt.self);
	},repeat,repeat);
	if(r) throw_uv_error(r);
}

void uvbind::timer::stop()
{
	assert( holder_ );
	int r=uv_timer_stop(holder_.get());
	if(r) throw_uv_error(r);
}

std::chrono::milliseconds uvbind::timer::repeat() const
{
	assert( holder_ );
	return std::chrono::milliseconds( uv_timer_get_repeat(holder_.get()) );
}

void uvbind::timer::repeat(std::chrono::milliseconds interval)
{
	assert( holder_ );
	uv_timer_set_repeat(holder_.get(),interval.count());
}
