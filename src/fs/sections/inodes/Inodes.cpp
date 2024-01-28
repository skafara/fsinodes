#include "Inodes.hpp"


Inodes::Inodes(const std::shared_ptr<I_ReadableWritable> &container, size_t offset) :
	A_OffsetReadableWritable(container, offset) {
	//
}

Inode Inodes::Get(size_t idx) {
	return {_rw_data, _offset + idx * sizeof(Inode::t_Inode)};
}
