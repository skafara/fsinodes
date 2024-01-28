#pragma once

#include "../../inodes/Inode.hpp"


class Iterator_DataBlocks {
private:
	using t_DataBlockAcquirer = std::function<uint32_t ()>;
	using t_DataBlockReleaser = std::function<void (uint32_t)>;

public:
	static constexpr bool kDepleted = false;

	Iterator_DataBlocks(Inode &inode, const std::shared_ptr<Data> &data);

	DataBlock operator*();
	Iterator_DataBlocks& operator++();

	bool operator==(const Iterator_DataBlocks &other) const;
	bool operator==(bool other) const;
	bool operator!=(const Iterator_DataBlocks &other) const;
	bool operator!=(bool other) const;

	Iterator_DataBlocks Append(const t_DataBlockAcquirer &acquirer);
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