#pragma once

#include <string>
#include <memory>

#include "../../container/io/A_OffsetReadableWritable.hpp"


class DataBlock : public A_OffsetReadableWritable {
private:
	struct s_DirItem;

public:
	using t_DirItem = struct s_DirItem;

	static constexpr uint32_t kSize = 1024;

	DataBlock(const std::shared_ptr<I_ReadableWritable> &dblock_data, size_t offset);

	void Get_Content(t_Byte_Buf &buf, uint32_t lim = kSize) const;
	void Set_Content(const t_Byte_Buf &buf, uint32_t lim = kSize);
	void Empty_Content();

	t_DirItem Get_Dir_Item(size_t idx) const;
	void Set_Dir_Item(size_t idx, uint32_t item_inode_idx, const std::string &item_name); // nebo inode a char[]

	uint32_t Get_Data_Block_Idx(uint32_t idx);
	void Set_Data_Block_Idx(uint32_t idx, uint32_t dblock_idx);

private:
	static const t_Byte_Buf kEmpty_Buf;

private:
#pragma pack(push, 1)
	struct s_DirItem {
		const uint32_t Inode_Idx;
		char Item_Name[12]; // TODO magic

		s_DirItem(uint32_t inode_idx, const char item_name[12]) : Inode_Idx(inode_idx) { // nebo str?
			std::strncpy(const_cast<char*>(Item_Name), item_name, sizeof(Item_Name));
		}
	};
#pragma pack(pop)

};