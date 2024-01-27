#pragma once

#include "../io/I_ReadableWritable.hpp"
#include "../io/I_Resizable.hpp"


class I_FSContainer : public I_ReadableWritable, public I_Resizable {
public:
	~I_FSContainer() override = default;

	virtual size_t Get_Size() const = 0;
	virtual void Clear() = 0;
};