#include "DataBlock.hpp"


DataBlock::DataBlock(const std::shared_ptr<I_ReadableWritable> &dblock_data, size_t offset) :
		A_OffsetReadableWritable(dblock_data, offset) {
	//
}

void DataBlock::Get_Content(t_Byte_Buf &buf, uint32_t lim) const {
	Read(buf, 0, lim);
}

void DataBlock::Set_Content(const t_Byte_Buf &buf, uint32_t lim) {
	Write(buf, 0, lim);
}

DataBlock::t_DirItem DataBlock::Get_Dir_Item(size_t idx) const {
	return Read<t_DirItem>(idx * sizeof(t_DirItem));
}

void DataBlock::Set_Dir_Item(size_t idx, uint32_t item_inode_idx, const std::string &item_name) {
	t_DirItem dir_item{item_inode_idx, item_name.c_str()};
	Write(idx * sizeof(t_DirItem), dir_item);
}

uint32_t DataBlock::Get_Data_Block_Idx(uint32_t idx) {
	return Read<uint32_t>(idx * sizeof(uint32_t));
}

void DataBlock::Set_Data_Block_Idx(uint32_t idx, uint32_t dblock_idx) {
	Write(idx * sizeof(uint32_t), dblock_idx);
}
