#include "compression.h"

#include "logging.h"

namespace pico {
namespace compression {
static bool g_compression_enabled = false;

bool is_compression_enabled() {
    return g_compression_enabled;
}

void set_compression_enabled(bool enabled) {
    g_compression_enabled = enabled;
}

std::string compress(const std::string& data, CompressionType type) {
    std::string compressed_str;
    z_stream stream{};
    if (::deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, type, 8, Z_DEFAULT_STRATEGY) ==
        Z_OK) {
        char buffer[1024] = {};

        stream.avail_in = static_cast<uInt>(data.size());
        stream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(data.c_str()));

        int code = Z_OK;
        do {
            stream.avail_out = sizeof(buffer);
            stream.next_out = reinterpret_cast<Bytef*>(&buffer[0]);
            code = ::deflate(&stream, Z_FINISH);
            if (code == Z_OK || code == Z_STREAM_END) {
                std::copy(&buffer[0],
                          &buffer[sizeof(buffer) - stream.avail_out],
                          std::back_inserter(compressed_str));
            }
        } while (code == Z_OK);

        ::deflateEnd(&stream);

        if (code != Z_STREAM_END) {
            LOG_ERROR("Compression failed: %s", stream.msg);
            compressed_str.clear();
        }
    }
    return compressed_str;
}

std::string decompress(const std::string& data, CompressionType type) {
    std::string decompressed_str;
    Bytef buffer[8192] = {};

    z_stream stream{};
    stream.avail_in = static_cast<uInt>(data.size());
    stream.next_in = const_cast<Bytef*>(reinterpret_cast<Bytef const*>(data.c_str()));

    if (::inflateInit2(&stream, type) == Z_OK) {
        do {
            stream.avail_out = sizeof(buffer);
            stream.next_out = reinterpret_cast<Bytef*>(&buffer[0]);
            int code = ::inflate(&stream, Z_NO_FLUSH);
            if (code == Z_OK || code == Z_STREAM_END) {
                std::copy(&buffer[0],
                          &buffer[sizeof(buffer) - stream.avail_out],
                          std::back_inserter(decompressed_str));
            }
            else {
                LOG_ERROR("Decompression failed: %s", stream.msg);
                decompressed_str.clear();
                break;
            }
        } while (stream.avail_out == 0);
    }
    return decompressed_str;
}

}   // namespace compression
}   // namespace pico