#pragma once

#include <cstddef>


/**
 * Resizable
 */
class I_Resizable {
public:
	/**
	 * Resizes to size
	 * @param size Size
	 */
	virtual void Resize(size_t size) = 0;
};