#pragma once

#include "../../io/A_OffsetReadableWritable.hpp"
#include "Inode.hpp"


class Inodes : public A_OffsetReadableWritable {
public:
	Inodes(const std::shared_ptr<I_ReadableWritable> &inodes_data, size_t offset);

	Inode Get(size_t idx) const;
};
