
#pragma once

#include "myspace/config.h"

myspace_begin

template<class t, class... a>
typename enable_if<!is_array<t>::value, unique_ptr<t>>::type
new_unique(a&&... args)
{
	return unique_ptr<t>(new t(forward<a>(args)...));
}

template<class t>
typename enable_if<is_array<t>::value && extent<t>::value == 0, unique_ptr<t>>::type
new_unique(size_t count)
{
	typedef typename remove_extent<t>::type T;
	return unique_ptr<t>(new T[count]());
}


template<class t, class... a>
shared_ptr<t> new_shared(a&&... args)
{
	return move(make_shared<t>(forward<a>(args)...));
}

myspace_end

