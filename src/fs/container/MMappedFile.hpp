#pragma once

#include <string>

#include "I_FSContainer.hpp"


/**
 * Memory-Mapped File
 */
class MMappedFile : public I_FSContainer {
public:
	/**
	 * Opens or creates (size 0) a file and maps the file if size > 0
	 * @param path Path Path
	 */
	MMappedFile(const std::string &path);
	/**
	 * Unmaps and closes the file
	 */
	~MMappedFile() override;

	MMappedFile(const MMappedFile &) = delete;
	MMappedFile &operator=(const MMappedFile &) = delete;

public:
	size_t Get_Size() const override;
	void Resize(size_t size) override;

	void Clear() override;

	void Read(t_Byte_Buf &buf, size_t idx, size_t len) const override;
	void Write(const t_Byte_Buf &buf, size_t idx, size_t len) override;

private:
	void Open();
	void Close();

	void Map();
	void Unmap();

private:
	const std::string _path;

	int _fd;
	std::byte *_data;
	uint32_t _size;
};
