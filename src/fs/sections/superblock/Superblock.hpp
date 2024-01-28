#pragma once

#include "../../container/io/A_OffsetReadableWritable.hpp"


class Superblock : public A_OffsetReadableWritable {
private:
	struct s_Superblock;

public:
	using t_Superblock = struct s_Superblock;

	Superblock(const std::shared_ptr<I_ReadableWritable> &sb_data, size_t offset);

	void Set(const t_Superblock &superblock);

public:
	uint32_t Get_Inodes_Cnt() const;
	uint32_t Get_Data_Blocks_Cnt() const;

	uint32_t Get_BMap_Inodes_Start_Addr() const;
	uint32_t Get_BMap_Data_Start_Addr() const;

	uint32_t Get_Inodes_Start_Addr() const;
	uint32_t Get_Data_Start_Addr() const;

private:
#pragma pack(push, 1)
	struct s_Superblock { // TODO desc a typy
		/** Author Signature */
		const char Signature[9];
		// Volume description
		const char Volume_Desc[251];

		// Total FS size
		const uint32_t Disk_Size;
		// Logical size
		const uint32_t Cluster_Size;
		// Number of inodes
		const uint32_t Inodes_Cnt;
		// Number of clusters
		const uint32_t Clusters_Cnt;

		// Start address of Inodes bitmap
		const uint32_t BMap_Inodes_Start_Addr;
		// Start address of Data blocks bitmap
		const uint32_t BMap_Data_Start_Addr;
		// Start address of Inodes section
		const uint32_t Inodes_Start_Addr;
		// Start address of Data blocks section
		const uint32_t Data_Start_Addr;
	};
#pragma pack(pop)

};
