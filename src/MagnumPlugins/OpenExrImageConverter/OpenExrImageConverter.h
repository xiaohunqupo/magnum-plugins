#ifndef Magnum_Trade_OpenExrImageConverter_h
#define Magnum_Trade_OpenExrImageConverter_h
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
 * @brief Class @ref Magnum::Trade::OpenExrImageConverter
 * @m_since_latest_{plugins}
 */

#include <Magnum/Trade/AbstractImageConverter.h>

#include "MagnumPlugins/OpenExrImageConverter/configure.h"

#ifndef DOXYGEN_GENERATING_OUTPUT
#ifndef MAGNUM_OPENEXRIMAGECONVERTER_BUILD_STATIC
    #ifdef OpenExrImageConverter_EXPORTS
        #define MAGNUM_OPENEXRIMAGECONVERTER_EXPORT CORRADE_VISIBILITY_EXPORT
    #else
        #define MAGNUM_OPENEXRIMAGECONVERTER_EXPORT CORRADE_VISIBILITY_IMPORT
    #endif
#else
    #define MAGNUM_OPENEXRIMAGECONVERTER_EXPORT CORRADE_VISIBILITY_STATIC
#endif
#define MAGNUM_OPENEXRIMAGECONVERTER_LOCAL CORRADE_VISIBILITY_LOCAL
#else
#define MAGNUM_OPENEXRIMAGECONVERTER_EXPORT
#define MAGNUM_OPENEXRIMAGECONVERTER_LOCAL
#endif

