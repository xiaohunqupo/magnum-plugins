/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
                2020, 2021, 2022, 2023, 2024, 2025
              Vladimír Vondruš <mosra@centrum.cz>
    Copyright © 2019 Jonathan Hale <squareys@googlemail.com>
    Copyright © 2021, 2022, 2024 Pablo Escobar <mail@rvrs.in>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#include "BasisImageConverter.h"

#include <thread>
#include <Corrade/Containers/Array.h>
#include <Corrade/Containers/ArrayView.h>
#include <Corrade/Containers/Optional.h>
#include <Corrade/Containers/Pair.h>
#include <Corrade/Containers/StridedArrayView.h>
#include <Corrade/Containers/String.h>
#include <Corrade/Utility/ConfigurationGroup.h>
#include <Corrade/Utility/Path.h>
#include <Corrade/Utility/String.h>
#include <Magnum/ImageView.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Swizzle.h>
#include <Magnum/PixelFormat.h>

#include <basisu_enc.h>
#include <basisu_comp.h>
#include <basisu_file_headers.h>
#if BASISU_LIB_VERSION >= 116
#include <basisu_opencl.h>
#endif

namespace Magnum { namespace Trade {

using namespace Containers::Literals;

#ifndef MAGNUM_BUILD_DEPRECATED
enum class BasisImageConverter::Format: Int {
    /* 0 used for default value, Basis unless overridden by
       convertToFile */
    Basis = 1,    /* Output Basis images */
    Ktx,          /* Output KTX2 images */
};
#endif

namespace {

template<typename T> void copySlice(const ImageView3D& image3D, UnsignedInt slice, Containers::StridedArrayView2D<Math::Color4<T>>& dst, bool yFlip, bool hasCustomSwizzle) {
    const UnsignedInt channelCount = pixelFormatChannelCount(image3D.format());

    if(yFlip)
        dst = dst.template flipped<0>();

    /* basis image is always RGBA, fill in alpha if necessary */
    if(channelCount == 4) {
        const auto src = image3D.pixels<Math::Vector<4, T>>()[slice];
        for(std::size_t y = 0; y != src.size()[0]; ++y)
            for(std::size_t x = 0; x != src.size()[1]; ++x)
                dst[y][x] = src[y][x];

    } else if(channelCount == 3) {
        const auto src = image3D.pixels<Math::Vector<3, T>>()[slice];
        for(std::size_t y = 0; y != src.size()[0]; ++y)
            for(std::size_t x = 0; x != src.size()[1]; ++x)
                dst[y][x] = Math::Vector3<T>{src[y][x]}; /* Alpha implicitly opaque */

    } else if(channelCount == 2) {
        const auto src = image3D.pixels<Math::Vector<2, T>>()[slice];
        /* If the user didn't specify a custom swizzle, assume they want
           the two channels compressed in separate slices, R in RGB and G
           in Alpha. This significantly improves quality. */
        if(!hasCustomSwizzle)
            for(std::size_t y = 0; y != src.size()[0]; ++y)
                for(std::size_t x = 0; x != src.size()[1]; ++x)
                    dst[y][x] = Math::gather<'r', 'r', 'r', 'g'>(src[y][x]);
        else
            for(std::size_t y = 0; y != src.size()[0]; ++y)
                for(std::size_t x = 0; x != src.size()[1]; ++x)
                    dst[y][x] = Math::Vector3<T>::pad(src[y][x]); /* Alpha implicitly opaque */

    } else if(channelCount == 1) {
        const auto src = image3D.pixels<Math::Vector<1, T>>()[slice];
        /* If the user didn't specify a custom swizzle, assume they want
           a gray-scale image. Alpha is always implicitly opaque. */
        if(!hasCustomSwizzle)
            for(std::size_t y = 0; y != src.size()[0]; ++y)
                for(std::size_t x = 0; x != src.size()[1]; ++x)
                    dst[y][x] = Math::gather<'r', 'r', 'r'>(src[y][x]);
        else
            for(std::size_t y = 0; y != src.size()[0]; ++y)
                for(std::size_t x = 0; x != src.size()[1]; ++x)
                    dst[y][x] = Math::Vector3<T>::pad(src[y][x]); /* Alpha implicitly opaque */

    } else CORRADE_INTERNAL_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
}

template<UnsignedInt dimensions> Containers::Optional<Containers::Array<char>> convertLevelsToData(const Containers::ArrayView<const BasicImageView<dimensions>> imageLevels, const Utility::ConfigurationGroup& configuration, const ImageConverterFlags flags, const bool isKtx) {
    /* Check input */
    const PixelFormat pixelFormat = imageLevels.front().format();
    bool isSrgb;
    #if BASISU_LIB_VERSION >= 150
    bool isHdr;
    #endif
    switch(pixelFormat) {
        case PixelFormat::RGBA8Unorm:
        case PixelFormat::RGB8Unorm:
        case PixelFormat::RG8Unorm:
        case PixelFormat::R8Unorm:
            isSrgb = false;
            #if BASISU_LIB_VERSION >= 150
            isHdr = false;
            #endif
            break;
        case PixelFormat::RGBA8Srgb:
        case PixelFormat::RGB8Srgb:
        case PixelFormat::RG8Srgb:
        case PixelFormat::R8Srgb:
            isSrgb = true;
            #if BASISU_LIB_VERSION >= 150
            isHdr = false;
            #endif
            break;
        #if BASISU_LIB_VERSION >= 150
        case PixelFormat::R32F:
        case PixelFormat::RG32F:
        case PixelFormat::RGB32F:
        case PixelFormat::RGBA32F:
            isSrgb = false;
            isHdr = true;
            break;
        #endif
        default:
            Error{} << "Trade::BasisImageConverter::convertToData(): unsupported format" << pixelFormat;
            return {};
    }

    basisu::basis_compressor_params params;

    Math::Vector<dimensions, Int> mipMask{1};
    if(dimensions == 2) {
        if(ImageFlag2D(UnsignedShort(imageLevels[0].flags())) & ImageFlag2D::Array) {
            Error{} << "Trade::BasisImageConverter::convertToData(): 1D array images are not supported by Basis Universal";
            return {};
        }

        params.m_tex_type = basist::basis_texture_type::cBASISTexType2D;
    } else if(dimensions == 3) {
        const ImageFlags3D imageFlags = ImageFlag3D(UnsignedShort(imageLevels[0].flags()));

        /* Cube map and cube map array is the same thing, distinguishable only
           by number of layers */
        if(imageFlags & ImageFlag3D::CubeMap) {
            params.m_tex_type = basist::basis_texture_type::cBASISTexTypeCubemapArray;

        /* Encoding 3D images as KTX2 always produces 2D array images and mip
           levels in .basis files are inherently 2D images, so we always export
           2D array images. This affects the expected mip sizes and prevents
           a possible z-flip, so print a warning in case the input image wasn't
           marked as a 2D array. */
        } else {
            if(!(flags & ImageConverterFlag::Quiet) && !(imageFlags & ImageFlag3D::Array)) {
                Warning{} << "Trade::BasisImageConverter::convertToData(): exporting 3D image as a 2D array image";
            }

            params.m_tex_type = basist::basis_texture_type::cBASISTexType2DArray;
        }

        mipMask[dimensions - 1] = 0;
    } else CORRADE_INTERNAL_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */

    const auto baseSize = imageLevels.front().size();
    const UnsignedInt numMipmaps = Math::min<UnsignedInt>(imageLevels.size(), Math::log2((baseSize*mipMask).max()) + 1);

    if(imageLevels.size() > numMipmaps) {
        Error{} << "Trade::BasisImageConverter::convertToData(): there can be only" << numMipmaps <<
            "levels with base image size" << baseSize << "but got" << imageLevels.size();
        return {};
    }

    params.m_create_ktx2_file = isKtx;

    /* Options deduced from input data. Config values that are not emptied out
       override these below. */
    params.m_perceptual = isSrgb;
    params.m_mip_gen = numMipmaps == 1;
    params.m_mip_srgb = isSrgb;

    /* To retain sanity, keep this in the same order and grouping as in the
       conf file */
    #define PARAM_CONFIG(name, type) \
        if(!configuration.value(#name).empty()) params.m_##name = configuration.value<type>(#name)
    #define PARAM_CONFIG_FIX_NAME(name, type, fixed) \
        if(!configuration.value(fixed).empty()) params.m_##name = configuration.value<type>(fixed)
    /* Options */
    PARAM_CONFIG(quality_level, int);
    PARAM_CONFIG(perceptual, bool);
    PARAM_CONFIG(debug, bool);
    #if BASISU_LIB_VERSION >= 116
    PARAM_CONFIG_FIX_NAME(validate_etc1s, bool, "validate");
    #else
    PARAM_CONFIG(validate, bool);
    #endif
    PARAM_CONFIG(debug_images, bool);
    PARAM_CONFIG(compute_stats, bool);
    PARAM_CONFIG(compression_level, int);

    /* More options */
    PARAM_CONFIG(max_endpoint_clusters, int);
    PARAM_CONFIG(max_selector_clusters, int);
    PARAM_CONFIG(y_flip, bool);
    PARAM_CONFIG(check_for_alpha, bool);
    PARAM_CONFIG(force_alpha, bool);

    const auto swizzle = configuration.value<Containers::StringView>("swizzle");
    if(swizzle) {
        if(swizzle.size() != 4) {
            Error{} << "Trade::BasisImageConverter::convertToData(): invalid swizzle length, expected 4 but got" << swizzle.size();
            return {};
        }

        /** @todo clean up once StringView has findAnyNotOf() or some such */
        if(std::string{swizzle}.find_first_not_of("rgba") != std::string::npos) {
            Error{} << "Trade::BasisImageConverter::convertToData(): invalid characters in swizzle" << swizzle;
            return {};
        }

        for(std::size_t i = 0; i != 4; ++i) {
            switch(swizzle[i]) {
                case 'r': params.m_swizzle[i] = 0; break;
                case 'g': params.m_swizzle[i] = 1; break;
                case 'b': params.m_swizzle[i] = 2; break;
                case 'a': params.m_swizzle[i] = 3; break;
                default: CORRADE_INTERNAL_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
            }
        }
    }

    PARAM_CONFIG(renormalize, bool);
    PARAM_CONFIG(resample_width, int);
    PARAM_CONFIG(resample_height, int);
    PARAM_CONFIG(resample_factor, float);

    UnsignedInt threadCount = configuration.value<Int>("threads");
    if(threadCount == 0)
        threadCount = std::thread::hardware_concurrency();
    const bool multithreading = threadCount > 1;
    params.m_multithreading = multithreading;
    basisu::job_pool jpool{threadCount};
    params.m_pJob_pool = &jpool;

    PARAM_CONFIG(disable_hierarchical_endpoint_codebooks, bool);

    /* Mipmap generation options */
    PARAM_CONFIG(mip_gen, bool);
    PARAM_CONFIG(mip_srgb, bool);
    PARAM_CONFIG(mip_scale, float);
    PARAM_CONFIG(mip_filter, std::string);
    PARAM_CONFIG(mip_renormalize, bool);
    PARAM_CONFIG(mip_wrapping, bool);
    PARAM_CONFIG(mip_fast, bool);
    PARAM_CONFIG(mip_smallest_dimension, int);

    if(params.m_mip_gen && numMipmaps > 1 && !(flags & ImageConverterFlag::Quiet)) {
        Warning{} << "Trade::BasisImageConverter::convertToData(): found user-supplied mip levels, ignoring mip_gen config value";
        params.m_mip_gen = false;
    }

    /* Backend endpoint/selector RDO codec options */
    PARAM_CONFIG(no_selector_rdo, bool);
    PARAM_CONFIG_FIX_NAME(selector_rdo_thresh, float, "selector_rdo_threshold");
    PARAM_CONFIG(no_endpoint_rdo, bool);
    PARAM_CONFIG_FIX_NAME(endpoint_rdo_thresh, float, "endpoint_rdo_threshold");

    /* UASTC options */
    PARAM_CONFIG(uastc, bool);
    params.m_pack_uastc_flags = configuration.value<Int>("pack_uastc_level");
    PARAM_CONFIG(pack_uastc_flags, int);
    PARAM_CONFIG(rdo_uastc, bool);
    PARAM_CONFIG(rdo_uastc_quality_scalar, float);
    PARAM_CONFIG(rdo_uastc_dict_size, int);
    PARAM_CONFIG(rdo_uastc_max_smooth_block_error_scale, float);
    PARAM_CONFIG(rdo_uastc_smooth_block_max_std_dev, float);
    PARAM_CONFIG(rdo_uastc_max_allowed_rms_increase_ratio, float);
    PARAM_CONFIG_FIX_NAME(rdo_uastc_skip_block_rms_thresh, float, "rdo_uastc_skip_block_rms_threshold");
    PARAM_CONFIG(rdo_uastc_favor_simpler_modes_in_rdo_mode, bool);
    params.m_rdo_uastc_multithreading = multithreading;

    /* KTX2 options */
    params.m_ktx2_uastc_supercompression =
        configuration.value<bool>("ktx2_uastc_supercompression") ? basist::KTX2_SS_ZSTANDARD : basist::KTX2_SS_NONE;
    PARAM_CONFIG(ktx2_zstd_supercompression_level, int);
    params.m_ktx2_srgb_transfer_func = params.m_perceptual;

    /* y_flip sets a flag in Basis files, but not in KTX2 files:
       https://github.com/BinomialLLC/basis_universal/issues/258
       Manually specify the orientation in the key/value data:
       https://www.khronos.org/registry/KTX/specs/2.0/ktxspec_v2.html#_ktxorientation
       Output images are always 2D or 2D arrays. */
    constexpr char OrientationKey[] = "KTXorientation";
    char orientationValue[] = "rd";
    if(params.m_y_flip)
        orientationValue[1] = 'u';
    basist::ktx2_transcoder::key_value& keyValue = *params.m_ktx2_key_values.enlarge(1);
    keyValue.m_key.append(reinterpret_cast<const uint8_t*>(OrientationKey), sizeof(OrientationKey));
    keyValue.m_key.append(reinterpret_cast<const uint8_t*>(orientationValue), sizeof(orientationValue));

    /* OpenCL */
    #if BASISU_LIB_VERSION >= 116
    PARAM_CONFIG(use_opencl, bool);

    if(params.m_use_opencl && !basisu::opencl_is_available()) {
        Warning{} << "Trade::BasisImageConverter::convertToData(): OpenCL not supported, falling back to CPU encoding";
        params.m_use_opencl = false;
    }
    #endif

    /* HDR */
    #if BASISU_LIB_VERSION >= 150
    params.m_hdr = isHdr;

    params.m_uastc_hdr_options.init();
    params.m_uastc_hdr_options.set_quality_level(configuration.value<int>("hdr_quality_level"));
    PARAM_CONFIG(hdr_favor_astc, bool);
    #endif

    /* Set various fields in the Basis file header */
    PARAM_CONFIG(userdata0, int);
    PARAM_CONFIG(userdata1, int);
    #undef PARAM_CONFIG
    #undef PARAM_CONFIG_FIX_NAME

    /* Don't spam stdout with debug info by default. Basis error output is
       unaffected by this. Unfortunately, there's no way to redirect the output
       to Debug. */
    params.m_status_output = flags >= ImageConverterFlag::Verbose;

    /* If these are enabled, the library reads BMPs/JPGs/PNGs/TGAs from the
       filesystem and then writes basis files there also. DO NOT WANT. */
    params.m_read_source_images = false;
    #if BASISU_LIB_VERSION >= 150
    params.m_write_output_basis_or_ktx2_files = false;
    #else
    params.m_write_output_basis_files = false;
    #endif

    /* One image per slice. The base mip is in m_source_images, mip 1 and
       higher go into m_source_mipmap_images. */
    const UnsignedInt numImages = Vector3i::pad(baseSize, 1).z();
    #if BASISU_LIB_VERSION >= 150
    if(isHdr) {
        params.m_source_images_hdr.resize(numImages);
        if(numMipmaps > 1) {
            params.m_source_mipmap_images_hdr.resize(numImages);
            for(auto& slice: params.m_source_mipmap_images_hdr)
                slice.resize(numMipmaps - 1);
        }
    } else
    #endif
    {
        params.m_source_images.resize(numImages);
        if(numMipmaps > 1) {
            params.m_source_mipmap_images.resize(numImages);
            for(auto& slice: params.m_source_mipmap_images)
                slice.resize(numMipmaps - 1);
        }
    }

    for(UnsignedInt level = 0; level != numMipmaps; ++level) {
        const auto mipSize = Math::max(baseSize >> level, 1)*mipMask + baseSize*(Math::Vector<dimensions, Int>{1} - mipMask);
        const auto& image = imageLevels[level];
        if(image.size() != mipSize) {
            Error{} << "Trade::BasisImageConverter::convertToData(): expected "
                "size" << mipSize << "for level" << level << "but got" << image.size();
            return {};
        }

        /* Always get a 3D view to generalize indexing for 2D and 3D images */
        const ImageView3D image3D{image};

        for(UnsignedInt slice = 0; slice != numImages; ++slice) {
            /* Copy image data into the basis image. There is no way to construct a
               basis image from existing data as it is based on basisu::vector,
               moreover we need to tightly pack it and flip Y. */
            /* basisu doesn't apply m_y_flip to user-supplied mipmaps, so only
               do this for the base image:
               https://github.com/BinomialLLC/basis_universal/issues/257 */
            const bool yFlip = !params.m_y_flip || level == 0;
            const bool hasCustomSwizzle = !swizzle.isEmpty();
            #if BASISU_LIB_VERSION >= 150
            if(isHdr) {
                basisu::imagef& basisImage = level > 0 ? params.m_source_mipmap_images_hdr[slice][level - 1] : params.m_source_images_hdr[slice];
                basisImage.resize(mipSize[0], mipSize[1]);
                auto dst = Containers::arrayCast<Color4>(Containers::StridedArrayView2D<basisu::vec4F>({basisImage.get_ptr(), basisImage.get_total_pixels()}, {std::size_t(mipSize[1]), std::size_t(mipSize[0])}));

                copySlice(image3D, slice, dst, yFlip, hasCustomSwizzle);
            } else
            #endif
            {
                basisu::image& basisImage = level > 0 ? params.m_source_mipmap_images[slice][level - 1] : params.m_source_images[slice];
                basisImage.resize(mipSize[0], mipSize[1]);
                auto dst = Containers::arrayCast<Color4ub>(Containers::StridedArrayView2D<basisu::color_rgba>({basisImage.get_ptr(), basisImage.get_total_pixels()}, {std::size_t(mipSize[1]), std::size_t(mipSize[0])}));

                copySlice(image3D, slice, dst, yFlip, hasCustomSwizzle);
            }
        }
    }

    basisu::basis_compressor basis;
    basis.init(params);

    const basisu::basis_compressor::error_code errorCode = basis.process();
    if(errorCode != basisu::basis_compressor::error_code::cECSuccess) switch(errorCode) {
        case basisu::basis_compressor::error_code::cECFailedReadingSourceImages:
            /* Emitted e.g. when source image is 0-size */
            Error{} << "Trade::BasisImageConverter::convertToData(): source image is invalid";
            return {};
        case basisu::basis_compressor::error_code::cECFailedValidating:
            /* process() will have printed additional error information to stderr */
            Error{} << "Trade::BasisImageConverter::convertToData(): type constraint validation failed";
            return {};
        case basisu::basis_compressor::error_code::cECFailedEncodeUASTC:
            Error{} << "Trade::BasisImageConverter::convertToData(): UASTC encoding failed";
            return {};
        case basisu::basis_compressor::error_code::cECFailedFrontEnd:
            /* process() will have printed additional error information to stderr */
            Error{} << "Trade::BasisImageConverter::convertToData(): frontend processing failed";
            return {};
        case basisu::basis_compressor::error_code::cECFailedFontendExtract:
            Error{} << "Trade::BasisImageConverter::convertToData(): frontend extraction failed";
            return {};
        case basisu::basis_compressor::error_code::cECFailedBackend:
            Error{} << "Trade::BasisImageConverter::convertToData(): encoding failed";
            return {};
        case basisu::basis_compressor::error_code::cECFailedCreateBasisFile:
            /* process() will have printed additional error information to stderr */
            Error{} << "Trade::BasisImageConverter::convertToData(): assembling basis file data or transcoding failed";
            return {};
        case basisu::basis_compressor::error_code::cECFailedUASTCRDOPostProcess:
            Error{} << "Trade::BasisImageConverter::convertToData(): UASTC RDO postprocessing failed";
            return {};
        case basisu::basis_compressor::error_code::cECFailedCreateKTX2File:
            Error{} << "Trade::BasisImageConverter::convertToData(): assembling KTX2 file failed";
            return {};

        /* LCOV_EXCL_START */
        #if BASISU_LIB_VERSION >= 116
        case basisu::basis_compressor::error_code::cECFailedInitializing:
            /* This error is returned by the parallel compression API if
               basis_compressor::init() returns false */
        #endif
        case basisu::basis_compressor::error_code::cECFailedWritingOutput:
            /* We do not write any files, just data */
        default:
            CORRADE_INTERNAL_ASSERT_UNREACHABLE();
        /* LCOV_EXCL_STOP */
    }

    #if BASISU_LIB_VERSION >= 116
    /* If OpenCL fails at any stage of encoding, Basis falls back to encoding
       on the CPU. It also spams stderr but printing to Warning is still useful
       for anyone parsing or redirecting it. */
    if(params.m_use_opencl && basis.get_opencl_failed()) {
        Warning{} << "Trade::BasisImageConverter::convertToData(): OpenCL encoding failed, fell back to CPU encoding";
    }
    #endif

    const basisu::uint8_vec& out = params.m_create_ktx2_file ? basis.get_output_ktx2_file() : basis.get_output_basis_file();

    Containers::Array<char> fileData{InPlaceInit, Containers::arrayCast<const char>(Containers::arrayView(out.data(), out.size()))};

    /* UASTC output in a Basis container has the sRGB flag set always, patch it
       away if the data is not sRGB. Doesn't happen with ETC1S and doesn't
       happen with the KTX container either. */
    if(!params.m_create_ktx2_file) {
        auto& header = *reinterpret_cast<basist::basis_file_header*>(fileData.data());
        if(!params.m_perceptual && (header.m_flags & basist::basis_header_flags::cBASISHeaderFlagSRGB)) {
            if(flags & ImageConverterFlag::Verbose) Debug{} <<
                "Trade::BasisImageConverter::convertToData(): patching away an incorrect sRGB flag in the output Basis file";
            header.m_flags = header.m_flags & ~basist::basis_header_flags::cBASISHeaderFlagSRGB;
            header.m_header_crc16 = basist::crc16(&header.m_data_size, sizeof(basist::basis_file_header) - offsetof(basist::basis_file_header, m_data_size), 0);
        }
    }

    /* GCC 4.8 needs extra help here */
    return Containers::optional(Utility::move(fileData));
}

}

void BasisImageConverter::initialize() {
    #if BASISU_LIB_VERSION >= 116
    basisu::basisu_encoder_init(true);
    #else
    basisu::basisu_encoder_init();
    #endif
}

void BasisImageConverter::finalize() {
    #if BASISU_LIB_VERSION >= 116
    basisu::basisu_encoder_deinit();
    #endif
}

#ifdef MAGNUM_BUILD_DEPRECATED /* LCOV_EXCL_START */
BasisImageConverter::BasisImageConverter(Format format): _format{format} {
    /* Passing an invalid Format enum is user error, we'll assert on that in
       the convertToData() function */
}
#endif /* LCOV_EXCL_STOP */

BasisImageConverter::BasisImageConverter(PluginManager::AbstractManager& manager, const Containers::StringView& plugin): AbstractImageConverter{manager, plugin} {
    if(plugin == "BasisKtxImageConverter"_s)
        _format = Format::Ktx;
    else
        _format = {}; /* Overridable by openFile() */
}

ImageConverterFeatures BasisImageConverter::doFeatures() const {
    return ImageConverterFeature::Convert2DToData|
           ImageConverterFeature::Convert3DToData|
           ImageConverterFeature::Levels;
}

Containers::String BasisImageConverter::doExtension() const {
    if(_format == Format::Ktx)
        return "ktx2"_s;
    return "basis"_s;
}

Containers::String BasisImageConverter::doMimeType() const {
    if(_format == Format::Ktx)
        return "image/ktx2"_s;
    return {};
}

Containers::Optional<Containers::Array<char>> BasisImageConverter::doConvertToData(Containers::ArrayView<const ImageView2D> imageLevels) {
    /* Use KTX if explicitly set, fall back to Basis if explicitly set or not
       specified at all */
    return convertLevelsToData(imageLevels, configuration(), flags(), _format == Format::Ktx);
}

Containers::Optional<Containers::Array<char>> BasisImageConverter::doConvertToData(Containers::ArrayView<const ImageView3D> imageLevels) {
    /* Use KTX if explicitly set, fall back to Basis if explicitly set or not
       specified at all */
    return convertLevelsToData(imageLevels, configuration(), flags(), _format == Format::Ktx);
}

template<UnsignedInt dimensions> bool BasisImageConverter::convertLevelsToFile(const Containers::ArrayView<const BasicImageView<dimensions>> imageLevels, const Containers::StringView filename) {
    const Containers::String normalizedExtension = Utility::String::lowercase(Utility::Path::splitExtension(filename).second());

    /* Save the previous format to restore it back after, detect the format
       from extension if it's not supplied explicitly */
    const Format previousFormat = _format;
    if(_format == Format{}) {
        if(normalizedExtension == ".ktx2"_s)
            _format = Format::Ktx;
        else
            _format = Format::Basis;
    }

    /* Delegate to the base implementation which calls doConvertToData() */
    const bool out = AbstractImageConverter::doConvertToFile(imageLevels, filename);

    /* Restore the previous format and return the result */
    _format = previousFormat;
    return out;
}

bool BasisImageConverter::doConvertToFile(Containers::ArrayView<const ImageView2D> imageLevels, const Containers::StringView filename) {
    return convertLevelsToFile(imageLevels, filename);
}

bool BasisImageConverter::doConvertToFile(Containers::ArrayView<const ImageView3D> imageLevels, const Containers::StringView filename) {
    return convertLevelsToFile(imageLevels, filename);
}

}}

CORRADE_PLUGIN_REGISTER(BasisImageConverter, Magnum::Trade::BasisImageConverter,
    MAGNUM_TRADE_ABSTRACTIMAGECONVERTER_PLUGIN_INTERFACE)
