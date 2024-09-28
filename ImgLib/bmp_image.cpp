#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib {

    PACKED_STRUCT_BEGIN BitmapFileHeader{
        char signature[2]; 
        uint32_t file_size;
        uint32_t reserved; 
        uint32_t data_offset; 
    }
        PACKED_STRUCT_END

     
        PACKED_STRUCT_BEGIN BitmapInfoHeader{
            uint32_t header_size;
            int32_t width; 
            int32_t height; 
            uint16_t planes;
            uint16_t bit_count; 
            uint32_t compression;
            uint32_t image_size; 
            int32_t x_resolution;
            int32_t y_resolution;
            uint32_t colors_used;
            uint32_t important_colors; 
    }
        PACKED_STRUCT_END

        static int GetBMPStride(int w) {
        return 4 * ((w * 3 + 3) / 4);
    }

    bool SaveBMP(const Path& file, const Image& image) {
        ofstream out(file, ios::binary);
        if (!out) return false;

        const int width = image.GetWidth();
        const int height = image.GetHeight();
        const int stride = GetBMPStride(width);

        BitmapFileHeader file_header = {
            {'B', 'M'},
            static_cast<uint32_t>(sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + stride * height),
            0,
            static_cast<uint32_t>(sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader))
        };

        BitmapInfoHeader info_header = {
            sizeof(BitmapInfoHeader),
            width,
            height,
            1,
            24,
            0,
            static_cast<uint32_t>(stride * height),
            11811,  
            11811,  
            0,
            0x1000000
        };


        out.write(reinterpret_cast<const char*>(&file_header), sizeof(file_header));
        out.write(reinterpret_cast<const char*>(&info_header), sizeof(info_header));

 
        std::array<std::byte, 3> padding = { std::byte{0}, std::byte{0}, std::byte{0} };
        for (int y = height - 1; y >= 0; --y) {
            const Color* line = image.GetLine(y);
            for (int x = 0; x < width; ++x) {
                std::array<std::byte, 3> bgr = { line[x].b, line[x].g, line[x].r };
                out.write(reinterpret_cast<const char*>(bgr.data()), 3);
            }
            out.write(reinterpret_cast<const char*>(padding.data()), stride - width * 3);
        }

        return true;
    }


    Image LoadBMP(const Path& file) {
        ifstream in(file, ios::binary);
        if (!in) return {};

        BitmapFileHeader file_header;
        BitmapInfoHeader info_header;
        in.read(reinterpret_cast<char*>(&file_header), sizeof(file_header));
        in.read(reinterpret_cast<char*>(&info_header), sizeof(info_header));

        if (file_header.signature[0] != 'B' || file_header.signature[1] != 'M' || info_header.bit_count != 24 || info_header.compression != 0) {
            return {};
        }

        const int width = info_header.width;
        const int height = info_header.height;
        const int stride = GetBMPStride(width);


        Image image(width, height, Color::Black());

        in.seekg(file_header.data_offset, ios::beg);
        for (int y = height - 1; y >= 0; --y) {
            Color* line = image.GetLine(y);
            for (int x = 0; x < width; ++x) {
                std::array<std::byte, 3> bgr;
                in.read(reinterpret_cast<char*>(bgr.data()), 3);
                line[x] = { bgr[2], bgr[1], bgr[0], std::byte{255} };  
            }
            in.ignore(stride - width * 3);
        }

        return image;
    }

}  // namespace img_lib
