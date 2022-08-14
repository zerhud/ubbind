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

#include "handle.h"
#include "errors.h"

uvbind::handle::handle(uvbind::loop_ptr l)
    : loop_(l)
{
	if(!l) throw binder_error("cannor initialize libuv handler without loop");
}

uvbind::handle::~handle() noexcept
{
}

bool uvbind::handle::is_active() const
{
	return uv_is_active(get_handle())!=0;
}

uvbind::loop_ptr uvbind::handle::owned_loop() const
{
	return loop_;
}

uv_loop_t* uvbind::handle::bound_loop() const
{
	return &loop_->loop_;
}

void uvbind::handle::throw_uv_error(int rcode)
{
	if(rcode != 0)
		throw error(rcode);
}
