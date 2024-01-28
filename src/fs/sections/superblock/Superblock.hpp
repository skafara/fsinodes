#pragma once

#include "../../container/io/A_OffsetReadableWritable.hpp"


/**
 * Superblock
 */
class Superblock : public A_OffsetReadableWritable {
private:
	struct s_Superblock;

public:
	/**
	 * Superblock Type
	 */
	using t_Superblock = struct s_Superblock;

	/**
	 * Transparently constructs
	 * @param container Underlying RW data
	 * @param offset Offset
	 */
	Superblock(const std::shared_ptr<I_ReadableWritable> &container, size_t offset);

	/**
	 * Sets superblock contents
	 * @param superblock Superblock
	 */
	void Set(const t_Superblock &superblock);

public:
	/**
	 * Returns inodes count
	 * @return Inodes Count
	 */
	uint32_t Get_Inodes_Cnt() const;
	/**
	 * Returns data blocks count
	 * @return Data Blocks Count
	 */
	uint32_t Get_Data_Blocks_Cnt() const;

	/**
	 * Returns inodes bitmap start address
	 * @return Inodes Bitmap Start Address
	 */
	uint32_t Get_BMap_Inodes_Start_Addr() const;
	/**
	 * Returns data blocks bitmap start address
	 * @return Data Blocks Bitmap Start Address
	 */
	uint32_t Get_BMap_Data_Start_Addr() const;

	/**
	 * Returns inodes start address
	 * @return Inodes Start Address
	 */
	uint32_t Get_Inodes_Start_Addr() const;
	/**
	 * Returns data start address
	 * @return Data Start Address
	 */
	uint32_t Get_Data_Start_Addr() const;

private:
#pragma pack(push, 1)
	struct s_Superblock {
		/// Author Signature
		const char Signature[9];
		/// Volume description
		const char Volume_Desc[251];

		/// Total FS size
		const uint32_t Disk_Size;
		/// Logical size
		const uint32_t Cluster_Size;
		/// Number of inodes
		const uint32_t Inodes_Cnt;
		/// Number of clusters
		const uint32_t Clusters_Cnt;

		/// Start address of Inodes bitmap
		const uint32_t BMap_Inodes_Start_Addr;
		/// Start address of Data blocks bitmap
		const uint32_t BMap_Data_Start_Addr;
		/// Start address of Inodes section
		const uint32_t Inodes_Start_Addr;
		/// Start address of Data blocks section
		const uint32_t Data_Start_Addr;
	};
#pragma pack(pop)

};
