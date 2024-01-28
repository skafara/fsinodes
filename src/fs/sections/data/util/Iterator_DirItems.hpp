#pragma once

#include "Iterator_DataBlocks.hpp"


class Iterator_DirItems {
public:
	static constexpr bool kDepleted = false;

	Iterator_DirItems(Inode &inode, const std::shared_ptr<Data> &data);

	DataBlock::t_DirItem operator*();
	Iterator_DirItems& operator++();

	bool operator==(const Iterator_DirItems& other) const;
	bool operator==(bool other) const;
	bool operator!=(const Iterator_DirItems& other) const;
	bool operator!=(bool other) const;

	Iterator_DirItems Append(uint32_t inode_idx, const std::string &item_name);
	Iterator_DirItems Remove();

private:
	Iterator_DataBlocks _it_dblocks;

	uint32_t _it_dir_item_no = 0;
	bool Is_Depleted = false;

};
