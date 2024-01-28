#pragma once

#include "../../inodes/Inode.hpp"
#include "../DataBlock.hpp"
#include "../Data.hpp"
#include "../../../FileSystem.hpp"


class Iterator_DataBlocks {
public:
	Iterator_DataBlocks(Inode &inode, const std::shared_ptr<Data> &data);

	Iterator_DataBlocks begin() const;
	Iterator_DataBlocks end() const;

	DataBlock operator*();
	Iterator_DataBlocks& operator++();

	bool operator==(const Iterator_DataBlocks& other) const;
	bool operator!=(const Iterator_DataBlocks& other) const;

	Iterator_DataBlocks Append_Data_Block(const t_DataBlockAcquirer &dblock_acquirer);
	Iterator_DataBlocks Release_Data_Blocks(const t_DataBlockReleaser &dblock_releaser);

private:
	Inode &_inode;
	std::shared_ptr<Data> _data;
	uint32_t _dblock_idx;

	bool Is_Depleted = false;

	bool it_directs = true;
	uint32_t it_direct_no = 0;

	bool it_indirect1 = false;
	uint32_t it_indirect1_no = 0;

	bool it_indirect2 = false;
	uint32_t it_indirect2_no = 0;
	uint32_t it_indirect2_indirect1_no = 0;
};
