#include "Iterator_DataBlocks.hpp"


Iterator_DataBlocks::Iterator_DataBlocks(Inode &inode, const std::shared_ptr<Data> &data) :
	_inode(inode), _data(data) {
	_dblock_idx = _inode.Get_Direct(0);
	if (_dblock_idx == -1) {
		Is_Depleted = true;
	}
}

Iterator_DataBlocks Iterator_DataBlocks::begin() const {
	return {_inode, _data};
}

Iterator_DataBlocks Iterator_DataBlocks::end() const {
	Iterator_DataBlocks it{_inode, _data};
	it.Is_Depleted = true;
	it._dblock_idx = -1;
	return it;
}

DataBlock Iterator_DataBlocks::operator*() {
	return _data->Get(_dblock_idx);
}

Iterator_DataBlocks &Iterator_DataBlocks::operator++() {
	it_direct_no++;
	if (it_directs) {
		if (it_direct_no >= 5) {
			it_directs = false;
			it_indirect1 = true;
			it_direct_no = 0;
		} else {
			_dblock_idx = _inode.Get_Direct(it_direct_no);
			if (_dblock_idx == -1) {
				Is_Depleted = true;
				return *this;
			}
		}
	}
	if (it_indirect1) {
		_dblock_idx = _inode.Get_Indirect1();
		if (_dblock_idx == -1) {
			Is_Depleted = true;
			return *this;
		}

		if (it_direct_no >= 1024 / sizeof(uint32_t)) {
			it_indirect1 = false;
			it_indirect2 = true;
			it_indirect1_no = 0;
			it_direct_no = 0;
		} else {
			_dblock_idx = _data->Get(_dblock_idx).Get_Data_Block_Idx(it_direct_no);
			if (_dblock_idx == 0) {
				Is_Depleted = true;
				return *this;
			}
		}
	}
	if (it_indirect2) {
		_dblock_idx = _inode.Get_Indirect2();
		if (_dblock_idx == -1) {
			Is_Depleted = true;
			return *this;
		}

		if (it_direct_no >= 1024 / sizeof(uint32_t)) {
			it_indirect1_no++;
			it_direct_no = 0;
		}
		if (it_indirect1_no >= 1024 / sizeof(uint32_t)) {
			Is_Depleted = true;
			return *this;
		}

		const uint32_t dblock_indirect1_idx = _data->Get(_inode.Get_Indirect2()).Get_Data_Block_Idx(it_indirect1_no);
		if (dblock_indirect1_idx == 0) {
			Is_Depleted = true;
			return *this;
		}

		_dblock_idx = _data->Get(dblock_indirect1_idx).Get_Data_Block_Idx(it_direct_no);
		if (_dblock_idx == 0) {
			Is_Depleted = true;
			return *this;
		}
	}

	return *this;
}

bool Iterator_DataBlocks::operator==(const Iterator_DataBlocks &other) const {
	return (this->Is_Depleted && other.Is_Depleted) || (this->_dblock_idx == other._dblock_idx); // ?
}

bool Iterator_DataBlocks::operator!=(const Iterator_DataBlocks &other) const {
	return !(*this == other);
}

/*uint32_t Iterator_DataBlocks::Get_Direct() {
	if (it_directs) {
		return _inode.Get_Direct(it_direct_no);
	}
	if (it_indirect1) {
		return _data->Get(_inode.Get_Indirect1()).Get_Data_Block_Idx(it_direct_no);
	}
	if (it_indirect2) {
		// TODO
	}

	throw -1;
}

Iterator_DataBlocks Iterator_DataBlocks::Reserve_Another_Data_Block() {
	return {_inode, _data}; // delpteed new, acquire data blocks (allocator(cnt) or throw)
}*/

Iterator_DataBlocks Iterator_DataBlocks::Append_Data_Block(const t_DataBlockAcquirer& dblock_acquirer) {
	for (; *this != end(); ++(*this));

	_dblock_idx = dblock_acquirer();
	Is_Depleted = false; // spis nahoru? a nebo zmenit end

	if (it_directs) {
		_inode.Set_Direct(it_direct_no, _dblock_idx);
	}
	if (it_indirect1) {
		if (_inode.Get_Indirect1() == -1) {
			_inode.Set_Indirect1(_dblock_idx);
			_dblock_idx = dblock_acquirer();
		}

		_data->Get(_inode.Get_Indirect1()).Set_Data_Block_Idx(it_direct_no, _dblock_idx);
	}
	if (it_indirect2) {
		if (_inode.Get_Indirect2() == -1) {
			_inode.Set_Indirect2(_dblock_idx);
			_dblock_idx = dblock_acquirer();
		}

		DataBlock dblock_indirect2 = _data->Get(_inode.Get_Indirect2());
		if (dblock_indirect2.Get_Data_Block_Idx(it_indirect1_no) == 0) {
			dblock_indirect2.Set_Data_Block_Idx(it_indirect1_no, _dblock_idx);
			_dblock_idx = dblock_acquirer();
		}

		DataBlock dblock_indirect1 = _data->Get(dblock_indirect2.Get_Data_Block_Idx(it_indirect1_no));
		dblock_indirect1.Set_Data_Block_Idx(it_direct_no, _dblock_idx);
	}

	/*if (it_directs) {
		if (it_direct_no >= 5) {
			it_directs = false;
			it_indirect1 = true;
			it_direct_no = 0;

			uint32_t indirect1_dblock_idx = dblock_acquirer();
			_inode.Set_Indirect1(indirect1_dblock_idx);
		} else {
			_inode.Set_Direct(it_direct_no, _dblock_idx);
		}
	}
	if (it_indirect1) {
		if (it_direct_no >= 1024 / sizeof(uint32_t)) {
			it_indirect1 = false;
			it_indirect2 = true;
			it_indirect1_no = 0;
			it_direct_no = 0;
		} else {
			_data->Get(_inode.Get_Indirect1()).Set_Data_Block_Idx(it_direct_no, _dblock_idx);
		}
	}*/

	// TODO

	//++(*this);
	return *this;
}
