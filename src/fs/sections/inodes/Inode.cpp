#include "Inode.hpp"


Inode::Inode(const std::shared_ptr<I_ReadableWritable> &container, size_t offset) :
		A_OffsetReadableWritable(container, offset) {
	//
}

bool Inode::Get_Is_Dir() const {
	return Read<bool>(offsetof(t_Inode, Is_Dir));
}

bool Inode::Get_Is_Symlink() const {
	return Read<bool>(offsetof(t_Inode, Is_Symlink));
}

uint8_t Inode::Get_Refs_Cnt() const {
	return Read<uint32_t>(offsetof(t_Inode, Refs_Cnt));
}

uint32_t Inode::Get_File_Size() const {
	return Read<uint32_t>(offsetof(t_Inode, File_Size));
}

uint32_t Inode::Get_Direct(uint32_t idx) const {
	return Read<uint32_t>(offsetof(t_Inode, Direct[idx]));
}

uint32_t Inode::Get_Indirect1() const {
	return Read<uint32_t>(offsetof(t_Inode, Indirect1));
}

uint32_t Inode::Get_Indirect2() const {
	return Read<uint32_t>(offsetof(t_Inode, Indirect2));
}

Inode &Inode::Set_Is_Dir(bool val) {
	Write(offsetof(t_Inode, Is_Dir), val);
	return *this;
}

Inode &Inode::Set_Is_Symlink(bool val) {
	Write(offsetof(t_Inode, Is_Symlink), val);
	return *this;
}

Inode &Inode::Set_Refs_Cnt(uint8_t cnt) {
	Write(offsetof(t_Inode, Refs_Cnt), cnt);
	return *this;
}

Inode &Inode::Set_File_Size(uint32_t size) {
	Write(offsetof(t_Inode, File_Size), size);
	return *this;
}

Inode &Inode::Set_Direct(uint32_t idx, uint32_t direct) {
	Write(offsetof(t_Inode, Direct[idx]), direct);
	return *this;
}

Inode &Inode::Unset_References() {
	for (uint32_t i = 0; i < kDirect_Refs_Cnt; ++i) {
		Set_Direct(i, kDirect_Ref_Unset);
	}
	Set_Indirect1(kIndirect_Ref_Unset);
	Set_Indirect2(kIndirect_Ref_Unset);
	return *this;
}

Inode &Inode::Set_Indirect1(uint32_t indirect1) {
	Write(offsetof(t_Inode, Indirect1), indirect1);
	return *this;
}

Inode &Inode::Set_Indirect2(uint32_t indirect2) {
	Write(offsetof(t_Inode, Indirect2), indirect2);
	return *this;
}
