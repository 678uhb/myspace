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


class coroutine_t {
	friend class coroutine_pool_t;
public:

	coroutine_t() {
		cout << "coroutine_t() {" << endl;
	}

	~coroutine_t() {
		cout << "~coroutine_t()" << endl;
	}

	void yield() {
		
	}
private:
	static void CALLBACK FiberProc(_In_ PVOID lpParameter) {
		cout << "void CALLBACK FiberProc(_In_ PVOID lpParameter) {"	<< endl;
	}
	shared_ptr<coroutine_pool_t>	_cpool;
	LPVOID							_stack;
	function<void(shared_ptr<coroutine_t>)>		_routine;
};

class coroutine_pool_t {
	friend class coroutine_t;
public:
	coroutine_pool_t() {

	}

	template<class functype>
	void push_back(functype f) {

		class test_t {
		public:
			~test_t() {
				cout << "~test_t() {" << endl;
			}
		private:
			int _x = 100;
		} t;

		//auto co = new_shared<coroutine_t>();
		coroutine_t co;
		co._routine = f;
		co._stack = ::CreateFiberEx(0,0, FIBER_FLAG_FLOAT_SWITCH,&coroutine_t::FiberProc, nullptr);
		SwitchToFiber(co._stack);
	}
private:

	inline void yield(shared_ptr<coroutine_t> co) {
		longjmp(_jb, 0);
	}

	list<shared_ptr<coroutine_t>>				_coroutines;
	jmp_buf										_jb = { 0 };
};

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
void foo(shared_ptr<coroutine_t> co) {

	class test_t2 {
	public:
		~test_t2() {
			cout << "~test_t() {" << endl;
		}
	private:
		int _x = 1011;
	} t;


	cout << "this is a coroutine " << endl;
	co->yield();
	cout << "this is a coroutine 2 " << endl;
};
void test_coroutine() {

	auto cpool = new_shared<coroutine_pool_t>();
	cpool->push_back(foo);
}
int bar (int aaaa, int b, int c) {
	__asm {mov eax aaaa;
mov ebx b
add eax ebx
	}
	return aaaa;
};
void test_asm() {

	

	auto foo1 = []() {
		return 1 + bar(1,2,3);
	};

	cout << foo1() << endl;
}

int main() {
	//test_listen_connect();
	//test_coroutine();
	test_asm();
	//test_thread_pool();
	system("pause");
	return 0;
}