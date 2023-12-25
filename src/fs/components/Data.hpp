#pragma once

#include <string>

#include "../../io/A_OffsetReadableWritable.hpp"
#include "DataBlock.hpp"


class Data : public A_OffsetReadableWritable {
public:
	Data(const std::shared_ptr<I_ReadableWritable> &data_data, size_t offset);

	DataBlock Get(size_t idx) const;
};
