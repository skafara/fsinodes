#pragma once

#include "I_ReadableWritable.hpp"


/**
 * Offset Readable Writable
 */
class A_OffsetReadableWritable : public I_ReadableWritable {
public:
	/**
	 * Transparently constructs
	 * @param rw_data Readable/Writable data
	 * @param offset Offset
	 */
	A_OffsetReadableWritable(const std::shared_ptr<I_ReadableWritable> &rw_data, size_t offset) :
			_rw_data(rw_data), _offset(offset) {
		//
	}

	using I_ReadableWritable::Read;
	using I_ReadableWritable::Write;

	void Read(t_Byte_Buf &buf, size_t idx, size_t len) const override {
		_rw_data->Read(buf, _offset + idx, len);
	};
	void Write(const t_Byte_Buf &buf, size_t idx, size_t len) override {
		_rw_data->Write(buf, _offset + idx, len);
	};

protected:
	const std::shared_ptr<I_ReadableWritable> _rw_data;
	const size_t _offset;
};
