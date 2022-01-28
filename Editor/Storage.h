#pragma once

/* Container class meant to store arbitrary data, usually structures that are not publically accessible. */
template <class C>
class Storage {
private:

	C *mData;
	typedef C tData;

public:

	Storage();
	Storage(const Storage<C>&);
	Storage(Storage<C>&&);
	~Storage();

	Storage<C>& operator=(const Storage<C>&);
	Storage<C>& operator=(Storage<C>&&);
	C* operator->();
};
