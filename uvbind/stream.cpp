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

#include <memory>
#include <cassert>

#include "stream.h"
#include "errors.h"


void uvbind::stream::shutdown(const uvbind::stream::shutdown_cb_t& cb)
{
	struct req_details {
		shutdown_cb_t cb;
	};

	std::unique_ptr<req_details> d(new req_details());
	d->cb = cb;

	std::unique_ptr<uv_shutdown_t> req(new uv_shutdown_t);
	req->data = d.get();

	uv_stream_t* s = get_stream(); assert( s );
	int r=uv_shutdown(req.get(),s,[](uv_shutdown_t* req, int status){
		req_details* d=(req_details*)req->data;
		if(status<0) {
			delete d;
			return ;
		}

		d->cb(req,status);
		delete d;
	});

	if(r) throw error(r);
	d.release();
	req.release();
}

bool uvbind::stream::is_readable() const
{
	uv_stream_t* s = get_stream();
	assert( s );
	return uv_is_readable(s)!=0;
}

bool uvbind::stream::is_writable() const
{
	uv_stream_t* s = get_stream();
	assert( s );
	return uv_is_writable(s)!=0;
}
