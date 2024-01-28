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

	return *this;
}

Iterator_DataBlocks Iterator_DataBlocks::Release_Data_Blocks(const t_DataBlockReleaser &dblock_releaser) {
	if (_inode.Get_Indirect2() != -1) {
		DataBlock dblock_indirect2 = _data->Get(_inode.Get_Indirect2());
		for (uint32_t i = 0; i < DataBlock::kSize / sizeof(uint32_t); ++i) {
			const uint32_t indirect1 = dblock_indirect2.Get_Data_Block_Idx(i);
			if (indirect1 == -1) {
				break;
			}

			DataBlock dblock_indirect1 = _data->Get(indirect1);
			for (uint32_t j = 0; j < DataBlock::kSize / sizeof(uint32_t); ++j) {
				const uint32_t direct = dblock_indirect1.Get_Data_Block_Idx(j);
				if (direct == -1) {
					break;
				}

				dblock_releaser(direct);
			}

			dblock_releaser(indirect1);
		}

		dblock_releaser(_inode.Get_Indirect2());
	}

	if (_inode.Get_Indirect1() != -1) {
		DataBlock dblock_indirect1 = _data->Get(_inode.Get_Indirect1());
		for (uint32_t i = 0; i < DataBlock::kSize / sizeof(uint32_t); ++i) {
			const uint32_t direct = dblock_indirect1.Get_Data_Block_Idx(i);
			if (direct == -1) {
				break;
			}

			dblock_releaser(direct);
		}

		dblock_releaser(_inode.Get_Indirect1());
	}

	for (uint32_t i = 0; i < 5; ++i) {
		const uint32_t direct = _inode.Get_Direct(i);
		if (direct == -1) {
			break;
		}

		dblock_releaser(direct);
	}

	return {_inode, _data};
}
