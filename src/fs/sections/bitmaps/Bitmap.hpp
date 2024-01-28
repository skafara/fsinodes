#pragma once

#include <bitset>
#include <functional>

#include "../../container/MMappedFile.hpp"
#include "../../container/io/A_OffsetReadableWritable.hpp"



class Bitmap : A_OffsetReadableWritable {
private:
	using t_Word = std::byte;
	using t_Processor = std::function<void (uint32_t)>;

	static constexpr size_t kWord_Bits_Cnt = 8 * sizeof(t_Word);

	const size_t _words_cnt;

	std::bitset<kWord_Bits_Cnt> Get_Bits(size_t word_idx) const;

public:
	Bitmap(const std::shared_ptr<I_ReadableWritable>& bm_data, size_t offset, size_t words_cnt);

	bool Is_Set(size_t idx) const;
	void Set(size_t idx, bool val);

	bool Has_Free(size_t cnt) const;
	void Process_Free(size_t cnt, const t_Processor &processor);
};
