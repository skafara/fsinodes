#pragma once

#include "../../inodes/Inode.hpp"


/**
 * Data Blocks Iterator
 */
class Iterator_DataBlocks {
private:
	using t_DataBlockAcquirer = std::function<uint32_t ()>;
	using t_DataBlockReleaser = std::function<void (uint32_t)>;

	static constexpr uint32_t kBlock_Refs_Cnt = DataBlock::kSize / sizeof(uint32_t);

public:
	/**
	 * Depleted Iterator (END)
	 */
	static constexpr bool kDepleted = false;

	/**
	 * Transparently constructs
	 * @param inode Inode
	 * @param data Data
	 */
	Iterator_DataBlocks(Inode &inode, const std::shared_ptr<Data> &data);

	/**
	 * Returns current data block
	 * @return Data Block
	 */
	DataBlock operator*();
	/**
	 * Increments the iterator
	 * @return This
	 */
	Iterator_DataBlocks &operator++();

	/**
	 * Returns whether iterators are equal
	 * @param other Other
	 * @return Bool
	 */
	bool operator==(const Iterator_DataBlocks &other) const;
	/**
	 * Returns whether iterators are equal
	 * @param other Other
	 * @return Bool
	 */
	bool operator==(bool other) const;
	/**
	 * Returns whether iterators are not equal
	 * @param other Other
	 * @return Bool
	 */
	bool operator!=(const Iterator_DataBlocks &other) const;
	/**
	 * Returns whether iterators are not equal
	 * @param other Other
	 * @return Bool
	 */
	bool operator!=(bool other) const;

	/**
	 * Appends a "data-containing" data block to the end
	 * @param acquirer Function called when required to acquire a new block (1 + possible indirect to store indirect, direct references)
	 * @return This
	 */
	Iterator_DataBlocks &Append(const t_DataBlockAcquirer &acquirer);
	/**
	 * Releases all data blocks
	 * @param inode Inode
	 * @param data Data
	 * @param releaser Function called when releasing a block
	 */
	static void Release_All(Inode &inode, const std::shared_ptr<Data> &data, const t_DataBlockReleaser &releaser);

private:
	Inode &_inode;
	const std::shared_ptr<Data> &_data;

	bool Is_Depleted = false;

	bool it_directs = true;
	bool it_indirect1 = false;
	bool it_indirect2 = false;

	uint32_t _direct_ref;
	uint32_t it_direct_no = 0;
	uint32_t it_indirect1_no = 0;

};