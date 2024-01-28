#pragma once

#include <vector>


/**
 * Readable Writable
 */
class I_ReadableWritable {
public:
	/**
	 * Byte Buffer Type
	 */
	using t_Byte_Buf = std::vector<std::byte>;

	/**
	 * Transparently destructs
	 */
	virtual ~I_ReadableWritable() = default;

public:
	/**
	 * Reads item T starting at idx
	 * @tparam T Type
	 * @param idx Index
	 * @return T
	 */
	template<typename T>
	T Read(size_t idx) const;
	/**
	 * Writes item T starting at idx
	 * @tparam T Type
	 * @param idx Index
	 * @param item Item
	 */
	template<typename T>
	void Write(size_t idx, T item);

	/**
	 * Reads len bytes starting at idx to buf
	 * @param buf Buffer
	 * @param idx Index
	 * @param len Length
	 */
	virtual void Read(t_Byte_Buf &buf, size_t idx, size_t len) const = 0;
	/**
	 * Writes buf of len bytes starting at idx
	 * @param buf Buffer
	 * @param idx Index
	 * @param len Length
	 */
	virtual void Write(const t_Byte_Buf &buf, size_t idx, size_t len) = 0;
};

template<typename T>
T I_ReadableWritable::Read(size_t idx) const {
	t_Byte_Buf buf;
	buf.reserve(sizeof(T));
	Read(buf, idx, buf.capacity());
	return *reinterpret_cast<T*>(buf.data());
}

template<typename T>
void I_ReadableWritable::Write(size_t idx, T item) {
	t_Byte_Buf buf;
	buf.resize(sizeof(T));
	for (size_t i = 0; i < buf.size(); ++i) {
		buf[i] = reinterpret_cast<std::byte *>(&item)[i];
	}
	Write(buf, idx, buf.size());
}
