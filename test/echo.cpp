#include "this_space.hpp"

int test_listen_connect() {
	

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
#ifdef this_platform_windows
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
#endif
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
				log_debug(l, 123);
				log_debug(l, 456, 789);
				log_debug(l, "test var1 " ,123,  " var2 ", 456," var3 ", 64);
			}
		});
	}
	
}

void test_middlelayer() {


	log_t logger;

	struct conn_thread_t {

		conn_thread_t(shared_ptr<socket_t> conn)
		:_conn(conn){
			
		}

		~conn_thread_t() {
			_conn->close();
			_write_msgs._cond.notify_all();
			if (this_thread::get_id() == _read_thread.get_id()) {
				_read_thread.detach();
			}
			else {
				_read_thread.join();
			}
			if (this_thread::get_id() == _write_thread.get_id()) {
				_write_thread.detach();
			}
			else {
				_write_thread.join();
			}
		}

		thread							_read_thread;
		thread							_write_thread;
		shared_ptr<socket_t>			_conn;
		shared_t<deque<string>>			_write_msgs;
	};


	{
		deque<shared_ptr<listener_t>> listeners;
		listeners.push_back( new_shared<listener_t>(6001));
		listeners.push_back( new_shared<listener_t>(6002));
		listeners.push_back( new_shared<listener_t>(6003));
		listeners.push_back( new_shared<listener_t>(6004));

		for (;;) {
			auto sel = new_shared<select_t>();
			for (auto l : listeners) {
				sel->push(l);
			}
			log_debug(logger, "before accept");
			sel->wait(hours(1));
			for (auto l : listeners) {
				auto conn = l->accept(milliseconds(1));
				if (conn) {
					log_debug(logger, "new conn");
					auto ct = new_shared<conn_thread_t>(conn);

					ct->_write_thread = thread([ct]() {
						for (; *ct->_conn;) {
							string buf;
							if_lock(ct->_write_msgs._mtx) {
								if (ct->_write_msgs._hold.empty()) {
									ct->_write_msgs._cond.wait_for(__ul, hours(1));
									continue;
								}
								buf.swap(ct->_write_msgs._hold.front());
								ct->_write_msgs._hold.pop_front();
							}
							ct->_conn->send(buf, hours(1));
						}
					});

					ct->_read_thread = thread([ct]() {
						string endtoken("abc");
						while (*ct->_conn) {
							auto buf = ct->_conn->recv_until(endtoken, hours(1));
							if (buf.find("quit") != string::npos)
								break;
							buf = "\r\n" + strip(buf) + "\r\n";
							if_lock(ct->_write_msgs._mtx) {
								ct->_write_msgs._hold.emplace_back(move(buf));
							}
							ct->_write_msgs._cond.notify_all();
							
						}
						ct->_conn->close();
						ct->_write_msgs._cond.notify_all();
					});

				}
			}
		}
	}
}


void test()
{
	string s1, s2;
	s2 = s1 = "123";
	s1.append(1, 0);
	s2.append(1, 0);
	s1.append("321");
	s2.append("321");
	cout << s1.size() << endl;
	cout << (s1 == s2) << endl;
}

void test_recv_until()
{

	thread([] {
		auto l = new_shared<listener_t>(8888);
		for (;;)
		{
			auto conn = l->accept(seconds(3));
			if (!conn)
				continue;
			thread([conn]() {
				for (;;){
					auto buf = conn->recv_until("aabbaac", seconds(10));
					cout << buf << endl;
				}
			}).detach();
		}
	}).detach();

	cout << "before connect" << endl;
	auto c = new_shared<socket_t>("127.0.0.1:8888",seconds(10));
	cout << "after connect" << endl;
	for (;;)
	{
		c->send("aabbaaaaafdsaabbaacfdsafsdafasdfsdafsdfaabbaac", seconds(2));
		this_thread::sleep_for(seconds(100));
	}
}

int main() {
#ifdef this_platform_windows
	WSAData wd;
	WSAStartup(MAKEWORD(1, 1), &wd);
	scope(WSACleanup());
#endif
	//test_listen_connect();
	//test_coroutine();
	//test_asm();
	//test_thread_pool();
	//test_log();
	//test_middlelayer();
	//test();
	test_recv_until();
	system("pause");
	return 0;
}