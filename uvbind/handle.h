#pragma once

/*************************************************************************
 * Copyright © 2016 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cpphttpx.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <functional>
#include <cassert>
#include <uv.h>
#include "loop.h"

namespace uvbind {

class handle {
public:
	handle(const handle&)=delete;
	handle(handle&&)=default;
	handle& operator = (const handle&)=delete;
	handle& operator = (handle&&)=default;

	handle(loop_ptr l);
	virtual ~handle() noexcept;
	bool is_active() const ;

	loop_ptr owned_loop() const ;
protected:
	uv_loop_t* bound_loop() const ;
	virtual uv_handle_t* get_handle() const =0;
	static void throw_uv_error(int rcode) ;
private:
	loop_ptr loop_;
};

template< typename H, typename UV, int (*init) (uv_loop_t*,UV*), typename callback_type_=std::function<void(H*)> >
class callbackable_handle : public handle {
public:
	typedef callback_type_ callback_type;
protected:
	typedef UV uv_type;
public:
	struct inner_UV : UV {
		callback_type cb;
		H* self;
	};
protected:
	struct deleter {
		void operator() (UV* h) noexcept {
			uv_close((uv_handle_t*)h,[](uv_handle_t* h_){
				inner_UV* h=(inner_UV*)h_;
				delete h;
			});
		}
	};
public:
	callbackable_handle(callbackable_handle&& other) : handle(std::move(other)), holder_(std::move(other.holder_)) {((inner_UV*)holder_)->self=this;}
	callbackable_handle& operator = (callbackable_handle&& other) {
		handle::operator =(std::move(other));
		holder_.swap(other.holder_);
		((inner_UV*)holder_)->self=this;
		return *this; }
	callbackable_handle(loop_ptr l) : handle(l), holder_(new inner_UV(),deleter()) {holder_->data=nullptr;}
	callbackable_handle(loop_ptr l, const callback_type& cb)
	    : callbackable_handle(l)
	{
		assert( init );
		int r=init(bound_loop(),(UV*)holder_.get());
		prepare_holder(r,cb);
	}

	void release_holder() noexcept {holder_.release();}
protected:
	uv_handle_t* get_handle() const override {return (uv_handle_t*)holder_.get();}
	void prepare_holder(int r, const callback_type& cb) {
		if(r) {
			delete holder_.get();
			holder_.release();
			throw_uv_error(r);
		}

		holder_->cb=cb;
		holder_->self=static_cast<H*>(this);
	}

	static inner_UV& uv_callback(UV* h) {
		assert( h );
		return *((inner_UV*)h);
	}

	std::unique_ptr<inner_UV,deleter> holder_;
};

} // namespace uvbind
