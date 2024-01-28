#pragma once

#include "Iterator_DataBlocks.hpp"


/**
 * Directory Items Iterator
 */
class Iterator_DirItems {
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
	Iterator_DirItems(Inode &inode, const std::shared_ptr<Data> &data);

	/**
	 * Returns current data block
	 * @return Directory Item
	 */
	DataBlock::t_DirItem operator*();
	/**
	 * Increments the iterator
	 * @return This
	 */
	Iterator_DirItems& operator++();

	/**
	 * Returns whether iterators are equal
	 * @param other Other
	 * @return Bool
	 */
	bool operator==(const Iterator_DirItems& other) const;
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
	bool operator!=(const Iterator_DirItems& other) const;
	/**
	 * Returns whether iterators are not equal
	 * @param other Other
	 * @return Bool
	 */
	bool operator!=(bool other) const;

	/**
	 * Appends a directory item to the end
	 * @param inode_idx Inode Index
	 * @param item_name Item Name
	 * @return This
	 */
	Iterator_DirItems &Append(uint32_t inode_idx, const std::string &item_name);
	/**
	 * Remove current directory item
	 * @return This
	 */
	Iterator_DirItems &Remove();

private:
	static constexpr uint32_t kBlock_Dir_Items_Cnt = DataBlock::kSize / sizeof(DataBlock::t_DirItem);

	Iterator_DataBlocks _it_dblocks;

	uint32_t _it_dir_item_no = 0;
	bool Is_Depleted = false;

};
