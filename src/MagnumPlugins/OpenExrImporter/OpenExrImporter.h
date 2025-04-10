#ifndef Magnum_Trade_OpenExrImporter_h
#define Magnum_Trade_OpenExrImporter_h
/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
                2020, 2021, 2022, 2023, 2024, 2025
              Vladimír Vondruš <mosra@centrum.cz>

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

/** @file
 * @brief Class @ref Magnum::Trade::OpenExrImporter
 * @m_since_latest_{plugins}
 */

#include <Corrade/Containers/Pointer.h>
#include <Magnum/Trade/AbstractImporter.h>

#include "MagnumPlugins/OpenExrImporter/configure.h"

#ifndef DOXYGEN_GENERATING_OUTPUT
#ifndef MAGNUM_OPENEXRIMPORTER_BUILD_STATIC
    #ifdef OpenExrImporter_EXPORTS
        #define MAGNUM_OPENEXRIMPORTER_EXPORT CORRADE_VISIBILITY_EXPORT
    #else
        #define MAGNUM_OPENEXRIMPORTER_EXPORT CORRADE_VISIBILITY_IMPORT
    #endif
#else
    #define MAGNUM_OPENEXRIMPORTER_EXPORT CORRADE_VISIBILITY_STATIC
#endif
#define MAGNUM_OPENEXRIMPORTER_LOCAL CORRADE_VISIBILITY_LOCAL
#else
#define MAGNUM_OPENEXRIMPORTER_EXPORT
#define MAGNUM_OPENEXRIMPORTER_LOCAL
#endif

