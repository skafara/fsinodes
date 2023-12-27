#pragma once

#include <memory>

#include "../../io/A_OffsetReadableWritable.hpp"


constexpr uint32_t kInode_Directs_Cnt = 5;

#pragma pack(push, 1)

struct s_Inode {
	const bool Is_Dir{};
	const uint8_t Refs_Cnt{};
	const uint32_t File_Size{};

	const uint32_t Direct[kInode_Directs_Cnt]{};

	const uint32_t Indirect1{};
	const uint32_t Indirect2{};
};
#pragma pack(pop)

using t_Inode = struct s_Inode;

class Inode : public A_OffsetReadableWritable {
public:
	static constexpr uint32_t kRef_Unset = -1;

	Inode(const std::shared_ptr<I_ReadableWritable> &inode_data, size_t offset);

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
	Inode &Unset_Directs_Indirects();

	Inode &Set_Indirect1(uint32_t indirect1);
	Inode &Set_Indirect2(uint32_t indirect2);

	// emmpty dir, file?
};
