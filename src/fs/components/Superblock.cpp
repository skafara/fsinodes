#include "Superblock.hpp"

#include <utility>

Superblock::Superblock(const std::shared_ptr<I_ReadableWritable> &sb_data, size_t offset) :
	A_OffsetReadableWritable(sb_data, offset) {
	//
}

void Superblock::Set(const t_Superblock &superblock) {
	Write(0, superblock);
}

uint32_t Superblock::Get_Inodes_Cnt() const {
	return Read<uint32_t>(offsetof(t_Superblock, Inodes_Cnt));
}

uint32_t Superblock::Get_Data_Blocks_Cnt() const {
	return Read<uint32_t>(offsetof(t_Superblock, Clusters_Cnt));
}

uint32_t Superblock::Get_BMap_Inodes_Start_Addr() const {
	return Read<uint32_t>(offsetof(t_Superblock, BMap_Inodes_Start_Addr));
}

uint32_t Superblock::Get_BMap_Data_Start_Addr() const {
	return Read<uint32_t>(offsetof(t_Superblock, BMap_Data_Start_Addr));
}

uint32_t Superblock::Get_Inodes_Start_Addr() const {
	return Read<uint32_t>(offsetof(t_Superblock, Inodes_Start_Addr));
}

uint32_t Superblock::Get_Data_Start_Addr() const {
	return Read<uint32_t>(offsetof(t_Superblock, Data_Start_Addr));
}
