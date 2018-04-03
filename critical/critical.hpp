
#pragma once

#include "myspace/myspace_include.h"

MYSPACE_BEGIN

template<class Hold, class Mtx, class Cond, class X>
	class _CriticalImpl : public Hold, public Mtx, public Cond
{
public:
	template<class... Targs>
	_CriticalImpl(Targs&&... args)
		:Hold(forward<Targs>(args)...)
	{
		
	}
};

template<class Hold, class Mtx, class Cond>
	class _CriticalImpl<Hold, Mtx, Cond, typename std::enable_if<is_fundamental<Hold>::value, Hold>::type> 
		: public Mtx, public Cond
{
public:
	template<class... Targs>
	_CriticalImpl(Targs&&... args)
		:_hold(forward<Targs>(args)...)
	{
		
	}

#if defined(MYSPACE_WINDOWS)
	template<class X>
	typename enable_if<!is_same<X, GUID>::value, bool>::type 
		operator == (const Hold& other) const
	{
		return _hold == other;
	}
#else
	template<class X>
	bool operator == (const X& other) const
	{
		return _hold == other;
	}
#endif

	bool operator < (const Hold& other) const
	{
		return _hold < other;
	}

	void operator -- ()
	{
		--_hold;
	}

	void operator ++ ()
	{
		++_hold;
	}

	operator const Hold& () const
	{
		return _hold;
	}

protected:
	Hold _hold;
};

template<class Hold, class Mtx = mutex, class Cond = condition_variable>
	class Critical : public _CriticalImpl<Hold, Mtx, Cond, Hold>
{
public:
	template<class... Targs>
	Critical(Targs&&... args)
		:_CriticalImpl<Hold, Mtx, Cond, Hold>(forward<Targs>(args)...)
	{
		
	}
};


MYSPACE_END
