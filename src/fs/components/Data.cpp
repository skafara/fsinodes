#include "Data.hpp"


Data::Data(const std::shared_ptr<I_ReadableWritable> &data_data, size_t offset) :
	A_OffsetReadableWritable(data_data, offset) {
	//
}

DataBlock Data::Get(size_t idx) const {
	return {_rw_data, _offset + idx * DataBlock::kSize};
}
