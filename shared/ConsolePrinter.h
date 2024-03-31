#pragma once

#include <mutex>
#include <malloc.h>

class IConsolePrint {
public:
	virtual void Print(const char * text) = 0;
};

class CConsolePrinter {
public:
	void Print(IConsolePrint * pConsolePrint, const char* fmt, va_list args) {
		std::unique_lock<std::mutex> lock(m_Print_Mutex);
		if(nullptr == m_Print_Memory) {
			m_Print_Memory = (char *)malloc(sizeof(char)*512);
			if(nullptr != m_Print_Memory) m_Print_MemorySize = 512;
			else return;
		}
		int chars = vsnprintf(m_Print_Memory,m_Print_MemorySize,fmt,args);
		if(chars < 0) return;
		if(chars >= m_Print_MemorySize) {
			m_Print_Memory = (char *)realloc(m_Print_Memory,sizeof(char)*(chars+1));
			if(nullptr != m_Print_Memory) m_Print_MemorySize = chars+1;
			else return;		
			vsnprintf(m_Print_Memory,m_Print_MemorySize,fmt,args);
		}
		char * ptr = m_Print_Memory;
		for(int i = 0; true; ) {
			bool bEndReached = false;
			bool bNewLine = false;
			size_t length = 1;
			size_t size = 1;
			unsigned char lb = ptr[i];
			if (( lb & 0x80 ) == 0 ) { // lead bit is zero, must be a single ascii
				switch(lb) {
				case '\0':
					length = 0;
					bEndReached = true;
					break;
				case '\n':
					length = 0;
					bNewLine = true;
					break;
				default:
					length = 1;
				}
			}
			else if (( lb & 0xE0 ) == 0xC0 ) { // 110x xxxx
				size = 2;
			} else if (( lb & 0xF0 ) == 0xE0 ) { // 1110 xxxx
				size = 3;
			} else if (( lb & 0xF8 ) == 0xF0 ) { // 1111 0xxx
				size = 4;
			} else {
				// invalid UTF-8 length.
				ptr[i] = '\0';
				bEndReached = true;
			}
			if(i+size > chars) {
				// UTF-8 too long / invalid
				ptr[i] = '\0';
				bEndReached = true;
				length = 0;
				size = 1;
			} else {
				for(size_t j=1; j < size; j++) {		
					if((ptr[i+j] & 0xC0) != 0x80) {
						// following octets must be 0x10xxxxxx, so  this is invalid.
						ptr[i] = '\0';
						bEndReached = true;
						length = 0;
						size = 1;
						break;
					}
				}
			}
			if(bEndReached) {
				pConsolePrint->Print(ptr);
				m_Print_Length += length;
				break;
			}
			bool bSizeLimitReached = i + size >= m_Print_SizeLimit;
			if(bSizeLimitReached) {
				char tmp = ptr[i];
				ptr[i] = '\0';
				pConsolePrint->Print(ptr);
				ptr[i] = tmp;
				ptr = &(ptr[i]);
				m_Print_Length += length;
				chars -= i;
				i = 0;
				continue;
			}
			if(bNewLine) {
				i += size;
				m_Print_Length = 0;
			} else {
				bool bLineLimitReached = 0 < m_Print_LineLimit && m_Print_Length + length >= m_Print_LineLimit;
				if(bLineLimitReached) {
					char tmp = ptr[i];
					char tmp2 = ptr[i+1];
					ptr[i] = '\n';
					ptr[i+1] = '\0';
					pConsolePrint->Print(ptr);
					ptr[i] = tmp;
					ptr[i+1] = tmp2;
					ptr = &(ptr[i]);
					m_Print_Length = 0;
					chars -= i;
					i = 0;
					continue;
				} else {
					i += size;
					m_Print_Length += length;
				}
			}
		}
	}

private:
	std::mutex m_Print_Mutex;
	char * m_Print_Memory = nullptr;
	size_t m_Print_MemorySize = 0;
	size_t m_Print_Length = 0;
	size_t m_Print_SizeLimit = 200;
	size_t m_Print_LineLimit = 240; // There's currently a bug with the CS2 console where it won't show text if there's no line-break after 300 characters.
};
