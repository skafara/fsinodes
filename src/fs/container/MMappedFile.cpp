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
	//std::memset(_data, 0, _size);
	std::filesystem::resize_file(_path, 0);
	std::filesystem::resize_file(_path, _size);
}

void MMappedFile::Read(t_Byte_Buf &buf, size_t idx, size_t len) const {
	if (Get_Size() == 0) {
		throw std::runtime_error{"Cannot Read From File '" + _path + "'"};
	}

	const size_t old_size = buf.size();
	buf.resize(old_size + len);
	std::memcpy(buf.data() + old_size, _data + idx, len);
}

void MMappedFile::Write(const t_Byte_Buf &buf, size_t idx, size_t len) {
	if (Get_Size() == 0) {
		throw std::runtime_error{"Cannot Write To File '" + _path + "'"};
	}

	std::memcpy(_data + idx, buf.data(), len);
}

void MMappedFile::Open() {
	_fd = open(_path.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if (_fd == -1) {
		throw std::runtime_error{"Cannot Open/Create File '" + _path + "'"};
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
		throw std::runtime_error{"Cannot Map File '" + _path + "'"};
	}
}

void MMappedFile::Unmap() {
	if (_data == nullptr || _data == MAP_FAILED) {
		return;
	}

	if (munmap(_data, Get_Size()) == -1) {
		throw std::runtime_error{"Cannot Unmap File '" + _path + "'"};
	}
}
