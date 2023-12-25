#pragma once

#include <string>
#include <memory>

#include "../../io/A_OffsetReadableWritable.hpp"


#pragma pack(push, 1)
struct s_DataBlock { // netreba?
	const std::byte Data[1024]; // TODO magic
};
#pragma pack(pop)

#pragma pack(push, 1)
struct s_DirItem {
	const uint32_t Inode_Idx;
	char Item_Name[12]; // TODO magic

	s_DirItem(uint32_t inode_idx, const char item_name[12]) : Inode_Idx(inode_idx) { // nebo str?
		std::strncpy(const_cast<char*>(Item_Name), item_name, sizeof(Item_Name));
	}
};
#pragma pack(pop)

using t_DataBlock = struct s_DataBlock;
using t_DirItem = struct s_DirItem;

class DataBlock : public A_OffsetReadableWritable {
public:
	DataBlock(const std::shared_ptr<I_ReadableWritable> &dblock_data, size_t offset);

	void Get_Content(t_Byte_Buf &buf, uint32_t lim = 1024) const;
	void Set_Content(const t_Byte_Buf &buf, uint32_t lim = 1024);

	t_DirItem Get_Dir_Item(size_t idx) const;
	void Set_Dir_Item(size_t idx, uint32_t item_inode_idx, const std::string &item_name); // nebo inode a char[]

	uint32_t Get_Data_Block_Idx(uint32_t idx);
	void Set_Data_Block_Idx(uint32_t idx, uint32_t dblock_idx);
};