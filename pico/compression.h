#ifndef __PICO_COMPRESSION_H__
#define __PICO_COMPRESSION_H__

#include <string>
#include <zlib.h>

namespace pico {

namespace compression {
enum CompressionType
{
    DEFLATE = 15,
    GZIP = 15 | 16,

};

std::string compress(const std::string& data, CompressionType type = DEFLATE);

std::string decompress(const std::string& data, CompressionType type = DEFLATE);

bool is_compression_enabled();

void set_compression_enabled(bool enabled);

}   // namespace compression
}   // namespace pico

#endif