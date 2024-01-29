#pragma once

#include <string>
#include <memory>

#include "../../container/io/A_OffsetReadableWritable.hpp"


/**
 * Data Block
 */
class DataBlock : public A_OffsetReadableWritable {
private:
	struct s_DirItem;

public:
	/**
	 * Directory Item Type
	 */
	using t_DirItem = struct s_DirItem;

	/**
	 * Data Block Size
	 */
	static constexpr uint32_t kSize = 4096;

	/**
	 * Directory Item Name Length
	 */
	static constexpr uint8_t kDir_Item_Name_Len = 11;

	static const t_DirItem kEmpty_Dir_Item;
	static bool Is_Empty_Dir_Item(const t_DirItem &dir_item);

public:
	/**
	 * Transparently constructs
	 * @param container Underlying RW data
	 * @param offset Offset
	 */
	DataBlock(const std::shared_ptr<I_ReadableWritable> &container, size_t offset);

	/**
	 * Reads lim bytes into buf
	 * @param buf Buffer
	 * @param lim Limit
	 */
	void Read_Content(t_Byte_Buf &buf, uint32_t lim = kSize) const;
	/**
	 * Writes lim bytes from buf
	 * @param buf Buffer
	 * @param lim Limit
	 */
	void Write_Content(const t_Byte_Buf &buf, uint32_t lim = kSize);
	/**
	 * Empties block content
	 */
	void Empty_Content();

	/**
	 * Reads idx'th directory item
	 * @param idx Index
	 * @return Directory Item
	 */
	t_DirItem Read_Dir_Item(size_t idx) const;
	/**
	 * Writes idx'th directory item
	 * @param idx Index
	 * @param item_inode_idx Item Inode Index
	 * @param item_name Item Name
	 */
	void Write_Dir_Item(size_t idx, uint32_t item_inode_idx, const std::string &item_name);

	/**
	 * Reads idx'th reference
	 * @param idx Index
	 * @return Reference
	 */
	uint32_t Read_Reference(uint32_t idx);
	/**
	 * Writes idx'th reference
	 * @param idx Index
	 * @param dblock_idx Data Block Index (reference)
	 */
	void Write_Reference(uint32_t idx, uint32_t dblock_idx);

	std::string Read_Path();
	void Write_Path(const std::string &path);

private:
	static const t_Byte_Buf kEmpty_Buf;

private:
#pragma pack(push, 1)
	struct s_DirItem {
		/// Inode Index
		const uint32_t Inode_Idx;
		/// Item Name
		char Item_Name[kDir_Item_Name_Len + 1];

		s_DirItem(uint32_t inode_idx, const char item_name[kDir_Item_Name_Len + 1]) : Inode_Idx(inode_idx) {
			std::strncpy(const_cast<char*>(Item_Name), item_name, sizeof(Item_Name));
		}
	};
#pragma pack(pop)

};