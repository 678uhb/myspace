#include "this_space.hpp"

int test_listen_connect() {
	
	WSAData wd;
	WSAStartup(MAKEWORD(1, 1), &wd);
	scope(WSACleanup());

	list<thread> threads;

	string textstring(100 << 20, '0');

	threads.emplace_back([&textstring]() {
		auto l = new_shared<listener_t>(8080);
		auto tpool = new_shared<thread_pool_t>();
		for (size_t countdown = 3; countdown; ) {
			if (false) {
				sleep_for(seconds(1));
				continue;
			}
			else {
				cout << "-------------before accept " << endl;
				auto conn = l->accept(seconds(10));
				if (conn && *conn) {
					countdown--;
					cout << "------------after accept " << endl;
					tpool->push_back([conn, &textstring]() {
						size_t count = 0;
						auto last_time = high_resolution_clock::now();
						while (*conn) {
							auto x = conn->recv(textstring.size(), seconds(9999));
							if (x.size() != textstring.size()) {
								cerr << " accept thread error ! " << endl;
								break;
							}
							else {
								count += textstring.size();
								
								auto this_time = high_resolution_clock::now();
								if (this_time - last_time >= seconds(3)) {
									count /= (1<<20);
									cout << " accept thread speed = " << count / duration_cast<seconds>(this_time - last_time).count() << " mb" << endl;
									last_time = this_time;
									count = 0;
								}
							}
							conn->send(textstring, seconds(3));
						}
						cout << "accept thread quit" << endl;
					});
				}
				else {
					cout << "no conn accepted " << endl;
				}
			}
		}
		
	});

	threads.emplace_back([&textstring]() {
		size_t count = 0;
		auto last_time = high_resolution_clock::now();
		for (;;) {
			cout << "before connect" << endl;
			auto conn = new_shared<socket_t>("127.0.0.1",8080, seconds(10));
			if (!conn || !*conn)
				break;
			cout << "after connect" << endl;
			for  ( size_t countdown = 10 ; conn && *conn; countdown--) {

				if (countdown == 0) {
					cout << "connect thread quit" << endl;
					break;
				}
					

				conn->send(textstring, seconds(9999999));
				
				auto x = move(conn->recv(textstring.size(), seconds(9999999)));
				if (x.size() != textstring.size()) {
					cerr << " connect thread error !" << endl;
					break;
				}
				else {
					count += textstring.size();

					auto this_time = high_resolution_clock::now();
					if (this_time - last_time >= seconds(3)) {
						count /= (1 << 20);
						cout << " connect thread speed = " << count / duration_cast<seconds>(this_time - last_time).count() << " mb" << endl;
						last_time = this_time;
						count = 0;
					}
				}
			}
		}
	});
	
	for (auto& t : threads) {
		if (t.joinable())
			t.join();
	}
	
	return 0;
}

int fiber(int x) {
	if (x <= 2)
		return 1;
	auto subfilber = [x](coroutine_t<int>& co) {
		co.yield(fiber(x - 1) + fiber(x - 2));
	};
	coroutine_t<int> co(subfilber);
	return co.get();
};
void test_coroutine() {	
	cout << fiber(10) << endl;
}

void test_thread_pool() {
	auto tpool = new_shared<thread_pool_t>();
	list<future<size_t>> results;
	for (size_t i = 0; i < 10; ++i) {
		results.emplace_back(tpool->push_back([i]() { sleep_for(seconds(1)); return i; }));
	}
	for (auto& f : results) {
		cout << f.get() << endl;
	}
}

void test_log() {
	log_t l;
	auto pool = new_shared<thread_pool_t>();
	for (size_t i = 0; i < 10; ++i) {
		pool->push_back([&]() {
			for (;;) {
				sleep_for(milliseconds(100));
				log_debug(l, 123);
				log_debug(l, 456, 789);
				log_debug(l, "hello word");
			}
		});
	}
	
}

int main() {
	//test_listen_connect();
	//test_coroutine();
	//test_asm();
	//test_thread_pool();
	test_log();
	system("pause");
	return 0;
}