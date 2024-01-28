#include "Iterator_DataBlocks.hpp"


Iterator_DataBlocks::Iterator_DataBlocks(Inode &inode, const std::shared_ptr<Data> &data) :
	_inode(inode), _data(data) {
	_direct_ref = _inode.Get_Direct(0);
	if (_direct_ref == Inode::kDirect_Ref_Unset) {
		Is_Depleted = true;
	}
}

DataBlock Iterator_DataBlocks::operator*() {
	return _data->Get(_direct_ref);
}

Iterator_DataBlocks &Iterator_DataBlocks::operator++() {
	it_direct_no++;
	if (it_directs) {
		if (it_direct_no >= Inode::kDirect_Refs_Cnt) {
			it_directs = false;
			it_indirect1 = true;
			it_direct_no = 0;
		}
		else {
			_direct_ref = _inode.Get_Direct(it_direct_no);
			if (_direct_ref == Inode::kDirect_Ref_Unset) {
				Is_Depleted = true;
				return *this;
			}
		}
	}
	if (it_indirect1) {
		_direct_ref = _inode.Get_Indirect1();
		if (_direct_ref == Inode::kIndirect_Ref_Unset) {
			Is_Depleted = true;
			return *this;
		}

		if (it_direct_no >= DataBlock::kSize / sizeof(uint32_t)) {
			it_indirect1 = false;
			it_indirect2 = true;
			it_indirect1_no = 0;
			it_direct_no = 0;
		}
		else {
			_direct_ref = _data->Get(_direct_ref).Get_Data_Block_Idx(it_direct_no);
			if (_direct_ref == Inode::kIndirect_Ref_Unset) {
				Is_Depleted = true;
				return *this;
			}
		}
	}
	if (it_indirect2) {
		_direct_ref = _inode.Get_Indirect2();
		if (_direct_ref == Inode::kIndirect_Ref_Unset) {
			Is_Depleted = true;
			return *this;
		}

		if (it_direct_no >= DataBlock::kSize / sizeof(uint32_t)) {
			it_indirect1_no++;
			it_direct_no = 0;
		}
		if (it_indirect1_no >= DataBlock::kSize / sizeof(uint32_t)) {
			Is_Depleted = true;
			return *this;
		}

		const uint32_t dblock_indirect1_idx = _data->Get(_inode.Get_Indirect2()).Get_Data_Block_Idx(it_indirect1_no);
		if (dblock_indirect1_idx == Inode::kIndirect_Ref_Unset) {
			Is_Depleted = true;
			return *this;
		}

		_direct_ref = _data->Get(dblock_indirect1_idx).Get_Data_Block_Idx(it_direct_no);
		if (_direct_ref == Inode::kIndirect_Ref_Unset) {
			Is_Depleted = true;
			return *this;
		}
	}

	return *this;
}

bool Iterator_DataBlocks::operator==(const Iterator_DataBlocks &other) const {
	return (this->Is_Depleted && other.Is_Depleted) || (this->_direct_ref == other._direct_ref); // ?
}

bool Iterator_DataBlocks::operator==(bool other) const {
	return Is_Depleted != other;
}

bool Iterator_DataBlocks::operator!=(const Iterator_DataBlocks &other) const {
	return !(*this == other);
}

bool Iterator_DataBlocks::operator!=(bool other) const {
	return !(*this == other);
}

Iterator_DataBlocks Iterator_DataBlocks::Append(const t_DataBlockAcquirer& acquirer) {
	for (; *this != kDepleted; ++(*this));
	Is_Depleted = false;

	_direct_ref = acquirer();

	if (it_directs) {
		_inode.Set_Direct(it_direct_no, _direct_ref);
	}
	if (it_indirect1) {
		if (_inode.Get_Indirect1() == Inode::kIndirect_Ref_Unset) {
			_inode.Set_Indirect1(_direct_ref);
			_direct_ref = acquirer();
		}

		_data->Get(_inode.Get_Indirect1()).Set_Data_Block_Idx(it_direct_no, _direct_ref);
	}
	if (it_indirect2) {
		if (_inode.Get_Indirect2() == Inode::kIndirect_Ref_Unset) {
			_inode.Set_Indirect2(_direct_ref);
			_direct_ref = acquirer();
		}

		DataBlock dblock_indirect2 = _data->Get(_inode.Get_Indirect2());
		if (dblock_indirect2.Get_Data_Block_Idx(it_indirect1_no) == Inode::kIndirect_Ref_Unset) {
			dblock_indirect2.Set_Data_Block_Idx(it_indirect1_no, _direct_ref);
			_direct_ref = acquirer();
		}

		DataBlock dblock_indirect1 = _data->Get(dblock_indirect2.Get_Data_Block_Idx(it_indirect1_no));
		dblock_indirect1.Set_Data_Block_Idx(it_direct_no, _direct_ref);
	}

	return *this;
}

void Iterator_DataBlocks::Release_All(Inode &inode, const std::shared_ptr<Data> &data, const t_DataBlockReleaser &releaser) {
	if (inode.Get_Indirect2() != Inode::kIndirect_Ref_Unset) {
		DataBlock dblock_indirect2 = data->Get(inode.Get_Indirect2());
		for (uint32_t i = 0; i < DataBlock::kSize / sizeof(uint32_t); ++i) {
			const uint32_t indirect1 = dblock_indirect2.Get_Data_Block_Idx(i);
			if (indirect1 == Inode::kIndirect_Ref_Unset) {
				break;
			}

			DataBlock dblock_indirect1 = data->Get(indirect1);
			for (uint32_t j = 0; j < DataBlock::kSize / sizeof(uint32_t); ++j) {
				const uint32_t direct = dblock_indirect1.Get_Data_Block_Idx(j);
				if (direct == Inode::kIndirect_Ref_Unset) {
					break;
				}

				releaser(direct);
			}

			releaser(indirect1);
		}

		releaser(inode.Get_Indirect2());
	}

	if (inode.Get_Indirect1() != Inode::kIndirect_Ref_Unset) {
		DataBlock dblock_indirect1 = data->Get(inode.Get_Indirect1());
		for (uint32_t i = 0; i < DataBlock::kSize / sizeof(uint32_t); ++i) {
			const uint32_t direct = dblock_indirect1.Get_Data_Block_Idx(i);
			if (direct == Inode::kIndirect_Ref_Unset) {
				break;
			}

			releaser(direct);
		}

		releaser(inode.Get_Indirect1());
	}

	for (uint32_t i = 0; i < Inode::kDirect_Refs_Cnt; ++i) {
		const uint32_t direct = inode.Get_Direct(i);
		if (direct == Inode::kDirect_Ref_Unset) {
			break;
		}

		releaser(direct);
	}
}
