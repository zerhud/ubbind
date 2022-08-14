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

#include "errors.h"
#include <uv.h>
#include <errno.h>
#include <string.h>

uvbind::error::error(int ercode)
	: what_(
	      std::string(uv_strerror(ercode))+
	      (0<ercode?" ("+std::string(strerror(errno))+")":"")
	       )
{
}

const char* uvbind::error::what() const noexcept
{
	return what_.c_str();
}

uvbind::error::error(const std::string& what) : what_(what)
{
}

uvbind::binder_error::binder_error(const std::string& ex) : error(ex)
{
}

std::string uvbind::explain_error(int ercode)
{
	return std::string(uv_strerror(ercode));
}
