#include "Bitmap.hpp"

#include <utility>


Bitmap::Bitmap(const std::shared_ptr<I_ReadableWritable> &bm_data, size_t offset) :
	A_OffsetReadableWritable(bm_data, offset) {
	//
}

bool Bitmap::Is_Set(size_t idx) const {
	const auto bits = Get_Bits(idx / kWord_Bits_Cnt);
	const size_t idx_bit = idx % kWord_Bits_Cnt;

	return bits[idx_bit];
}

void Bitmap::Set(size_t idx, bool val) {
	if (Is_Set(idx) == val) {
		return;
	}

	const size_t idx_word = idx / kWord_Bits_Cnt;
	auto bits = Get_Bits(idx_word);
	bits[idx % kWord_Bits_Cnt] = val;

	Write(idx_word, static_cast<t_Bitmap_Word>(bits.to_ullong()));
}

std::bitset<Bitmap::kWord_Bits_Cnt> Bitmap::Get_Bits(size_t idx_word) const {
	const auto word = Read<t_Bitmap_Word>(idx_word);
	return static_cast<size_t>(word);
}
