#pragma once

/*************************************************************************
 * Copyright © 2016 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cpphttpx.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <filesystem>
#include "handle.h"

namespace uvbind {

class fs_event : public callbackable_handle<fs_event,uv_fs_event_t,uv_fs_event_init,std::function<void(fs_event*,const std::string&,uv_fs_event,int)> > {
public:
	fs_event(loop_ptr l, const callback_type& cb);
	~fs_event() noexcept override ;

	void start(const std::filesystem::path& des_dir, uv_fs_event_flags flags);
	void stop();

	std::filesystem::path cur_path() const ;
};

} // namespace uvbind
