#pragma once

/*************************************************************************
 * Copyright © 2016 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cpphttpx.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <functional>
#include "handle.h"

namespace uvbind {

class signal;
class signal : public callbackable_handle
      <
         signal
        ,uv_signal_t
        ,uv_signal_init
        ,std::function<void (signal*,int)>
      >
{
public:
	signal(loop_ptr l, const callback_type& cb);
	void start(int signum);
	void stop();
};

} // namespace uvbind
