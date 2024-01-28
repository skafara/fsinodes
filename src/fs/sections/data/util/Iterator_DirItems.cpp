#include "Iterator_DirItems.hpp"


Iterator_DirItems::Iterator_DirItems(Inode &inode, const std::shared_ptr<Data> &data) :
	_inode(inode), _data(data), _it_dblocks(_inode, _data) {
	if (_it_dblocks == _it_dblocks.end()) {
		Is_Depleted = true;
	} else {
		DataBlock::t_DirItem dir_item = *(*this);
		if (dir_item.Inode_Idx == 0 && std::string{dir_item.Item_Name}.empty()) {
			Is_Depleted = true;
		}
	}
}

Iterator_DirItems Iterator_DirItems::begin() const {
	return {_inode, _data};
}

Iterator_DirItems Iterator_DirItems::end() const {
	Iterator_DirItems it{_inode, _data};
	it.Is_Depleted = true;
	it._it_dir_item_idx = -1;
	return it;
}

DataBlock::t_DirItem Iterator_DirItems::operator*() {
	return (*_it_dblocks).Get_Dir_Item(_it_dir_item_idx);
}

Iterator_DirItems &Iterator_DirItems::operator++() {
	if (_it_dir_item_idx >= 1024 / sizeof(DataBlock::t_DirItem)) {
		++_it_dblocks;
		_it_dir_item_idx = 0;

		if (_it_dblocks == _it_dblocks.end()) {
			Is_Depleted = true;
			return *this;
		}
	}

	_it_dir_item_idx++;
	DataBlock::t_DirItem dir_item = *(*(this));
	if (dir_item.Inode_Idx == 0 && std::string{dir_item.Item_Name}.empty()) {
		Is_Depleted = true;
		return *this;
	}

	return *this;
}

bool Iterator_DirItems::operator==(const Iterator_DirItems &other) const {
	return (this->Is_Depleted && other.Is_Depleted) || (this->_it_dir_item_idx == other._it_dir_item_idx);
}

bool Iterator_DirItems::operator!=(const Iterator_DirItems &other) const {
	return !(*this == other);
}

Iterator_DirItems Iterator_DirItems::Append_Dir_Item(uint32_t inode_idx, const std::string &item_name) {
	for (; *this != end(); ++(*this));

	(*_it_dblocks).Set_Dir_Item(_it_dir_item_idx, inode_idx, item_name);
	Is_Depleted = false;

	return *this;
}

Iterator_DirItems Iterator_DirItems::Remove_Dir_Item() {
	// TODO swap s poslednim
	Iterator_DirItems it_old{*this};
	for (;; ++it_old) {
		Iterator_DirItems it_new{it_old};
		++it_new;
		if (it_new == it_new.end()) {
			break;
		}
	}
	DataBlock::t_DirItem last = *it_old;

	(*_it_dblocks).Set_Dir_Item(_it_dir_item_idx, last.Inode_Idx, last.Item_Name);
	(*_it_dblocks).Set_Dir_Item(it_old._it_dir_item_idx, 0, ""); // TODO empty

	if ((*_it_dblocks).Get_Dir_Item(_it_dir_item_idx).Inode_Idx == 0) {
		Is_Depleted = true;
		_it_dir_item_idx = -1;
	}

	return *this;
}
