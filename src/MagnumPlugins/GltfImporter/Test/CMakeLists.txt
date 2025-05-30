#
#   This file is part of Magnum.
#
#   Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
#               2020, 2021, 2022, 2023, 2024, 2025
#             Vladimír Vondruš <mosra@centrum.cz>
#   Copyright © 2021 Pablo Escobar <mail@rvrs.in>
#
#   Permission is hereby granted, free of charge, to any person obtaining a
#   copy of this software and associated documentation files (the "Software"),
#   to deal in the Software without restriction, including without limitation
#   the rights to use, copy, modify, merge, publish, distribute, sublicense,
#   and/or sell copies of the Software, and to permit persons to whom the
#   Software is furnished to do so, subject to the following conditions:
#
#   The above copyright notice and this permission notice shall be included
#   in all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#   DEALINGS IN THE SOFTWARE.
#

# IDE folder in VS, Xcode etc. CMake 3.12+, older versions have only the FOLDER
# property that would have to be set on each target separately.
set(CMAKE_FOLDER "MagnumPlugins/GltfImporter/Test")

if(CORRADE_TARGET_EMSCRIPTEN OR CORRADE_TARGET_ANDROID)
    set(GLTFIMPORTER_TEST_DIR ".")
else()
    set(GLTFIMPORTER_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

find_package(Magnum REQUIRED DebugTools MeshTools)

if(NOT MAGNUM_GLTFIMPORTER_BUILD_STATIC)
    set(GLTFIMPORTER_PLUGIN_FILENAME $<TARGET_FILE:GltfImporter>)
    if(MAGNUM_WITH_BASISIMPORTER)
        set(BASISIMPORTER_PLUGIN_FILENAME $<TARGET_FILE:BasisImporter>)
    endif()
    if(MAGNUM_WITH_DDSIMPORTER)
        set(DDSIMPORTER_PLUGIN_FILENAME $<TARGET_FILE:DdsImporter>)
    endif()
    if(MAGNUM_WITH_KTXIMPORTER)
        set(KTXIMPORTER_PLUGIN_FILENAME $<TARGET_FILE:KtxImporter>)
    endif()
    if(MAGNUM_WITH_STBIMAGEIMPORTER)
        set(STBIMAGEIMPORTER_PLUGIN_FILENAME $<TARGET_FILE:StbImageImporter>)
    endif()
endif()

# First replace ${} variables, then $<> generator expressions
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/configure.h.in)
file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/configure.h
    INPUT ${CMAKE_CURRENT_BINARY_DIR}/configure.h.in)

corrade_add_test(GltfImporterDecodeTest GltfImporterDecodeTest.cpp LIBRARIES Magnum::Magnum)
target_include_directories(GltfImporterDecodeTest PRIVATE ${PROJECT_SOURCE_DIR}/src)

corrade_add_resource(GltfImporterTest_RESOURCES resources.conf)

# Emscripten 3.1.3 changed the way files are bundled, putting them directly to
# WASM instead of Base64'd to the JS file. However, it broke UTF-8 handling,
# causing both a compile error (due to a syntax error in the assembly file) and
# if that's patched, also runtime errors later.
#   https://github.com/emscripten-core/emscripten/pull/16050
# TODO: re-add and re-enable the test cases once a fix is made
if(NOT CORRADE_TARGET_EMSCRIPTEN OR EMSCRIPTEN_VERSION VERSION_LESS 3.1.3)
    set(GltfImporterTest_UTF8_FILES
        přívodní-šňůra.gltf
        přívodní-šňůra.bin
        přívodní-šňůra.png)
