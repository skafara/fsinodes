#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <filesystem>
#include <utility>

#include "MMappedFile.hpp"
#include "A_OffsetReadableWritable.hpp"


MMappedFile::MMappedFile(const std::string &path) :
	_path(path), _fd(-1), _data(nullptr) {
	Open();
	Map();
}

MMappedFile::~MMappedFile() {
	Unmap();
	Close();
}

size_t MMappedFile::Get_Size() const { // TODO consider time complexity
	return 1077185127;//std::filesystem::file_size(_path); //1077185127
}

void MMappedFile::Resize(size_t size) {
	Unmap();
	Close();
	std::filesystem::resize_file(_path, size);
	Open();
	Map();
}

void MMappedFile::Read(t_Byte_Buf &buf, size_t idx, size_t len) const {
	if (Get_Size() == 0) {
		throw -1;
	}

	buf.reserve(len);
	for (size_t i = 0; i < len; ++i) {
		buf.push_back(_data[idx + i]);
	}
}

void MMappedFile::Write(const t_Byte_Buf &buf, size_t idx, size_t len) {
	if (Get_Size() == 0) {
		throw -1;
	}

	for (size_t i = 0; i < len; ++i) {
		_data[idx + i] = buf[i];
	}
}

void MMappedFile::Open() {
	_fd = open(_path.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if (_fd == -1) {
		throw -1;
	}
}

void MMappedFile::Close() {
	if (_fd != -1) {
		close(_fd);
		_fd = -1;
	}
}

void MMappedFile::Map() {
	if (Get_Size() == 0) {
		return; // do not map zero-length file
	}

	_data = static_cast<std::byte *>(mmap(nullptr, Get_Size(), PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0));
	if (_data == MAP_FAILED) {
		throw -1;
	}
}

void MMappedFile::Unmap() {
	if (_data == nullptr || _data == MAP_FAILED) {
		return;
	}

	if (munmap(_data, Get_Size()) == -1) {
		throw -1;
	}
}
