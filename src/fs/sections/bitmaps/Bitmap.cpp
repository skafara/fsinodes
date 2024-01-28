#include "Bitmap.hpp"

#include <utility>


Bitmap::Bitmap(const std::shared_ptr<I_ReadableWritable> &container, size_t offset, size_t words_cnt) :
		A_OffsetReadableWritable(container, offset), _words_cnt(words_cnt) {
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

	Write(idx_word, static_cast<t_Word>(bits.to_ullong()));
}

std::bitset<Bitmap::kWord_Bits_Cnt> Bitmap::Get_Bits(size_t word_idx) const {
	const auto word = Read<t_Word>(word_idx);
	return static_cast<size_t>(word);
}

bool Bitmap::Has_Free(size_t cnt) const {
	size_t found = 0;
	for (size_t i = 0; i < _words_cnt; ++i) {
		found += kWord_Bits_Cnt - Get_Bits(i).count();
		if (found >= cnt) {
			return true;
		}
	}

	return false;
}

void Bitmap::Process_Free(size_t cnt, const Bitmap::t_Processor &processor) {
	size_t found = 0;
	for (size_t i = 0, idx = 0; i < _words_cnt; ++i) {
		const auto bits = Get_Bits(i);
		for (size_t j = 0; j < kWord_Bits_Cnt; ++j, ++idx) {
			if (!bits[j]) {
				found++;
				processor(idx);
			}
			if (found == cnt) {
				return;
			}
		}
	}
}