endif()
corrade_add_test(GltfImporterTest
    GltfImporterTest.cpp
    ${GltfImporterTest_RESOURCES}
    LIBRARIES
        Magnum::DebugTools
        Magnum::MeshTools
        Magnum::Trade
    FILES
        animation.gltf
        animation.glb
        animation.bin
        animation-embedded.gltf
        animation-embedded.glb
        animation-invalid.gltf
        animation-invalid-buffer-notfound.gltf
        animation-missing-target-node.gltf
        animation-patching.bin
        animation-patching.gltf
        animation-splines-sharing.gltf
        animation-splines-single-or-no-keyframes.gltf
        buffer-invalid-short-size.glb
        buffer-invalid-short-size.gltf
        buffer-invalid-short-size-embedded.glb
        buffer-invalid-short-size-embedded.gltf
        buffer-long-size.gltf
        buffer-long-size.glb
        camera.gltf
        camera-invalid.gltf
        empty.gltf
        empty.glb
        encoded-uris.gltf
        error.gltf
        escaped-strings.gltf
        external-data.bin
        # external-data.gl* packed via a resource (but *.bin needed also by
        # buffer-wrong-size.gltf, so added separately as well)
        external-data-order.glb
        external-data-order.gltf
        image.basis
        image.gltf
        image.glb
        image.png
        image-2d-mips.ktx2 # Copied from KtxImporter/Test/2d-mipmaps.ktx2
        image-2d-array.ktx2 # Copied from KtxImporter/Test/2d-layers.ktx2
        image-2d-array-mips.ktx2 # Copied from KtxImporter/Test/2d-mipmaps-and-layers.ktx2
        image-3d.dds # Copied from DdsImporter/Test/dxt10-rgba8unorm-3d.dds
        image-basis.gltf
        image-basis.glb
        image-basis-embedded.gltf
        image-basis-embedded.glb
        image-buffer.gltf
        image-buffer.glb
        image-buffer-embedded.gltf
        image-buffer-embedded.glb
        image-embedded.gltf
        image-embedded.glb
        image-invalid.gltf
        image-invalid-notfound.gltf
        image-invalid-ktx.gltf
        image-invalid-ktx-notfound.gltf
        invalid-uri.gltf
        light.gltf
        light-invalid.gltf
        material-common.gltf
        material-clearcoat.gltf
        material-extras.gltf
        material-invalid.gltf
        material-invalid-ktx.gltf
        material-ior.gltf
        material-metallicroughness.gltf
        material-phong-fallback.gltf
        material-phong-fallback-ktx.gltf
        material-raw.gltf
        material-sheen.gltf
        material-specular.gltf
        material-specularglossiness.gltf
        material-transmission.gltf
        material-unlit.gltf
        material-volume.gltf
        mesh.gltf
        mesh.bin
        mesh.glb
        mesh-buffers.gltf
        mesh-buffers.0.bin
        mesh-buffers.1.bin
        mesh-buffers.2.bin
        mesh-colors.gltf
        mesh-colors.bin
        mesh-custom-attributes.bin
        mesh-custom-attributes.gltf
        mesh-duplicate-attributes.gltf
        mesh-embedded.gltf
        mesh-embedded.glb
        mesh-invalid.gltf
        mesh-invalid-buffer-notfound.gltf
        mesh-invalid-empty-primitives.gltf
        mesh-invalid-missing-primitives-property.gltf
        mesh-invalid-morph-target.gltf
        mesh-invalid-primitive-attributes-property.gltf
        mesh-invalid-primitive-targets-property.gltf
        mesh-invalid-primitive.gltf
        mesh-invalid-primitives-property.gltf
        mesh-invalid-sparse-indices.gltf
        mesh-invalid-sparse-indices.bin
        mesh-invalid-texcoord-flip-attribute-accessor-invalid-component-type.gltf
        mesh-invalid-texcoord-flip-attribute-accessor-invalid-normalized.gltf
        mesh-invalid-texcoord-flip-attribute-accessor-missing-component-type.gltf
        mesh-invalid-texcoord-flip-attribute-oob.gltf
        mesh-invalid-texcoord-flip-attribute.gltf
        mesh-invalid-texcoord-flip-morph-target-attribute.gltf
        mesh-morph-target-attributes.gltf
        mesh-morph-target-attributes.bin
        mesh-multiple-primitives.gltf
        mesh-no-indices-no-vertices-no-buffer-uri.gltf
        mesh-no-indices-no-vertices-no-buffer-uri.glb
        mesh-primitives-types.gltf
        mesh-primitives-types.bin
        mesh-size-not-multiple-of-stride.gltf
        mesh-size-not-multiple-of-stride.bin
        mesh-skin-attributes.gltf
        mesh-skin-attributes.bin
        mesh-sparse.gltf
        mesh-sparse.bin
        mesh-unordered-attributes.gltf
        mesh-unsigned-int-vertex-formats.gltf
        mesh-unsigned-int-vertex-formats.bin
        mesh-unsupported-vertex-formats.gltf
        mesh-unsupported-vertex-formats.bin
        ${GltfImporterTest_UTF8_FILES}
        required-extensions.gltf
        required-extensions-unsupported.gltf
        scene.gltf
        scene-custom-fields.gltf
        scene-default-none.gltf
        scene-default-oob.gltf
        scene-invalid-child-not-root.gltf
        scene-invalid-children-property.gltf
        scene-invalid-cycle.gltf
        scene-invalid-cycle-deep.gltf
        scene-invalid-extras-property.gltf
        scene-invalid-nested-extras-property.gltf
        scene-invalid-multiple-parents.gltf
        scene-invalid-child-oob.gltf
        scene-invalid-node-oob.gltf
        scene-invalid-nodes-property.gltf
        scene-invalid.gltf
        scene-transformation.gltf
        scene-transformation-patching.gltf
        skin-embedded.glb
        skin-embedded.gltf
        skin-invalid.gltf
        skin-invalid-buffer-notfound.gltf
        skin.gltf
        skin.bin
        skin.glb
        texcoord-flip.bin
        texcoord-flip.gltf
        texcoord-flip-unnormalized.gltf
        texcoord-flip-unnormalized-morph-target.gltf
        texture.gltf
        texture-extensions.gltf
        texture-invalid.gltf
        texture-invalid-ktx-extension.gltf
        texture-invalid-ktx-extension-source-oob.gltf
        texture-invalid-ktx-extension-source-property.gltf
        texture-invalid-ktx-extensions-property.gltf
        texture-invalid-ktx-layer-then-no-extension.gltf
        texture-invalid-ktx-layer-then-no-layer.gltf
        texture-invalid-ktx-missing-source-property.gltf
        texture-invalid-ktx-missing-extension-source-property.gltf
        texture-invalid-ktx-no-extension-then-layer.gltf
        texture-invalid-ktx-no-layer-then-layer.gltf
        texture-invalid-ktx-source-oob.gltf
        texture-invalid-ktx-source-property.gltf
        texture-ktx.gltf
        version-legacy.gltf
        version-supported.gltf
        version-unsupported.gltf
        version-unsupported-min.gltf)
