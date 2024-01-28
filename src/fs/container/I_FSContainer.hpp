#pragma once

#include "io/I_ReadableWritable.hpp"
#include "io/I_Resizable.hpp"


/**
 * FS Container
 */
class I_FSContainer : public I_ReadableWritable, public I_Resizable {
public:
	/**
	 * Transparently destructs
	 */
	~I_FSContainer() override = default;

	/**
	 * Returns size of the FS container
	 * @return Size
	 */
	virtual size_t Get_Size() const = 0;
	/**
	 * Clears the contents of the FS container
	 */
	virtual void Clear() = 0;
};