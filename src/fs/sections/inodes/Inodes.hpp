#pragma once

#include "../../container/io/A_OffsetReadableWritable.hpp"
#include "Inode.hpp"


/**
 * Inodes
 */
class Inodes : public A_OffsetReadableWritable {
public:
	/**
	 * Transparently constructs
	 * @param container Underlying RW data
	 * @param offset Offset
	 */
	Inodes(const std::shared_ptr<I_ReadableWritable> &container, size_t offset);

	/**
	 * Returns idx'th inode
	 * @param idx Index
	 * @return Inode
	 */
	Inode Get(size_t idx);

};