namespace Magnum { namespace Trade {

/**
@brief OpenEXR importer plugin
@m_since_latest_{plugins}

Imports OpenEXR (`*.exr`) images using the [OpenEXR](https://www.openexr.com)
library. You can use @ref OpenExrImageConverter to encode images into this
format.

@m_class{m-block m-success}

@thirdparty This plugin makes use of the [OpenEXR](https://www.openexr.com)
    library, licensed under @m_class{m-label m-success} **BSD 3-clause**
    ([license text](https://github.com/AcademySoftwareFoundation/openexr/blob/master/LICENSE.md),
    [choosealicense.com](https://choosealicense.com/licenses/bsd-3-clause/)).
    It requires attribution for public use.

@section Trade-OpenExrImporter-usage Usage

@m_class{m-note m-success}

@par
    This class is a plugin that's meant to be dynamically loaded and used
    through the base @ref AbstractImporter interface. See its documentation for
    introduction and usage examples.

This plugin depends on the @ref Trade library and is built if
`MAGNUM_WITH_OPENEXRIMPORTER` is enabled when building Magnum Plugins. To use
as a dynamic plugin, load @cpp "OpenExrImporter" @ce via
@ref Corrade::PluginManager::Manager.

Additionally, if you're using Magnum as a CMake subproject, bundle the
[magnum-plugins](https://github.com/mosra/magnum-plugins) and
[openexr](https://github.com/AcademySoftwareFoundation/openexr) repositories
(pin OpenEXR at `v3.0.1` at least) and do the following. OpenEXR depends on
libdeflate (versions before 3.2.0 on zlib) and Imath, however it's capable of
fetching those dependencies on its own so bundling them isn't necessary. If you
want to use system-installed OpenEXR, omit the  first part and point
`CMAKE_PREFIX_PATH` to its installation dir if necessary.

@code{.cmake}
# Disable unneeded functionality
set(PYILMBASE_ENABLE OFF CACHE BOOL "" FORCE)
set(IMATH_INSTALL_PKG_CONFIG OFF CACHE BOOL "" FORCE)
set(IMATH_INSTALL_SYM_LINK OFF CACHE BOOL "" FORCE)
set(OPENEXR_INSTALL OFF CACHE BOOL "" FORCE)
set(OPENEXR_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
set(OPENEXR_INSTALL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(OPENEXR_INSTALL_PKG_CONFIG OFF CACHE BOOL "" FORCE)
set(OPENEXR_INSTALL_TOOLS OFF CACHE BOOL "" FORCE)
# OPENEXR_BUILD_TOOLS used by 3.1+, OPENEXR_BUILD_UTILS by versions before
set(OPENEXR_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(OPENEXR_BUILD_UTILS OFF CACHE BOOL "" FORCE)
# Otherwise OpenEXR uses C++14, and before OpenEXR 3.0.2 also forces C++14 on
# all libraries that link to it.
set(OPENEXR_CXX_STANDARD 11 CACHE STRING "" FORCE)
# OpenEXR implicitly bundles Imath. However, without this only the first CMake
# run will pass and subsequent runs will fail.
set(CMAKE_DISABLE_FIND_PACKAGE_Imath ON)
# These variables may be used by other projects, so ensure they're reset back
# to their original values after. OpenEXR forces CMAKE_DEBUG_POSTFIX to _d,
# which isn't desired outside of that library.
set(_PREV_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
set(_PREV_BUILD_TESTING ${BUILD_TESTING})
set(BUILD_SHARED_LIBS OFF)
set(BUILD_TESTING OFF)
set(CMAKE_DEBUG_POSTFIX "" CACHE STRING "" FORCE)
add_subdirectory(openexr EXCLUDE_FROM_ALL)
set(BUILD_SHARED_LIBS ${_PREV_BUILD_SHARED_LIBS})
set(BUILD_TESTING ${_PREV_BUILD_TESTING})
unset(CMAKE_DEBUG_POSTFIX CACHE)

set(MAGNUM_WITH_OPENEXRIMPORTER ON CACHE BOOL "" FORCE)
add_subdirectory(magnum-plugins EXCLUDE_FROM_ALL)

# So the dynamically loaded plugin gets built implicitly
add_dependencies(your-app MagnumPlugins::OpenExrImporter)
@endcode

To use as a static plugin or as a dependency of another plugin with CMake,
request the `OpenExrImporter` component of the `MagnumPlugins` package in CMake
and link to the `MagnumPlugins::OpenExrImporter` target:

@code{.cmake}
find_package(MagnumPlugins REQUIRED OpenExrImporter)

# ...
target_link_libraries(your-app PRIVATE MagnumPlugins::OpenExrImporter)
@endcode

See @ref building-plugins, @ref cmake-plugins, @ref plugins and
@ref file-formats for more information.

@subsection Trade-OpenExrImporter-usage-emscripten Building for Emscripten

Since version 3.2, the library can compile for Emscripten as well with the
following additionally set before @cpp add_subdirectory(openexr) @ce. OpenEXR
uses exceptions, so the plugin internally also enables them for itself and
all code that links to it.

@code{.cmake}
# Force bundled Imath and libdeflate so Emscripten doesn't try to find them
# among native libraries
set(OPENEXR_FORCE_INTERNAL_IMATH ON CACHE BOOL "" FORCE)
set(OPENEXR_FORCE_INTERNAL_DEFLATE ON CACHE BOOL "" FORCE)
# May want to keep enabled if you already use pthreads in Emscripten
set(OPENEXR_ENABLE_THREADING OFF CACHE BOOL "" FORCE)
# The table is literally all half-floats, so 256 kB of data
set(IMATH_HALF_USE_LOOKUP_TABLE OFF CACHE BOOL "" FORCE)
@endcode

In addition to the above-disabled half-float conversion lookup table, there's
1.4 MB + 256 kB of tables used for DWA compression and decompression, and
another 128 + 128 kB of tables used for B44 (de)compression, of which none can
be easily controlled with a CMake option. If you know you won't need DWA or B44
compression with @ref OpenExrImageConverter, you can replace all tables except
the first two `dwaCompressorNoOp` and `dwaCompressorToLinear` in
`src/lib/OpenEXR/dwaLookups.h` with the following (or replace
`dwaCompressorNoOp` and `dwaCompressorToLinear` with a null pointer too if you
know you won't need DWA loading either):

@code{.c}
static const unsigned short* dwaCompressorToNonlinear = NULL;
static const unsigned int* closestDataOffset = NULL;
static const unsigned short* closestData = NULL;
@endcode

And similarly replace the `expTable` in `src/lib/OpenEXR/b44ExpLogTable` (or
also the `logTable` if you know you won't need B44 loading either):

@code{.c}
const unsigned short* expTable = NULL;
@endcode

Together with the disabled half-float conversion table this reduces the
@ref OpenExrImporter plugin size from 2.5 MB to ~800 kB. If
@ref OpenExrImageConverter is built against such patched code, attempting to
compress DWA or B44 images with it will cause a crash, but other compression
schemes will continue to work.

@section Trade-OpenExrImporter-behavior Behavior and limitations

Supports single-part scanline and tile-based 2D and cubemap images with
optional mip levels and half-float, float and unsigned integer channels.

The plugin recognizes @ref ImporterFlag::Quiet, which will cause all import
warnings to be suppressed.

@subsection Trade-OpenExrImporter-behavior-channel-mapping Channel mapping

Images containing `R`, `G`, `B` or `A` channels are imported as
@ref PixelFormat::R16F / @relativeref{PixelFormat,RG16F} /
@relativeref{PixelFormat,RGB16F} / @relativeref{PixelFormat,RGBA16F},
@ref PixelFormat::R32F / @relativeref{PixelFormat,RG32F} /
@relativeref{PixelFormat,RGB32F} / @relativeref{PixelFormat,RGBA32F} or
@ref PixelFormat::R32UI / @relativeref{PixelFormat,RG32UI} /
@relativeref{PixelFormat,RGB32UI} / @relativeref{PixelFormat,RGBA32UI}, all
channels are expected to have the same type.

If neither of the color channels is present and a a `Z` channel is present
instead, the image is imported as @ref PixelFormat::Depth32F, expecting the
channel to be of a float type.

If a file doesn't match ither of the above, custom channel mapping can be
supplied @ref Trade-OpenExrImporter-configuration "in the configuration".

@subsection Trade-OpenExrImporter-behavior-data-layout Data alignment, display and data windows

Image rows are always aligned to four bytes.

OpenEXR image metadata contain a display and data window, which can be used for
annotating crop borders or specifying that the data is just a small portion of
a larger image. The importer ignores the display window and imports everything
that's inside the data window, without offseting it in any way.

@subsection Trade-OpenExrImporter-behavior-multilayer-multipart Multilayer and multipart images, deep images

Images with custom layers (for example with separate channels for a left and
right view) can be imported by specifying the @cb{.ini} layer @ce
@ref Trade-OpenExrImporter-configuration "configuration option".

Multipart and deep images are not supported right now --- only the first part /
sample gets imported.

@subsection Trade-OpenExrImporter-behavior-multilevel Multilevel images

Tiled OpenEXR images with multiple mip levels are imported with the largest
level first, with the size divided by 2 in each following level. For
non-power-of-two sizes, the file format supports sizes rounded either up or
down.

The file format doesn't have a builtin way to describe level count for
incomplete mip chains, instead the file can have some tiles missing. This is
detected on import and empty levels at the end are not counted into the
reported level count, with a message printed if @ref ImporterFlag::Verbose is
enabled.

[Ripmap](https://en.wikipedia.org/wiki/Anisotropic_filtering#An_improvement_on_isotropic_MIP_mapping)
files are imported as a single-level image right now.

@subsection Trade-OpenExrImporter-behavior-cubemap Cube and lat/lon environment maps

A lat/long environment map is imported as a 2D image without any indication of
its type.

OpenEXR stores cubemaps as 2D images with the height being six times of its
width. For convenience these are imported as a 3D image with each face being
one slice, in order +X, -X, +Y, -Y, +Z and -Z, and @ref ImageFlag3D::CubeMap
set. Cube maps can have mip levels as well, however because OpenEXR treats them
internally as 2D images, it includes also levels with @cpp height < 6 @ce.
Because those would result in a 3D image with zero height, the smallest levels
are ignored, with a message printed if @ref ImporterFlag::Verbose is enabled.

@section Trade-OpenExrImporter-configuration Plugin-specific configuration

It's possible to tune various options mainly for channel mapping through
@ref configuration(). See below for all options and their default values:

@snippet MagnumPlugins/OpenExrImporter/OpenExrImporter.conf configuration_

See @ref plugins-configuration for more information and an example showing how
to edit the configuration values.

@subsection Trade-OpenExrImporter-configuration-threads Enabling multithreading

On Linux it may happen that setting the @cb{.ini} threads @ce option to
something else than `1` will cause @ref std::system_error to be thrown (or,
worst case, crashing with a null function pointer call on some systems).
There's no way to solve this from within the dynamically loaded module itself,
* *the application* has to be linked to `pthread` instead. With CMake it can be
done like this:

@code{.cmake}
find_package(Threads REQUIRED)
target_link_libraries(your-application PRIVATE Threads::Threads)
@endcode

*/
class MAGNUM_OPENEXRIMPORTER_EXPORT OpenExrImporter: public AbstractImporter {
    public:
        /** @brief Plugin manager constructor */
        explicit OpenExrImporter(PluginManager::AbstractManager& manager, const Containers::StringView& plugin);

