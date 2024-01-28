#pragma once

#include <memory>

#include "../data/Data.hpp"


class Inode : public A_OffsetReadableWritable {
private:
	struct s_Inode;

public:
	using t_Inode = struct s_Inode;

	static constexpr uint32_t kDirect_Refs_Cnt = 5;
	static constexpr uint32_t kDirect_Ref_Unset = -1;
	static constexpr uint32_t kIndirect_Ref_Unset = 0;

	Inode(const std::shared_ptr<I_ReadableWritable> &inode_data, size_t offset);

public:
	bool Get_Is_Dir() const;
	uint8_t Get_Refs_Cnt() const;
	uint32_t Get_File_Size() const;

	uint32_t Get_Direct(uint32_t idx) const;

	uint32_t Get_Indirect1() const;
	uint32_t Get_Indirect2() const;

	Inode &Set_Is_Dir(bool val);
	Inode &Set_Refs_Cnt(uint8_t cnt);
	Inode &Set_File_Size(uint32_t size);

	Inode &Set_Direct(uint32_t idx, uint32_t direct);
	Inode &Set_Indirect1(uint32_t indirect1);
	Inode &Set_Indirect2(uint32_t indirect2);

public:
	Inode &Unset_Directs_Indirects();

	// emmpty dir, file?

/*public:
	Iterator_DataBlocks begin_dblocks(const std::shared_ptr<Data> &data);
	static Iterator_DataBlocks end_dblocks();

	Iterator_DirItems begin_diritems(const std::shared_ptr<Data> &data);
	static Iterator_DirItems end_diritems();*/

private:
#pragma pack(push, 1)
	struct s_Inode {
		const bool Is_Dir;
		const uint8_t Refs_Cnt;
		const uint32_t File_Size;

		const uint32_t Direct[kDirect_Refs_Cnt];

		const uint32_t Indirect1;
		const uint32_t Indirect2;
	};
#pragma pack(pop)

};
