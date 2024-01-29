#pragma once

#include <memory>

#include "../data/Data.hpp"


/**
 * Inode
 */
class Inode : public A_OffsetReadableWritable {
private:
	struct s_Inode;

public:
	/**
	 * Inode Type
	 */
	using t_Inode = struct s_Inode;

	/**
	 * Direct clusters references count
	 */
	static constexpr uint32_t kDirect_Refs_Cnt = 5;
	/**
	 * Unset direct reference
	 */
	static constexpr uint32_t kDirect_Ref_Unset = -1;
	/**
	 * Unset indirect reference (also "direct references" in indirect clusters)
	 */
	static constexpr uint32_t kIndirect_Ref_Unset = 0;

	/**
	 * Transparently constructs
	 * @param container Underlying RW data
	 * @param offset Offset
	 */
	Inode(const std::shared_ptr<I_ReadableWritable> &container, size_t offset);

public:
	/**
	 * Returns whether is a directory
	 * @return Bool
	 */
	bool Get_Is_Dir() const;
	/**
	 * Returns whether is a symbolic link
	 * @return Bool
	 */
	bool Get_Is_Symlink() const;
	/**
	 * Returns references count
	 * @return Count
	 */
	uint8_t Get_Refs_Cnt() const;
	/**
	 * Returns file size [B]
	 * @return Size
	 */
	uint32_t Get_File_Size() const;

	/**
	 * Returns idx'th direct reference
	 * @param idx Index
	 * @return Reference
	 */
	uint32_t Get_Direct(uint32_t idx) const;

	/**
	 * Returns indirect reference (level 1)
	 * @return Reference
	 */
	uint32_t Get_Indirect1() const;
	/**
	 * Returns indirect reference (level 2)
	 * @return Reference
	 */
	uint32_t Get_Indirect2() const;

	/**
	 * Sets whether is a directory
	 * @param val Value
	 * @return This
	 */
	Inode &Set_Is_Dir(bool val);
	/**
	 * Sets whether is a symbolic link
	 * @param val Value
	 * @return This
	 */
	Inode &Set_Is_Symlink(bool val);
	/**
	 * Sets references count
	 * @param cnt Count
	 * @return This
	 */
	Inode &Set_Refs_Cnt(uint8_t cnt);
	/**
	 * Sets file size [B]
	 * @param size Size
	 * @return This
	 */
	Inode &Set_File_Size(uint32_t size);

	/**
	 * Sets idx'th direct reference
	 * @param idx Index
	 * @param direct Reference
	 * @return This
	 */
	Inode &Set_Direct(uint32_t idx, uint32_t direct);
	/**
	 * Sets indirect (level 1) reference
	 * @param indirect1 Reference
	 * @return This
	 */
	Inode &Set_Indirect1(uint32_t indirect1);
	/**
	 * Sets indirect (level 2) reference
	 * @param indirect2 Reference
	 * @return This
	 */
	Inode &Set_Indirect2(uint32_t indirect2);

public:
	/**
	 * Unsets direct, indirect references
	 * @return This
	 */
	Inode &Unset_References();

private:
#pragma pack(push, 1)
	struct s_Inode {
		/// Is directory
		const bool Is_Dir;
		/// Is symbolic link
		const bool Is_Symlink;
		/// References count
		const uint8_t Refs_Cnt;
		/// File size [B]
		const uint32_t File_Size;

		/// Direct clusters references
		const uint32_t Direct[kDirect_Refs_Cnt];

		/// Indirect (level 1) cluster reference
		const uint32_t Indirect1;
		/// Indirect (level 2) cluster reference
		const uint32_t Indirect2;
	};
#pragma pack(pop)

};
