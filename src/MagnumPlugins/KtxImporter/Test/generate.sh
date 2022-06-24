#!/bin/bash

set -e

# The Khronos tools don't support array, cubemap or 3D textures, so we need
# PVRTexTool as well. That doesn't support 3D textures, but we can patch the
# header of 2D array textures to turn them into 3D textures.
# Not a fan of using closed-source tools, but it works...
# https://github.com/KhronosGroup/KTX-Software v4.0.0
# https://developer.imaginationtech.com/pvrtextool/ v5.1.0

toktx version1.ktx black.png

toktx --t2 --swizzle rgb1 swizzle-identity.ktx2 pattern.png
toktx --t2 --target_type RGBA --swizzle rgb1 swizzle-unsupported.ktx2 pattern.png

# swizzled data, same swizzle in header
# data should come out normally after the importer swizzles it
toktx --t2 --input_swizzle bgra --swizzle bgr1 bgr-swizzle-bgr.ktx2 pattern.png
toktx --t2 --target_type RGBA --input_swizzle bgra --swizzle bgra bgra-swizzle-bgra.ktx2 pattern.png
toktx --t2 --input_swizzle bgra --swizzle bgr1 bgr-swizzle-bgr-16bit.ktx2 pattern-16bit.png

# swizzled header
# with patched swizzled vkFormat data should come out normally because both cancel each other out
toktx --t2 --swizzle bgr1 swizzle-bgr.ktx2 pattern.png
toktx --t2 --target_type RGBA --swizzle bgra swizzle-bgra.ktx2 pattern.png

# swizzled data
# with patched swizzled vkFormat data should come out normally
toktx --t2 --input_swizzle bgra bgr.ktx2 pattern.png
toktx --t2 --target_type RGBA --input_swizzle bgra bgra.ktx2 pattern.png

# 2D
toktx --t2 2d-rgb.ktx2 pattern.png
toktx --t2 --target_type RGBA 2d-rgba.ktx2 pattern.png

# A few interesting formats
# PVRTexTool lets us export to different formats although the documentation
# isn't very clear on how it converts
PVRTexToolCLI -i pattern.png -o 2d-rgb32.ktx2 -f r32g32b32,UI,sRGB
PVRTexToolCLI -i pattern.png -o 2d-rgbf32.ktx2 -f r32g32b32,SF,sRGB

# manual mipmaps, full pyramid and one without the last
toktx --t2 --mipmap 2d-mipmaps.ktx2 pattern.png pattern-mip1.png pattern-mip2.png
toktx --t2 --mipmap --levels 2 2d-mipmaps-incomplete.ktx2 pattern.png pattern-mip1.png

# layers
PVRTexToolCLI -i pattern.png,pattern.png,black.png -o 2d-layers.ktx2 -array -f r8g8b8,UBN,sRGB

# mipmaps and layers
# PVRTexTool doesn't let us specify mip images manually
PVRTexToolCLI -i pattern.png,pattern.png,black.png -o 2d-mipmaps-and-layers.ktx2 -array -m -mfilter nearest -f r8g8b8,UBN,sRGB

# Cube map. Source data for the cube-N.png files is in KtxImporterTest.cpp in
# a FacesRgbData array.
PVRTexToolCLI -i cube+x.png,cube-x.png,cube+y.png,cube-y.png,cube+z.png,cube-z.png -o cubemap.ktx2 -cube -f r8g8b8,UBN,sRGB
PVRTexToolCLI -i cube+x.png,cube-x.png,cube+y.png,cube-y.png,cube+z.png,cube-z.png -o cubemap-mipmaps.ktx2 -cube -m -mfilter nearest -f r8g8b8,UBN,sRGB
# layered cube map: faces for layer 0, then layer 1
PVRTexToolCLI -i cube+x.png,cube-x.png,cube+y.png,cube-y.png,cube+z.png,cube-z.png,cube+z.png,cube-z.png,cube+x.png,cube-x.png,cube+y.png,cube-y.png -o cubemap-layers.ktx2 -cube -array -f r8g8b8,UBN,sRGB

# 1D
# toktx 4.0 has a bug and doesn't write KTXorientation for 1D images, 4.1 RC
# does. To avoid unnecessary warning when testing opening 1D files in
# KtxImporter, the file is faked and produced by KtxImageConverterTest instead
# of toktx
# TODO re-enable and regenerate all test files when toktx 4.1 is stable
# toktx --t2 1d.ktx2 pattern-1d.png
# toktx --t2 --mipmap 1d-mipmaps.ktx2 pattern-1d.png pattern-mip1.png pattern-mip2.png
PVRTexToolCLI -i pattern-1d.png,pattern-1d.png,black-1d.png -o 1d-layers.ktx2 -array -f r8g8b8,UBN,sRGB

