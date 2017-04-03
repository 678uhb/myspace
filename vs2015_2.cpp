#include "this_space.hpp"
void foo() {
	auto t = new_shared<transaction_t>([]() {debug("first step"); });
	t->depends(new_shared<transaction_t>([]() {debug("but before first step , ..."); }));
}
