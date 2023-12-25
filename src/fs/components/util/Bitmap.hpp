#pragma once

#include <bitset>

#include "../../../io/MMappedFile.hpp"
#include "../../../io/A_OffsetReadableWritable.hpp"


using t_Bitmap_Word = std::byte;

class Bitmap : A_OffsetReadableWritable {
public:
	static constexpr size_t kWord_Bits_Cnt = 8 * sizeof(t_Bitmap_Word);

	Bitmap(const std::shared_ptr<I_ReadableWritable>& bm_data, size_t offset);

	bool Is_Set(size_t idx) const;
	void Set(size_t idx, bool val);

private:
	std::bitset<kWord_Bits_Cnt> Get_Bits(size_t idx_word) const;
};
