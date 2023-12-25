#pragma once

#include "../Inode.hpp"
#include "../DataBlock.hpp"
#include "../Data.hpp"
#include "Iterator_DataBlocks.hpp"


class Iterator_DirItems {
public:
	Iterator_DirItems(Inode &inode, const std::shared_ptr<Data> &data);

	Iterator_DirItems begin() const;
	Iterator_DirItems end() const;

	t_DirItem operator*();
	Iterator_DirItems& operator++();

	bool operator==(const Iterator_DirItems& other) const;
	bool operator!=(const Iterator_DirItems& other) const;

	Iterator_DirItems Append_Dir_Item(uint32_t inode_idx, const std::string &item_name);
	Iterator_DirItems Remove_Dir_Item();

private:
	Inode &_inode;
	const std::shared_ptr<Data> _data;

	Iterator_DataBlocks _it_dblocks;
	uint32_t _it_dir_item_idx = 0;

	bool Is_Depleted = false;
};
