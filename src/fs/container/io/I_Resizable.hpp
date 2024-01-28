#pragma once

#include <cstddef>


class I_Resizable {
public:
	virtual void Resize(size_t size) = 0;
};