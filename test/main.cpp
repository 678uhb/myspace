#include "this_space.hpp"

class client{
public:
	client(shared_ptr<socketstream_t> c) 
	:c_(c){
		
	}
	void run() {
		do_recv();
		pump_.run();
	}
private:
	void do_recv() {
		buf_.append(move(c_->read()));
		auto cmds = strip(cut(buf_, "\n"));
		for (auto& cmd : cmds) {
			debug("recv cmd : %s", cmd.c_str());
			if (cmd == "quit" || cmd == "exit")
				return ;
			else if (cmd == "help")
				c_->writeall("read me:\r\n"
					"\thelp\t:\tshow this tips\r\n"
					"\tquit/exit\t:\tend this session\r\n");
			else
				c_->writeall("unknown cmd : " + cmd + ", type \'help\' for avaiable cmds\r\n");
		}
		pump_.push(c_, true, false, true, [this](bool, bool, bool e) {
			if (!e) do_recv();
		});
	}
private:
	string buf_;
	shared_ptr<socketstream_t> c_;
	fdpump_t<socket_t> pump_;
};

class listener {
public:
	listener(uint16_t port) 
	:l_(new_shared<listenstream_t>(port)){
		
	}
	void run() {
		accept();
		pump_.run();
	}
private:
	void accept() {
		l_->setblock(false);
		auto c = l_->accept();
		if (c) {
			auto cli = new_shared<client>(c);
			thread([cli]() {cli->run(); }).detach();
		}
		pump_.push(l_, true, false, true, [this](bool, bool, bool e) {
			if(!e) accept();
		});
	}
private:
	shared_ptr<listenstream_t> l_; 
	fdpump_t<socket_t> pump_;
};

int main() {
	try {
		net_switch();
		info("test begin");
		new_shared<listener>(8888)->run();
		info("test end");
	}
	catch_all();
	system("pause");
	return 0;
}
