#include "Data.hpp"


Data::Data(const std::shared_ptr<I_ReadableWritable> &container, size_t offset) :
	A_OffsetReadableWritable(container, offset) {
	//
}

DataBlock Data::Get(size_t idx) const {
	return {_rw_data, _offset + idx * DataBlock::kSize};
}