        ~OpenExrImporter();

    private:
        MAGNUM_OPENEXRIMPORTER_LOCAL ImporterFeatures doFeatures() const override;
        MAGNUM_OPENEXRIMPORTER_LOCAL bool doIsOpened() const override;
        MAGNUM_OPENEXRIMPORTER_LOCAL void doClose() override;
        MAGNUM_OPENEXRIMPORTER_LOCAL void doOpenData(Containers::Array<char>&& data, DataFlags dataFlags) override;

        MAGNUM_OPENEXRIMPORTER_LOCAL UnsignedInt doImage2DCount() const override;
        MAGNUM_OPENEXRIMPORTER_LOCAL UnsignedInt doImage2DLevelCount(UnsignedInt id) override;
        MAGNUM_OPENEXRIMPORTER_LOCAL Containers::Optional<ImageData2D> doImage2D(UnsignedInt id, UnsignedInt level) override;
        MAGNUM_OPENEXRIMPORTER_LOCAL UnsignedInt doImage3DCount() const override;
        MAGNUM_OPENEXRIMPORTER_LOCAL UnsignedInt doImage3DLevelCount(UnsignedInt id) override;
        MAGNUM_OPENEXRIMPORTER_LOCAL Containers::Optional<ImageData3D> doImage3D(UnsignedInt id, UnsignedInt level) override;

        struct State;
        Containers::Pointer<State> _state;
};

}}

#endif
