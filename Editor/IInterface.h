#pragma once
#ifndef IINTERFACE_H_INCLUDED
#define IINTERFACE_H_INCLUDED


class IInterface {
public:

	virtual ~IInterface();

protected:

	IInterface();
	IInterface(const IInterface&);
};

#endif // IINTERFACE_H_INCLUDED
