#include "Iterator_DirItems.hpp"


Iterator_DirItems::Iterator_DirItems(Inode &inode, const std::shared_ptr<Data> &data) : _it_dblocks(inode, data) {
	if (DataBlock::Is_Empty_Dir_Item(*(*this))) {
		Is_Depleted = true;
	}
}

DataBlock::t_DirItem Iterator_DirItems::operator*() {
	return (*_it_dblocks).Read_Dir_Item(_it_dir_item_no);
}

Iterator_DirItems &Iterator_DirItems::operator++() {
	if (_it_dir_item_no >= kBlock_Dir_Items_Cnt) {
		++_it_dblocks;
		_it_dir_item_no = 0;

		if (_it_dblocks == Iterator_DataBlocks::kDepleted) {
			Is_Depleted = true;
			return *this;
		}
	}

	_it_dir_item_no++;
	DataBlock::t_DirItem dir_item = *(*this);
	if (DataBlock::Is_Empty_Dir_Item(dir_item)) {
		Is_Depleted = true;
		return *this;
	}

	return *this;
}

bool Iterator_DirItems::operator==(const Iterator_DirItems &other) const {
	return (this->Is_Depleted && other.Is_Depleted) || (this->_it_dir_item_no == other._it_dir_item_no);
}

bool Iterator_DirItems::operator==(bool other) const {
	return Is_Depleted != other;
}

bool Iterator_DirItems::operator!=(const Iterator_DirItems &other) const {
	return !(*this == other);
}

bool Iterator_DirItems::operator!=(bool other) const {
	return !(*this == other);
}

Iterator_DirItems &Iterator_DirItems::Append(uint32_t inode_idx, const std::string &item_name) {
	for (; *this != kDepleted; ++(*this));
	Is_Depleted = false;

	(*_it_dblocks).Write_Dir_Item(_it_dir_item_no, inode_idx, item_name);

	return *this;
}

Iterator_DirItems &Iterator_DirItems::Remove() {
	Iterator_DirItems other{*this};
	for (Iterator_DirItems tmp{other}; (++tmp) != kDepleted; ++other);

	DataBlock::t_DirItem last = *other;

	(*_it_dblocks).Write_Dir_Item(_it_dir_item_no, last.Inode_Idx, last.Item_Name);
	(*(other._it_dblocks)).Write_Dir_Item(other._it_dir_item_no, DataBlock::kEmpty_Dir_Item.Inode_Idx, DataBlock::kEmpty_Dir_Item.Item_Name);

	if ((*_it_dblocks).Read_Dir_Item(_it_dir_item_no).Inode_Idx == 0) {
		Is_Depleted = true;
	}

	return *this;
}
