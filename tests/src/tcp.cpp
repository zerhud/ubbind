#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE tcp

#include <thread>
#include <chrono>
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>

#include <uvbind/tcp.h>
#include <uvbind/allocator.hpp>

using namespace std::literals;

void run(auto& loop){
	std::thread([&loop]{ loop.run(); }).detach();
}

void wait_for(auto& var, auto&& val)
{
	for(std::size_t i=0;i<1'000'000;++i) {
		if( var == val ) return;
		else std::this_thread::yield();
	}
}

BOOST_AUTO_TEST_SUITE(binds)
BOOST_AUTO_TEST_CASE(loop_works)
{
	uvbind::loop loop;
	std::size_t count=0;
	loop.add_to_queue([&count]{ ++count; }, [](auto){});
	run(loop);
	wait_for( count, 1 );
	loop.stop();
	BOOST_TEST(count == 1);
}
BOOST_AUTO_TEST_CASE(tcp_send_recive)
{
	uvbind::loop _loop;
	uvbind::loop_ptr loop(&_loop, [](auto*){});

	std::size_t cons = 0, con_ok=0, read = 0, writes=0, queue_step = 0;

	loop->add_to_queue([&queue_step]{ ++queue_step; }, [](auto){});
	wait_for(queue_step, 1);
	BOOST_TEST(queue_step == 1);

	// create server
	std::shared_ptr<uvbind::stream> con;
	uvbind::tcp_recive srv(loop, uvbind::ip(uvbind::ip::v4, "0.0.0.0"sv,  7171), 128,
	                       [&cons,&con,&read](uvbind::tcp_recive* srv,int){
		// on new connection
		++cons;
		auto on_read = [&read](std::span<std::byte> buf){
			++read;
			std::string_view data{(const char*)buf.data(), buf.size()};
			BOOST_TEST(data == "from_client"sv);
		};
		con.reset( new uvbind::tcp_acceptor(srv, uvbind::allocator_static<4096>{}, on_read) );
	});


	run(*loop);

	// create client:
	BOOST_TEST(con_ok == 0);
	auto client_reader = [&read](std::span<std::byte> buf){
		++read;
		BOOST_TEST(std::string_view((const char*)buf.data(),buf.size()) == "read_client"sv);
	};
	auto client_on_connect = [&con_ok](int s){ ++con_ok; };
	uvbind::tcp_connect
	        client(loop, uvbind::ip(uvbind::ip::v4, "0.0.0.0"sv, 7171),
	               std::move(client_on_connect), std::move(client_reader),
	               uvbind::allocator_static<4096>{});
	wait_for( cons, 1 );
	wait_for( con_ok, 1 );
	BOOST_TEST(cons == 1);
	BOOST_TEST_REQUIRE(con_ok == 1);
	BOOST_TEST_REQUIRE( con != nullptr );


	// send message to server from client
	auto client_writer = client.start_write([&writes]{++writes;}, "from"sv, "_client"sv);
	wait_for( writes, 1 );
	wait_for( read, 1 );
	BOOST_TEST( writes == 1 );
	BOOST_TEST( read == 1 );

	// send message to client from server
	BOOST_TEST(con->is_writable() == true);
	BOOST_TEST(con->is_readable() == true);
	auto server_writer = con->start_write([&writes]{ ++writes; }, "read"sv,"_client"sv);
	wait_for( writes, 2 );
	BOOST_TEST( writes == 2 );
	wait_for( read, 2 );
	BOOST_TEST( read == 2 );

	loop->add_to_queue([&queue_step]{ ++queue_step; }, [](auto){});
	wait_for(queue_step, 2);
	BOOST_TEST(queue_step == 2);
	loop->stop();

	wait_for( read, 3 ); // after all read with empty input may be called
}
BOOST_AUTO_TEST_SUITE_END()
