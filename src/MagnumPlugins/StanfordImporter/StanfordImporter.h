#ifndef Magnum_Trade_StanfordImporter_h
#define Magnum_Trade_StanfordImporter_h
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
 * @brief Class @ref Magnum::Trade::StanfordImporter
 */

#include <Corrade/Containers/Pointer.h>
#include <Magnum/Trade/AbstractImporter.h>

#include "MagnumPlugins/StanfordImporter/configure.h"

#ifndef DOXYGEN_GENERATING_OUTPUT
#ifndef MAGNUM_STANFORDIMPORTER_BUILD_STATIC
    #ifdef StanfordImporter_EXPORTS
        #define MAGNUM_STANFORDIMPORTER_EXPORT CORRADE_VISIBILITY_EXPORT
    #else
        #define MAGNUM_STANFORDIMPORTER_EXPORT CORRADE_VISIBILITY_IMPORT
    #endif
#else
    #define MAGNUM_STANFORDIMPORTER_EXPORT CORRADE_VISIBILITY_STATIC
#endif
#define MAGNUM_STANFORDIMPORTER_LOCAL CORRADE_VISIBILITY_LOCAL
#else
#define MAGNUM_STANFORDIMPORTER_EXPORT
#define MAGNUM_STANFORDIMPORTER_LOCAL
#endif

namespace Magnum { namespace Trade {

/**
@brief Stanford PLY importer plugin

@m_keywords{PLY}

Imports PLY (`*.ply`) files. You can use @ref StanfordSceneConverter to encode
meshes into this format.

@section Trade-StanfordImporter-usage Usage

@m_class{m-note m-success}

@par
    This class is a plugin that's meant to be dynamically loaded and used
    through the base @ref AbstractImporter interface. See its documentation for
    introduction and usage examples.

This plugin depends on the @ref Trade library and is built if
`MAGNUM_WITH_STANFORDIMPORTER` is enabled when building Magnum Plugins. To use
as a dynamic plugin, load @cpp "StanfordImporter" @ce via
@ref Corrade::PluginManager::Manager.

Additionally, if you're using Magnum as a CMake subproject, bundle the
[magnum-plugins repository](https://github.com/mosra/magnum-plugins) and do the
following:

@code{.cmake}
set(MAGNUM_WITH_STANFORDIMPORTER ON CACHE BOOL "" FORCE)
add_subdirectory(magnum-plugins EXCLUDE_FROM_ALL)

# So the dynamically loaded plugin gets built implicitly
add_dependencies(your-app MagnumPlugins::StanfordImporter)
@endcode

To use as a static plugin or as a dependency of another plugin with CMake, put
[FindMagnumPlugins.cmake](https://github.com/mosra/magnum-plugins/blob/master/modules/FindMagnumPlugins.cmake)
into your `modules/` directory, request the `StanfordImporter` component of the
`MagnumPlugins` package and link to the `MagnumPlugins::StanfordImporter`
target:

@code{.cmake}
find_package(MagnumPlugins REQUIRED StanfordImporter)

# ...
target_link_libraries(your-app PRIVATE MagnumPlugins::StanfordImporter)
@endcode

See @ref building-plugins, @ref cmake-plugins, @ref plugins and
@ref file-formats for more information.

@section Trade-StanfordImporter-behavior Behavior and limitations

In order to optimize for fast import, the importer supports a restricted subset
of PLY features, which however shouldn't affect any real-world models.

-   Both Little- and Big-Endian binary files are supported, with bytes swapped
    to match platform endianness.
-   Position coordinates (`x`/`y`/`z`) are expected to have the same type, be
    tightly packed in a XYZ order and be either 32-bit floats or (signed) bytes
    or shorts. Resulting position type is then
    @ref VertexFormat::Vector3, @ref VertexFormat::Vector3ub,
    @ref VertexFormat::Vector3b, @ref VertexFormat::Vector3us or
    @ref VertexFormat::Vector3s. 32-bit integer and double positions are not
    supported.
-   Normal coordinates (`nx`/`ny`/`nz`) are expected to have the same type, be
    tightly packed in a XYZ order and be either 32-bit floats or *signed* bytes
    or shorts. Resulting normal type is then
    @ref VertexFormat::Vector3, @ref VertexFormat::Vector3bNormalized or
    @ref VertexFormat::Vector3sNormalized. 32-bit integer, unsigned and double
    normals are not supported.
-   Texture coordinates (`u`/`v` or `s`/`t`) are expected to have the same
    type, be tightly packed in a XY order and be either 32-bit floats or
    * *unsigned* bytes or shorts. Resulting texture coordinate type is then
    @ref VertexFormat::Vector2, @ref VertexFormat::Vector2ubNormalized or
    @ref VertexFormat::Vector2usNormalized. Double texture coordinates are not
    supported, signed and 32.bit integer coordinates don't have a well-defined
    mapping to the @f$ [0, 1] @f$ range and thus are not supported either.
-   Color channels (`red`/`green`/`blue` and optional `alpha`) are expected
    to have the same type, be tightly packed in a RGB or RGBA order and be
    either 32-bit floats or unsigned bytes/shorts. Tesulting color type is then
    @ref VertexFormat::Vector3 / @ref VertexFormat::Vector4,
    @ref VertexFormat::Vector3ubNormalized / @ref VertexFormat::Vector4ubNormalized
    or @ref VertexFormat::Vector3usNormalized /
    @ref VertexFormat::Vector4usNormalized. Signed, 32-bit integer and double
    colors are not supported.
-   Per-vertex object ID attribute is imported as either
    @ref VertexFormat::UnsignedInt, @ref VertexFormat::UnsignedShort or
    @ref VertexFormat::UnsignedByte. By default `object_id` is the recognized
    name, use the @cb{.ini} objectIdAttribute @ce
    @ref Trade-StanfordImporter-configuration "configuration option" to change
    the identifier that's being looked for. Because there are real-world files
    with signed object IDs, signed types are allowed as well, but interpreted
    as unsigned.
-   Indices (`vertex_indices` or `vertex_index`) are imported as either
    @ref MeshIndexType::UnsignedByte, @ref MeshIndexType::UnsignedShort or
    @ref MeshIndexType::UnsignedInt. Quads are triangulated, but higher-order
    polygons are not supported. Because there are real-world files with signed
    indices, signed types are allowed for indices as well, but interpreted as
    unsigned (because negative values wouldn't make sense anyway).

The mesh is always indexed; positions are always present, other attributes are
optional.

The importer recognizes @ref ImporterFlag::Verbose, printing additional info
when the flag is enabled.

@subsection Trade-StanfordImporter-behavior-per-face Per-face attributes

By default, if the mesh contains per-face attributes apart from indices, these
get converted to per-vertex during import. This is what the users usually want,
however for large meshes this may have a performance impact due to calculating
only unique per-vertex/per-face data combinations. If this conversion takes
place, the resulting index type is always @ref MeshIndexType::UnsignedInt,
independent of what the file has.

Alternatively, if the @cb{.ini} perFaceToPerVertex @ce
@ref Trade-StanfordImporter-configuration "configuration option" is disabled,
the importer provides access to per-face attributes in a second mesh level ---
i.e., @ref meshLevelCount() returns @cpp 2 @ce in that case, and calling
@ref mesh() with @p level set to @cpp 1 @ce will return a @ref MeshData
instance with @ref MeshPrimitive::Faces. The faces are triangulated, which
means each item contains data for three vertices in the
@ref MeshPrimitive::Triangles mesh at <tt>level</tt> @cpp 0 @ce.

From the builtin attributes, colors, normals and object IDs can be either
per-vertex or per-face, positions and texture coordinates are always
per-vertex.

@subsection Trade-StanfordImporter-behavior-custom-attributes Custom attributes

Custom and unrecognized vertex and face attributes of known types are present
in the imported meshes as well. Their mapping to/from a string can be queried
using @ref meshAttributeName() and @ref meshAttributeForName(). Attributes with
unknown types cause the import to fail, as the format relies on knowing the
type size.

@subsection Trade-StanfordImporter-behavior-ascii ASCII files

The plugin implements parsing of binary files only. If an ASCII file is
detected, it's forwarded to the @ref AssimpImporter plugin, if available. Calls
to @ref meshCount(), @ref meshLevelCount(), @ref meshAttributeName(),
@ref meshAttributeForName() and @ref mesh() are then proxied to
@ref AssimpImporter. The @ref close() function closes and discards the
internally instantiated plugin; @ref isOpened() works as usual.

Note that @ref AssimpImporter implements only a subset of features provided by
this plugin. In particular, ASCII files with custom mesh attributes or per-face
attributes will be imported as if they didn't have them, and both index and
vertex data will be always in the full 32-bit type.

@section Trade-StanfordImporter-configuration Plugin-specific configuration

It's possible to tune various import options through @ref configuration(). See
below for all options and their default values:

@snippet MagnumPlugins/StanfordImporter/StanfordImporter.conf configuration_

See @ref plugins-configuration for more information and an example showing how
to edit the configuration values.
*/
class MAGNUM_STANFORDIMPORTER_EXPORT StanfordImporter: public AbstractImporter {
    public:
        #ifdef MAGNUM_BUILD_DEPRECATED
        /**
         * @brief Default constructor
         * @m_deprecated_since_latest Direct plugin instantiation isn't a
         *      supported use case anymore, instantiate through the plugin
         *      manager instead.
         */
        CORRADE_DEPRECATED("instantiate through the plugin manager instead") explicit StanfordImporter();
        #endif

