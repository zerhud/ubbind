#pragma once

/*************************************************************************
 * Copyright © 2016 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cpphttpx.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "handle.h"

namespace uvbind {

class async : public callbackable_handle<async,uv_async_t,nullptr> {
public:
	async(loop_ptr l, const callback_type& cb);
	~async() noexcept override ;
	void send();
};

} // namespace uvbind
