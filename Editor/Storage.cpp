#include "stdafx.h"
#include "Storage.h"

#include <algorithm>

template <class C>
Storage<C>::Storage() :
	mData(new C)
{
}

template <class C>
Storage<C>::Storage(const Storage<C> &src) :
	mData(new C)
{
	std::copy(&src.mData[0], &src.mData[0] + sizeof(src.tData), &mData[0]);
}

template <class C>
Storage<C>::Storage(Storage<C>&& src) :
	mData(src.mData)
{
	src.mData = nullptr;
}

template <class C>
Storage<C>::~Storage() {
	delete mData;
}

template <class C>
Storage<C>& Storage<C>::operator=(const Storage<C>& src) {
	mData = new C;
	std::copy(&src.mData[0], &src.mData[0] + sizeof(src.tData), &mData[0]);

	return *this;
}

template <class C>
Storage<C>& Storage<C>::operator=(Storage<C>&& src) {
	mData = src.mData;
	src.mData = nullptr;

	return *this;
}

template <class C>
C* Storage<C>::operator->() {
	return &this->mData;
}