# 3D
# PVRTexTool doesn't support 3D images, sigh
# Create a layered image and patch the header to make it 3D. The level layout
# is identical to a 3D image, but the orientation metadata will become invalid.
# The importer will ignore invalid orientation but for correct ground-truth
# data for converter tests we need to patch it.
# Because the z axis will be flipped, we need to pass the images in reverse
# order compared to 2d-layers.ktx2.
PVRTexToolCLI -i black.png,pattern.png,pattern.png -o 3d.ktx2 -array -f r8g8b8,UBN,sRGB
# depth = 3, numLayers = 0
printf '\x03\x00\x00\x00\x00\x00\x00\x00' | dd conv=notrunc of=3d.ktx2 bs=1 seek=28
# KTXorientation length
printf '\x13' | dd conv=notrunc of=3d.ktx2 bs=1 seek=180
# KTXorientation, replace first padding byte
printf 'i' | dd conv=notrunc of=3d.ktx2 bs=1 seek=201

# We can't patch a 2D array texture with mipmaps into a 3D texture because the
# number of layers stays the same, unlike shrinking z in mipmap levels
# 3d-mipmaps.ktx2 and 3d-compressed-mipmaps.ktx2 are generated by running
# KtxImageConverterTest --save-diagnostic
PVRTexToolCLI -i black.png,pattern.png,pattern.png,black.png,black.png,pattern.png -o 3d-layers.ktx2 -array -f r8g8b8,UBN,sRGB
printf '\x03\x00\x00\x00\x02\x00\x00\x00' | dd conv=notrunc of=3d-layers.ktx2 bs=1 seek=28
# TODO: patch up KTXorientation for 3d-layers.ktx2 if we need it for the converter tests

# Compressed
# PVRTC and BC* don't support non-power-of-2
PVRTexToolCLI -i pattern-pot.png -o 2d-compressed-pvrtc.ktx2 -f PVRTC1_4,UBN,sRGB
PVRTexToolCLI -i pattern-pot.png -o 2d-compressed-bc1.ktx2 -f BC1,UBN,sRGB
PVRTexToolCLI -i pattern-pot.png -o 2d-compressed-bc3.ktx2 -f BC3,UBN,sRGB
PVRTexToolCLI -i pattern-uneven.png -o 2d-compressed-etc2.ktx2 -f ETC2_RGB,UBN,sRGB
PVRTexToolCLI -i pattern-uneven.png -o 2d-compressed-astc.ktx2 -f ASTC_12x10,UBN,sRGB

PVRTexToolCLI -i pattern-uneven.png -o 2d-compressed-mipmaps.ktx2 -m -mfilter nearest -f ETC2_RGB,UBN,sRGB
PVRTexToolCLI -i pattern-uneven.png,black.png -o 2d-compressed-layers.ktx2 -array -f ETC2_RGB,UBN,sRGB

PVRTexToolCLI -i pattern-1d.png -o 1d-compressed-bc1.ktx2 -f BC1,UBN,sRGB
PVRTexToolCLI -i pattern-1d-uneven.png -o 1d-compressed-etc2.ktx2 -f ETC2_RGB,UBN,sRGB

PVRTexToolCLI -i pattern-1d-uneven.png -o 1d-compressed-mipmaps.ktx2 -m -mfilter nearest -f ETC2_RGB,UBN,sRGB

PVRTexToolCLI -i pattern-uneven.png,pattern-uneven.png,black.png -o 3d-compressed.ktx2 -array -f ETC2_RGB,UBN,sRGB
printf '\x03\x00\x00\x00\x00\x00\x00\x00' | dd conv=notrunc of=3d-compressed.ktx2 bs=1 seek=28
printf '\x13' | dd conv=notrunc of=3d-compressed.ktx2 bs=1 seek=148
printf 'i' | dd conv=notrunc of=3d-compressed.ktx2 bs=1 seek=169

# Unlike 3d-mipmaps.ktx2, we don't have any verifiable data to check in viewers
# for 3d-compressed-mipmaps.ktx to make sure the converter output is plausible.
# Instead of randomly generating bytes, some of the existing 2D ETC2 .bin files
# are used to manually create the mipmaps. A hack, but better than nothing.
# We can simply repeat all the blocks for each z-slice.
# Last slice (if any) is all zeros to check the order.
yes 2d-compressed-mipmaps-mip0.bin | head -n 4 | xargs cat > 3d-compressed-mipmaps-mip0.bin
size=$(stat -c%s 2d-compressed-mipmaps-mip0.bin)
dd if=/dev/zero bs=1 count=$size >> 3d-compressed-mipmaps-mip0.bin

cp 2d-compressed-mipmaps-mip1.bin 3d-compressed-mipmaps-mip1.bin
size=$(stat -c%s 2d-compressed-mipmaps-mip1.bin)
dd if=/dev/zero bs=1 count=$size >> 3d-compressed-mipmaps-mip1.bin

cp 2d-compressed-mipmaps-mip2.bin 3d-compressed-mipmaps-mip2.bin
cp 2d-compressed-mipmaps-mip3.bin 3d-compressed-mipmaps-mip3.bin

# TODO:
# 3D (compressed) mips so we don't have to generate our own in the converter tests
# Should be possible once https://github.com/KhronosGroup/KTX-Software/pull/468 made it into a release
