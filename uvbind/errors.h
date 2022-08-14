#pragma once

/*************************************************************************
 * Copyright © 2016 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cpphttpx.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <string>
#include <exception>

namespace uvbind {

std::string explain_error(int ercode);

struct error : std::exception {
	error(int ercode);
	const char* what() const noexcept override;
protected:
	error(const std::string& what);
private:
	std::string what_;
};

struct binder_error : error {
	binder_error(const std::string& ex);
};

} // namespace uvbind
