#pragma once

#include <string>

#include "../../container/io/A_OffsetReadableWritable.hpp"
#include "DataBlock.hpp"


/**
 * Data
 */
class Data : public A_OffsetReadableWritable {
public:
	/**
	 * Transparently constructs
	 * @param container Underlying RW data
	 * @param offset Offset
	 */
	Data(const std::shared_ptr<I_ReadableWritable> &container, size_t offset);

	/**
	 * Returns idx'th data block
	 * @param idx Index
	 * @return Data Block
	 */
	DataBlock Get(size_t idx) const;
};