        /** @brief Plugin manager constructor */
        explicit StanfordImporter(PluginManager::AbstractManager& manager, const Containers::StringView& plugin);

        ~StanfordImporter();

    private:
        MAGNUM_STANFORDIMPORTER_LOCAL ImporterFeatures doFeatures() const override;

        MAGNUM_STANFORDIMPORTER_LOCAL bool doIsOpened() const override;
        MAGNUM_STANFORDIMPORTER_LOCAL void doOpenData(Containers::Array<char>&& data, DataFlags dataFlags) override;
        MAGNUM_STANFORDIMPORTER_LOCAL void doClose() override;

        MAGNUM_STANFORDIMPORTER_LOCAL UnsignedInt doMeshCount() const override;
        MAGNUM_STANFORDIMPORTER_LOCAL UnsignedInt doMeshLevelCount(UnsignedInt id) override;
        MAGNUM_STANFORDIMPORTER_LOCAL Containers::Optional<MeshData> doMesh(UnsignedInt id, UnsignedInt level) override;
        MAGNUM_STANFORDIMPORTER_LOCAL MeshAttribute doMeshAttributeForName(const Containers::StringView name) override;
        MAGNUM_STANFORDIMPORTER_LOCAL Containers::String doMeshAttributeName(MeshAttribute name) override;

        struct State;
        Containers::Pointer<State> _state;
        Containers::Pointer<AbstractImporter> _assimpImporter;
};

}}

#endif
