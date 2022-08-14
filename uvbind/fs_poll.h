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

class fs_poll : public callbackable_handle<fs_poll,uv_fs_poll_t,uv_fs_poll_init,std::function<void(fs_poll*,int,const uv_stat_t*,const uv_stat_t*)> > {
public:
	fs_poll(loop_ptr l, const callback_type& cb);
	~fs_poll() noexcept override ;

	void start(const std::string& path);
	void stop();

	std::string cur_path() const ;
};

} // namespace uvbind
