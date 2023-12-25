#pragma once

#include <string>

#include "../fs/I_FSContainer.hpp"


class MMappedFile : public I_FSContainer {
public:
	MMappedFile(const std::string &path);
	~MMappedFile() override;

	MMappedFile(const MMappedFile &) = delete;
	MMappedFile &operator=(const MMappedFile &) = delete;

	size_t Get_Size() const override;
	void Resize(size_t size) override;

	void Read(t_Byte_Buf &buf, size_t idx, size_t len) const override;
	void Write(const t_Byte_Buf &buf, size_t idx, size_t len) override;

private:
	void Open();
	void Close();

	void Map();
	void Unmap();

	const std::string _path;
	int _fd;
	std::byte *_data;
};
