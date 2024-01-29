#include "DataBlock.hpp"

#include <sstream>


const I_ReadableWritable::t_Byte_Buf DataBlock::kEmpty_Buf = [] () {
	I_ReadableWritable::t_Byte_Buf buf;
	buf.resize(kSize);
	return buf;
}();

const DataBlock::t_DirItem DataBlock::kEmpty_Dir_Item{0, ""};

DataBlock::DataBlock(const std::shared_ptr<I_ReadableWritable> &container, size_t offset) :
		A_OffsetReadableWritable(container, offset) {
	//
}

void DataBlock::Read_Content(t_Byte_Buf &buf, uint32_t lim) const {
	Read(buf, 0, lim);
}

void DataBlock::Write_Content(const t_Byte_Buf &buf, uint32_t lim) {
	Write(buf, 0, lim);
}

void DataBlock::Empty_Content() {
	Write_Content(kEmpty_Buf);
}

DataBlock::t_DirItem DataBlock::Read_Dir_Item(size_t idx) const {
	return Read<t_DirItem>(idx * sizeof(t_DirItem));
}

void DataBlock::Write_Dir_Item(size_t idx, uint32_t item_inode_idx, const std::string &item_name) {
	t_DirItem dir_item{item_inode_idx, item_name.c_str()};
	Write(idx * sizeof(t_DirItem), dir_item);
}

uint32_t DataBlock::Read_Reference(uint32_t idx) {
	return Read<uint32_t>(idx * sizeof(uint32_t));
}

void DataBlock::Write_Reference(uint32_t idx, uint32_t dblock_idx) {
	Write(idx * sizeof(uint32_t), dblock_idx);
}

bool DataBlock::Is_Empty_Dir_Item(const DataBlock::t_DirItem &dir_item) {
	return dir_item.Inode_Idx == kEmpty_Dir_Item.Inode_Idx && std::string{dir_item.Item_Name} == std::string{kEmpty_Dir_Item.Item_Name};
}

std::string DataBlock::Read_Path() {
	std::ostringstream osstream;
	for (size_t i = 0; ; ++i) {
		const char c = Read<char>(i);
		osstream << c;
		if (c == 0x00) {
			break;
		}
	}
	return osstream.str();
}

void DataBlock::Write_Path(const std::string &path) {
	size_t i;
	for (i = 0; i < path.length(); ++i) {
		Write(i, path[i]);
	}
	Write(i, static_cast<char>(0x00));
}
