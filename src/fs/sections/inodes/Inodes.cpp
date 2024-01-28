#include "Inodes.hpp"


Inodes::Inodes(const std::shared_ptr<I_ReadableWritable> &inodes_data, size_t offset) :
	A_OffsetReadableWritable(inodes_data, offset) {
	//
}

Inode Inodes::Get(size_t idx) {
	return {_rw_data, _offset + idx * sizeof(Inode::t_Inode)};
}