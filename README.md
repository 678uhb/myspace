# this_space
c++, string utils,  network utils, etc, all in one hpp
# example
```c++
#include "this_space.hpp"
auto tpool = new_shared<thread_pool_t>();
class client : public enable_shared_from_this<client> {
public:
	void do_business(shared_ptr<fdpump_t<socket_t>> pump) {
		try {
			sendbuf_.emplace_back(move(sock_->read()));
			while (!sendbuf_.empty()) {
				sendbuf_.front().erase(0, sock_->write(sendbuf_.front()));
				if (sendbuf_.front().empty())
					sendbuf_.pop_front();
				else
					break;
			}
			auto self(shared_from_this());
			pump->push(sock_, true, !sendbuf_.empty(), true, [self, pump](bool, bool, bool) {
				tpool->push_back([self, pump]() {self->do_business(pump); });
			});
		}
		catch_all();
	}
public:
	string recvbuf_;
	list<string> sendbuf_;
	shared_ptr<socketstream_t> sock_;
};
class main_t {
public:
	int run() {
		try {
			// init
			info("init");
			net_switch();
			auto l_long = new_shared<listenstream_t>(8888);
			auto l_short = new_shared<listenstream_t>(6666);
			auto pump = new_shared<fdpump_t<socket_t>>();
			function<void(bool, bool, bool)> l_accept_callback;
			l_accept_callback  = [&l_accept_callback, l_long, pump](bool, bool, bool) {
				tpool->push_back([&l_accept_callback, l_long, pump]() {
					while (auto sock = l_long->accept()) {
						auto c = new_shared<client>();
						c->sock_ = sock;
						pump->push(c->sock_, true, false, true, [pump, c](bool, bool, bool) {
							tpool->push_back([pump, c]() { c->do_business(pump); });
						});
					}
					pump->push(l_long, true, false, true, l_accept_callback);
				});
			};
			function<void(bool, bool, bool)> s_accept_callback;
			s_accept_callback = [&s_accept_callback, l_short, pump](bool, bool, bool) {
				tpool->push_front([&s_accept_callback, l_short, pump]() {
					while (auto sock = l_short->accept()) {
						auto c = new_shared<client>();
						c->sock_ = sock;
						pump->push(c->sock_, true, false, true, [pump, c](bool, bool, bool) {
							tpool->push_front([pump, c]() { c->do_business(pump); });
						});
					}
					pump->push(l_short, true, false, true, s_accept_callback);
				});
			};
			pump->push(l_long, true, false, true, l_accept_callback);
			pump->push(l_short, true, false, true, s_accept_callback);
			// run
			info("begin");
			pump->run();
			info("end");
			return 0;
		}
		catch_all();
		system("pause");
		return 1;
	}
};
int main() {
	return new_shared<main_t>()->run();
}

```
