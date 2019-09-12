#pragma once

#include <string>

namespace nt
{
int decodebase64(const void *pvBase64, size_t nBase64Len, void *apvBuffer, size_t nBufferLen );
int base64decode(const char *inbuf, size_t inLen, unsigned char *out, size_t outLen);

int decodebase64_buf_len(size_t nBase64Len);
int encodebase64_buf_len(size_t len);

std::string base64encode(const char *buf, size_t len);
std::string base64decode(const char *pvBase64, size_t nBase64Len);
}