target_include_directories(GltfImporterTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
if(MAGNUM_GLTFIMPORTER_BUILD_STATIC)
    target_link_libraries(GltfImporterTest PRIVATE GltfImporter)
    if(MAGNUM_WITH_BASISIMPORTER)
        target_link_libraries(GltfImporterTest PRIVATE BasisImporter)
    endif()
    if(MAGNUM_WITH_DDSIMPORTER)
        target_link_libraries(GltfImporterTest PRIVATE DdsImporter)
    endif()
    if(WITH_KTXIMPORTER)
        target_link_libraries(GltfImporterTest PRIVATE KtxImporter)
    endif()
    if(MAGNUM_WITH_STBIMAGEIMPORTER)
        target_link_libraries(GltfImporterTest PRIVATE StbImageImporter)
    endif()
else()
    # So the plugins get properly built when building the test
    add_dependencies(GltfImporterTest GltfImporter)
    if(MAGNUM_WITH_BASISIMPORTER)
        add_dependencies(GltfImporterTest BasisImporter)
    endif()
    if(MAGNUM_WITH_DDSIMPORTER)
        add_dependencies(GltfImporterTest DdsImporter)
    endif()
    if(MAGNUM_WITH_KTXIMPORTER)
        add_dependencies(GltfImporterTest KtxImporter)
    endif()
    if(MAGNUM_WITH_STBIMAGEIMPORTER)
        add_dependencies(GltfImporterTest StbImageImporter)
    endif()
endif()
if(CORRADE_BUILD_STATIC AND NOT MAGNUM_GLTFIMPORTER_BUILD_STATIC)
    # CMake < 3.4 does this implicitly, but 3.4+ not anymore (see CMP0065).
    # That's generally okay, *except if* the build is static, the executable
    # uses a plugin manager and needs to share globals with the plugins (such
    # as output redirection and so on).
    set_target_properties(GltfImporterTest PROPERTIES ENABLE_EXPORTS ON)
endif()
