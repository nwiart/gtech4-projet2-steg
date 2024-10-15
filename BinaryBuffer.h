#pragma once


class BinaryBuffer
{
public:

	BinaryBuffer()
		: m_buffer(0), m_size(0)
	{
	}

	BinaryBuffer(void* data, unsigned int size)
		: m_buffer((unsigned char*) data), m_size(size)
	{
	}

	inline void clear() { m_buffer = 0; m_size = 0; }

	inline bool isEmpty() const { return m_size == 0; }

	inline unsigned char* getData() const { return m_buffer; }
	inline unsigned int getSize() const { return m_size; }


private:

	unsigned char* m_buffer;
	unsigned int m_size;
};
