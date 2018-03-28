#pragma once

#include "myspace/myspace_include.h"

myspace_begin

class Any
{
	template<class X>
	using StorageType = typename decay<X>::type;

public:
	
	constexpr Any() noexcept
		:_ptr(nullptr)
	{ 
	}

	Any(const Any& a)
		:_ptr(a.clone())
	{
	}

	Any(Any&& a) noexcept
		: _ptr(a._ptr)
	{
		a._ptr = nullptr;
	}

	template<class X,
		typename enable_if<!is_same<typename decay<X>::type, Any>::value, int>::type = 0>
	Any(X&& x) 
	{
		auto p = new Derived<StorageType<X>>(forward<X>(x));

		_ptr = p;
	}

	Any& operator = (const Any& a)
	{
		if (_ptr == a._ptr)
			return *this;

		auto old = _ptr;

		_ptr = a._ptr->clone();

		if (old)
			delete old;

		return *this;
	}

	Any& operator = (Any&& a)
	{
		if (_ptr == a._ptr)
			return *this;

		_ptr = a._ptr;

		a._ptr = nullptr;

		return *this;
	}

	template<class X,
		typename enable_if<!is_same<typename decay<X>::type, Any>::value, int>::type = 0>
	Any& operator = (const X& x)
	{
		this->operator=(move(Any(x)));
	}

	~Any()
	{	
		delete _ptr;
	}

	operator bool() const
	{
		return hasValue();
	}

	bool hasValue() const
	{
		return !!_ptr;
	}

	template<class X>
	bool is() const
	{
		typedef StorageType<X> T;

		return !!dynamic_cast<Derived<T>*>(_ptr);
	}

	template<class X>
	StorageType<X>& as()
	{
		typedef StorageType<X> T;

		auto derived = dynamic_cast<Derived<T>*>(_ptr);

		if (!derived)
			throw bad_cast();

		return derived->_value;
	}

	template<class X>
	operator X ()
	{
		return as<StorageType<X>>();
	}

private:
	
	struct Base
	{
		virtual ~Base() {}

		virtual Base* clone() const = 0;
	};

	template<typename X>
	struct Derived : Base
	{
		template<typename T>
		Derived(T&& x)
			:_value(forward<T>(x))
		{ }

		Base* clone() const
		{
			return new Derived<X>(_value);
		}

		X	_value;
	};

	Base* clone() const
	{
		if (_ptr)
			return _ptr->clone();

		return nullptr;
	}

	Base* _ptr = nullptr;
};

myspace_end