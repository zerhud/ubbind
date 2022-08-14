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

#include <thread>
#include <cassert>
#include "loop.h"
#include "errors.h"

uvbind::loop::loop()
{
	int r=uv_loop_init(&loop_);
	if(r!=0) throw error(r);
}

uvbind::loop::~loop() noexcept
{
	uv_loop_close(&loop_);
}

void uvbind::loop::run_once()
{
	int r=uv_run(&loop_,UV_RUN_ONCE);
	if(r<0) throw error(r);
}

void uvbind::loop::run()
{
	int r=uv_run(&loop_,UV_RUN_DEFAULT);
	if(r<0) throw error(r);
}

void uvbind::loop::run_and_detach()
{
	std::thread([this]{run();}).detach();
}

void uvbind::loop::stop()
{
	uv_stop(&loop_);
}

bool uvbind::loop::alive() const
{
	return uv_loop_alive(&loop_)!=0;
}

void uvbind::loop::add_to_queue(std::function<void ()> work, std::function<void (std::exception_ptr)> finish)
{
	struct task_info {
		std::function<void()> work;
		std::function<void(std::exception_ptr)> finish;
		std::exception_ptr exception;
		uv_work_t request; };

	std::unique_ptr<task_info> task (new task_info());
	task->request.data = task.get();
	task->work=work;
	task->finish=finish;

	int r=uv_queue_work(&loop_,&task->request,[](uv_work_t* req){
		task_info* task=(task_info*)req->data;
		assert( task );
		try{ task->work(); }
		catch(...){task->exception=std::current_exception();}
	},[](uv_work_t* req, int status){
		task_info* task=(task_info*)req->data;
		assert( task );

		if(status) {
			if(task->exception) {
				try {
					try {std::rethrow_exception(task->exception);}
					catch(...){ std::throw_with_nested( error(status) ); }
				} catch (...){ task->exception=std::current_exception(); }
			}
		} else { task->exception=std::make_exception_ptr<error>(status); }

		task->finish(task->exception);
	});

	if(r) throw error(r);
	task.release();
}
