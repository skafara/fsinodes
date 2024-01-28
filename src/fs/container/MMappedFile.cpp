#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <filesystem>
#include <utility>

#include "MMappedFile.hpp"
#include "io/A_OffsetReadableWritable.hpp"


MMappedFile::MMappedFile(const std::string &path) :
	_path(path), _fd(-1), _data(nullptr) {
	Open();
	Map();
}

MMappedFile::~MMappedFile() {
	Unmap();
	Close();
}

size_t MMappedFile::Get_Size() const {
	return _size;
}

void MMappedFile::Resize(size_t size) {
	Unmap();
	Close();
	std::filesystem::resize_file(_path, size);
	_size = size;
	Open();
	Map();
}

void MMappedFile::Clear() {
	for (uint32_t i = 0; i < Get_Size(); ++i) {
		_data[i] = static_cast<std::byte>(0);
	}
}

void MMappedFile::Read(t_Byte_Buf &buf, size_t idx, size_t len) const {
	if (Get_Size() == 0) {
		throw -1;
	}

	buf.reserve(buf.size() + len);
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
		throw std::ios_base::failure{"Cannot Open/Create File '" + _path + "'."};
	}
	_size = std::filesystem::file_size(_path);
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
		throw std::ios_base::failure{"Cannot Map File '" + _path + "'."};
	}
}

void MMappedFile::Unmap() {
	if (_data == nullptr || _data == MAP_FAILED) {
		return;
	}

	if (munmap(_data, Get_Size()) == -1) {
		throw std::ios_base::failure{"Cannot Unmap File '" + _path + "'."};
	}
}
