#pragma once

/*************************************************************************
 * Copyright © 2016 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cpphttpx.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <chrono>
#include "handle.h"

namespace uvbind {

class timer : public callbackable_handle<timer,uv_timer_t,uv_timer_init> {
public:
	timer(loop_ptr l, const callback_type& cb);
	~timer() noexcept override ;

	void start();
	void stop();

	std::chrono::milliseconds repeat() const;
	void repeat(std::chrono::milliseconds interval);
};

} // namespace uvbind
