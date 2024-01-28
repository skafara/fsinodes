#pragma once

#include <bitset>
#include <functional>

#include "../../container/MMappedFile.hpp"
#include "../../container/io/A_OffsetReadableWritable.hpp"


/**
 * Bitmap
 */
class Bitmap : A_OffsetReadableWritable {
private:
	using t_Word = std::byte;
	using t_Processor = std::function<void (uint32_t)>;

	static constexpr size_t kWord_Bits_Cnt = 8 * sizeof(t_Word);

	std::bitset<kWord_Bits_Cnt> Get_Bits(size_t word_idx) const;

private:
	const size_t _words_cnt;

public:
	/**
	 * Transparently constructs
	 * @param container Underlying RW data
	 * @param offset Offset
	 * @param words_cnt Words count (length)
	 */
	Bitmap(const std::shared_ptr<I_ReadableWritable> &container, size_t offset, size_t words_cnt);

	/**
	 * Returns whether idx'th bit is set
	 * @param idx Index
	 * @return Bool
	 */
	bool Is_Set(size_t idx) const;
	/**
	 * Sets idx'th bit to val
	 * @param idx Index
	 * @param val Value
	 */
	void Set(size_t idx, bool val);

	/**
	 * Returns whether there are cnt free bits
	 * @param cnt Count
	 * @return Bool
	 */
	bool Has_Free(size_t cnt) const;
	/**
	 * Calls the processor function cnt times for each found free bit
	 * @param cnt Count (must exist)
	 * @param processor Processor
	 */
	void Process_Free(size_t cnt, const t_Processor &processor);
};
