#include "this_space.hpp"
using namespace std;
using namespace this_space;
int main() {
		cout << sizeof(uintmax_t) << endl;
	this_process_t::listen(7777);
	this_process_t::listen(8888);
	cout << this_thread_t::get_id() << endl;
	this_process_t::pause();
	return 0;
}
