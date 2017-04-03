#include "this_space.hpp"

void on_newlink(shared_ptr<socketstream_t> c) {
	thread([c]() {
		try {
			string buf;
			auto p = new_shared<pump_t>();
			p->push(c, [c, &buf]() {
				buf.append(move(c->read()));
				auto cmds = strip(cut(buf, "\n"));
				for (auto& cmd : cmds) {
					debug("recv cmd : %s", cmd.c_str());
					if (cmd == "quit" || cmd == "exit")
						throw pump_t::cmd_stop_t();
					else if (cmd == "help")
						c->writeall("read me:\r\n"
							"\thelp\t:\tshow this tips\r\n"
							"\tquit/exit\t:\tend this session\r\n");
					else
						c->writeall("unknown cmd : " + cmd + ", type \'help\' for avaiable cmds\r\n");
				}
			}).run();
		}
		catch_all();
	}).detach();
}

int main() {
	try {
		net_switch();
		info("test begin");
		auto tpool = new_shared<thread_pool_t>();
		auto listener = new_shared<listenstream_t>(8888);
		auto pump = new_shared<pump_t>();
		pump->push(listener, [listener, pump]() {
			auto c = listener->accept();
			if (!c)
				return;
			on_newlink(c);
		}).run();
		info("test end");
	}
	catch_all();
	system("pause");
	return 0;
}