namespace Magnum { namespace Trade {

/**
@brief OpenEXR image converter plugin
@m_since_latest_{plugins}

Creates OpenEXR (`*.exr`) files using the [OpenEXR](https://www.openexr.com)
library. You can use @ref OpenExrImporter to import images in this format.

@m_class{m-block m-success}

@thirdparty This plugin makes use of the [OpenEXR](https://www.openexr.com)
    library, licensed under @m_class{m-label m-success} **BSD 3-clause**
    ([license text](https://github.com/AcademySoftwareFoundation/openexr/blob/master/LICENSE.md),
    [choosealicense.com](https://choosealicense.com/licenses/bsd-3-clause/)).
    It requires attribution for public use.

@section Trade-OpenExrImageConverter-usage Usage

@m_class{m-note m-success}

@par
    This class is a plugin that's meant to be dynamically loaded and used
    via the base @ref AbstractImageConverter interface. See its documentation
    for introduction and usage examples.

This plugin depends on the @ref Trade library and is built if
`MAGNUM_WITH_OPENEXRIMAGECONVERTER` is enabled when building Magnum Plugins. To
use as a dynamic plugin, load @cpp "OpenExrImageConverter" @ce via
@ref Corrade::PluginManager::Manager.

Additionally, if you're using Magnum as a CMake subproject, bundle the
[magnum-plugins](https://github.com/mosra/magnum-plugins) and
[openexr](https://github.com/AcademySoftwareFoundation/openexr) repositories
(pin OpenEXR at `v3.0.1` at least) and do the same as shown in the
@ref Trade-OpenExrImporter-usage "OpenExrImporter usage docs", with the end
being the following instead. If you want to use system-installed OpenEXR,
omit the  first part and point `CMAKE_PREFIX_PATH` to its installation dir if
necessary.

@code{.cmake}
# (Same setup and add_subdirectory(openexr) as with OpenExrImporter)

set(MAGNUM_WITH_OPENEXRIMAGECONVERTER ON CACHE BOOL "" FORCE)
add_subdirectory(magnum-plugins EXCLUDE_FROM_ALL)

# So the dynamically loaded plugin gets built implicitly
add_dependencies(your-app MagnumPlugins::OpenExrImageConverter)
@endcode

To use as a static plugin or as a dependency of another plugin with CMake,
request the `OpenExrImageConverter` component of the `MagnumPlugins` package
and link to the `MagnumPlugins::OpenExrImageConverter` target:

@code{.cmake}
find_package(MagnumPlugins REQUIRED OpenExrImageConverter)

# ...
target_link_libraries(your-app PRIVATE MagnumPlugins::OpenExrImageConverter)
@endcode

See @ref building-plugins, @ref cmake-plugins, @ref plugins and
@ref file-formats for more information.

@section Trade-OpenExrImageConverter-behavior Behavior and limitations

Accepts 2D and cubemap images with optional mip levels in
@ref PixelFormat::R16F / @relativeref{PixelFormat,RG16F} /
@relativeref{PixelFormat,RGB16F} / @relativeref{PixelFormat,RGBA16F},
@ref PixelFormat::R32F / @relativeref{PixelFormat,RG32F} /
@relativeref{PixelFormat,RGB32F} / @relativeref{PixelFormat,RGBA32F} or
@ref PixelFormat::R32UI / @relativeref{PixelFormat,RG32UI} /
@relativeref{PixelFormat,RGB32UI} / @relativeref{PixelFormat,RGBA32UI} and
@ref PixelFormat::Depth32F.

As OpenEXR doesn't have a registered MIME type, @ref mimeType() returns
@cpp "image/x-exr" @ce.

The plugin recognizes @ref ImageConverterFlag::Quiet, which will cause all
conversion warnings to be suppressed.

@subsection Trade-OpenExrImageConverter-behavior-channel-mapping Channel mapping

Images with @ref PixelFormat::R16F / @relativeref{PixelFormat,RG16F} /
@relativeref{PixelFormat,RGB16F} / @relativeref{PixelFormat,RGBA16F},
@ref PixelFormat::R32F / @relativeref{PixelFormat,RG32F} /
@relativeref{PixelFormat,RGB32F} / @relativeref{PixelFormat,RGBA32F} or
@ref PixelFormat::R32UI / @relativeref{PixelFormat,RG32UI} /
@relativeref{PixelFormat,RGB32UI} / @relativeref{PixelFormat,RGBA32UI} are
implicitly written to channels named `R`, `G`, `B` and `A`; images with
@ref PixelFormat::Depth32F are implicitly written to a `Z` channel.

If the default behavior is not sufficient, custom channel mapping can be
supplied @ref Trade-OpenExrImageConverter-configuration "in the configuration".

@subsection Trade-OpenExrImageConverter-behavior-multilayer-multipart Multilayer and multipart images, deep images

Channels can be prefixed with a custom layer name by specifying the
@cb{.ini} layer @ce @ref Trade-OpenExrImporter-configuration "configuration option".
Combining multiple layers into a single image isn't supported right now,
writing multipart files or deep images is not supported either.

@subsection Trade-OpenExrImageConverter-behavior-cubemap Cube and lat/lon environment maps

A 2D image can be annotated as being a lat/lon environment map by setting
@cb{.ini} envmap=latlon @ce @ref Trade-OpenExrImporter-configuration "in the configuration". This requires it to have the width twice of the height.

A cube map image can be saved from an @ref ImageView3D where each slice is one
face in order +X, -X, +Y, -Y, +Z and -Z if you set @cb{.ini} envmap=cube @ce.
In this case, the image is expected to have six rectangular faces.

@subsection Trade-OpenExrImageConverter-array-3d Array and 3D images

Apart from cube maps, saving of arbitrary 3D and 2D array (or "deep") images
isn't implemented right now.

The OpenEXR file format doesn't have a way to distinguish between 2D and 1D
array images. If an image has @ref ImageFlag2D::Array set, a warning is printed
and the file is saved as a regular 2D image.

@subsection Trade-OpenExrImageConverter-behavior-multilevel Multilevel images

Both 2D and cube map images can be saved with multiple levels by using the list
variants of @ref convertToFile() / @ref convertToData(). Largest level is
expected to be first, with each following level having width and height divided
by two, rounded down. Cube map images additionally have the restrictions
specified above. OpenEXR has no builtin concept of an incomplete mip chain,
unspecified levels at the end result in a file with missing tiles. This *may*
cause problems with 3rd party tools, however the @ref OpenExrImporter detects
such case and reports the file as having less levels.

Multilevel images result in a tiled OpenEXR file, with a tile size taken from
the @cb{.ini} tileSize @ce @ref Trade-OpenExrImporter-configuration "configuration option".
Single-level images are implicitly written as scanline files, you can override
that with the @cpp forceTiledOutput @ce option.

@section Trade-OpenExrImageConverter-configuration Plugin-specific configuration

It's possible to tune various options mainly for channel mapping through
@ref configuration(). See below for all options and their default values:

@snippet MagnumPlugins/OpenExrImageConverter/OpenExrImageConverter.conf configuration_

See @ref plugins-configuration for more information and an example showing how
to edit the configuration values.

@subsection Trade-OpenExrImageConverter-configuration-threads Enabling multithreading

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
class MAGNUM_OPENEXRIMAGECONVERTER_EXPORT OpenExrImageConverter: public AbstractImageConverter {
    public:
        /** @brief Plugin manager constructor */
        explicit OpenExrImageConverter(PluginManager::AbstractManager& manager, const Containers::StringView& plugin);

    private:
        MAGNUM_OPENEXRIMAGECONVERTER_LOCAL ImageConverterFeatures doFeatures() const override;
        MAGNUM_OPENEXRIMAGECONVERTER_LOCAL Containers::String doExtension() const override;
        MAGNUM_OPENEXRIMAGECONVERTER_LOCAL Containers::String doMimeType() const override;

        MAGNUM_OPENEXRIMAGECONVERTER_LOCAL Containers::Optional<Containers::Array<char>> doConvertToData(Containers::ArrayView<const ImageView2D> imageLevels) override;
        MAGNUM_OPENEXRIMAGECONVERTER_LOCAL Containers::Optional<Containers::Array<char>> doConvertToData(Containers::ArrayView<const ImageView3D> imageLevels) override;
};

}}

#endif
