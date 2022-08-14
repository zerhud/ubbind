#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE udp

#include <thread>
#include <chrono>
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>

#include <uvbind/udp.h>
#include <uvbind/ip.hpp>
#include <uvbind/allocator.hpp>

using namespace std::literals;

void run(auto& loop){
	std::thread([&loop]{ loop.run(); }).detach();
}

void wait_for(auto& var, auto&& val)
{
	for(std::size_t i=0;i<100'000'000;++i) {
		if( var == val ) return;
		else std::this_thread::yield();
	}
}

BOOST_AUTO_TEST_SUITE(utils)
BOOST_AUTO_TEST_CASE(ip_addr)
{
	uvbind::ip ip("127.0.0.1"sv, 8090);
	const sockaddr* native = ip;

	BOOST_TEST(native != nullptr);

	std::string cvt;
	cvt.resize(128);
	uv_ip_name(native, cvt.data(), cvt.size());

	BOOST_TEST(ip.port() == 8090);
	BOOST_TEST(cvt.c_str() == "127.0.0.1"sv);
	BOOST_TEST(ip.to_str<std::string>() == "127.0.0.1"sv);
	BOOST_TEST(ip.to_str<std::pmr::string>(std::pmr::get_default_resource()) == "127.0.0.1"sv);

	uvbind::ip ip_6(uvbind::ip::v6, "2001:0db8:11a3:09d7:1f34:8a2e:07a0:765d"sv, 80);
	BOOST_TEST(ip_6.to_str<std::string>() == "2001:db8:11a3:9d7:1f34:8a2e:7a0:765d"sv);
	BOOST_TEST(ip_6.port() == 80);

	BOOST_CHECK_THROW((uvbind::ip{uvbind::ip::v4, "bad ip"sv, 80}), std::exception);

	uvbind::ip ip_cvt(ip.addr());
	BOOST_TEST(ip.to_str<std::string>() == ip_cvt.to_str<std::string>());
	BOOST_TEST(ip.port() == ip_cvt.port());
}
BOOST_AUTO_TEST_SUITE_END() // utils


BOOST_AUTO_TEST_SUITE(binds)
BOOST_AUTO_TEST_CASE(udp_send_recieve)
{
	uvbind::loop _loop;
	uvbind::loop_ptr loop(&_loop, [](auto*){});

	std::size_t queue_step=0, read1=0, read2=0, sent=0;
	run(*loop);

	uvbind::allocator_static alloc;
	uvbind::ip ip1("127.0.0.1"sv, 7776), ip2("127.0.0.1"sv, 7777);
	uvbind::udp u1(alloc, loop, ip1, [&read1](uvbind::udp_message m){++read1;});
	uvbind::udp u2(alloc, loop, ip2, [&read2](uvbind::udp_message m){++read2;});

	BOOST_TEST(read1 == 0);
	BOOST_TEST(read2 == 0);

	auto data = "test_data"sv;
	auto sender = u1.send(ip2, [&sent]{++sent;}, data);
	wait_for(read2, 1);
	wait_for(sent, 1);

	BOOST_TEST(read1 == 0);
	BOOST_TEST(read2 == 1);
	BOOST_TEST(sent == 1);

	loop->add_to_queue([&queue_step]{ ++queue_step; }, [](auto){});
	wait_for(queue_step, 1);
	BOOST_TEST(queue_step == 1);

	loop->stop();
}
BOOST_AUTO_TEST_SUITE_END() // binds

