/*************************************************************************
 * Copyright © 2016-2022 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cpphttpx.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "../errors.h"
#include "uv_cvt.hpp"

void uvbind::details::throw_error(int r)
{
	if(r!=0) throw error(r);
}
