#pragma once

/*************************************************************************
 * Copyright © 2016 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cpphttpx.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <memory>
#include <functional>
#include <uv.h>

namespace uvbind {

class loop;
class handle;
using loop_ptr = std::shared_ptr<loop> ;

class loop {
	friend class handle;
public:
	loop(const loop&)=delete;
	loop& operator = (const loop&) =delete;

	loop(loop&& other)=delete;
	loop& operator = (loop&& other)=delete;

	loop();
	~loop() noexcept ;

	void run_once();

	void run();
	void run_and_detach();
	void stop();
	bool alive() const ;

	void add_to_queue(std::function<void ()> work, std::function<void (std::exception_ptr)> finish);
private:
	uv_loop_t loop_;
};

} // namespace uvbind
