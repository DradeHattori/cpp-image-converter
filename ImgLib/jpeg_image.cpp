#include "jpeg_image.h"
#include <jpeglib.h>
#include <cstdio>
#include <vector>

namespace img_lib {

    bool SaveJPEG(const Path& file, const Image& image) {
        jpeg_compress_struct cinfo;
        jpeg_error_mgr jerr;

        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&cinfo);

        FILE* outfile;

#ifdef _MSC_VER
        if ((outfile = _wfopen(file.wstring().c_str(), L"wb")) == NULL) {
#else
        if ((outfile = fopen(file.string().c_str(), "wb")) == NULL) {
#endif
            return false;
        }

        jpeg_stdio_dest(&cinfo, outfile);

        cinfo.image_width = image.GetWidth();
        cinfo.image_height = image.GetHeight();
        cinfo.input_components = 3;
        cinfo.in_color_space = JCS_RGB;

        jpeg_set_defaults(&cinfo);

        jpeg_start_compress(&cinfo, TRUE);

        std::vector<JSAMPLE> row_buffer(image.GetWidth() * 3);

        while (cinfo.next_scanline < cinfo.image_height) {
            const Color* line = image.GetLine(cinfo.next_scanline);  

            for (int x = 0; x < image.GetWidth(); ++x) {
                row_buffer[x * 3] = static_cast<JSAMPLE>(line[x].r);  
                row_buffer[x * 3 + 1] = static_cast<JSAMPLE>(line[x].g); 
                row_buffer[x * 3 + 2] = static_cast<JSAMPLE>(line[x].b); 
            }

            JSAMPROW row_pointer = row_buffer.data();
            jpeg_write_scanlines(&cinfo, &row_pointer, 1);
        }

        jpeg_finish_compress(&cinfo);
        fclose(outfile);
        jpeg_destroy_compress(&cinfo);

        return true;
        }

    Image LoadJPEG(const Path& file) {
        jpeg_decompress_struct cinfo;
        jpeg_error_mgr jerr;

        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_decompress(&cinfo);

        FILE* infile;

#ifdef _MSC_VER
        if ((infile = _wfopen(file.wstring().c_str(), L"rb")) == NULL) {
#else
        if ((infile = fopen(file.string().c_str(), "rb")) == NULL) {
#endif
            return {};
        }

        jpeg_stdio_src(&cinfo, infile);
        jpeg_read_header(&cinfo, TRUE);
        jpeg_start_decompress(&cinfo);

        Image image(cinfo.output_width, cinfo.output_height, Color::Black());
        std::vector<JSAMPLE> row_buffer(cinfo.output_width * 3);

        while (cinfo.output_scanline < cinfo.output_height) {
            JSAMPROW row_pointer = row_buffer.data();
            jpeg_read_scanlines(&cinfo, &row_pointer, 1);

            Color* line = image.GetLine(cinfo.output_scanline);
            for (int x = 0; x < cinfo.output_width; ++x) {
                line[x].r = static_cast<std::byte>(row_buffer[x * 3 + 0]);
                line[x].g = static_cast<std::byte>(row_buffer[x * 3 + 1]);
                line[x].b = static_cast<std::byte>(row_buffer[x * 3 + 2]);
            }
        }

        jpeg_finish_decompress(&cinfo);
        fclose(infile);
        jpeg_destroy_decompress(&cinfo);

        return image;
        }
    } // namespace img_lib


