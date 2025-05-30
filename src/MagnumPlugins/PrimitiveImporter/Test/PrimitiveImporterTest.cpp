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

#include <Corrade/Containers/Optional.h>
#include <Corrade/Containers/Pair.h>
#include <Corrade/Containers/String.h>
#include <Corrade/TestSuite/Tester.h>
#include <Corrade/TestSuite/Compare/Numeric.h>
#include <Corrade/TestSuite/Compare/String.h>
#include <Corrade/Utility/ConfigurationGroup.h>
#include <Corrade/Utility/Format.h>
#include <Magnum/Math/Matrix3.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/Trade/AbstractImporter.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/Trade/SceneData.h>

#include "configure.h"

namespace Magnum { namespace Trade { namespace Test { namespace {

struct PrimitiveImporterTest: TestSuite::Tester {
    explicit PrimitiveImporterTest();

    void test();
    void mesh();
    void meshInvalid();

    void scene2D();
    void scene3D();

    /* Explicitly forbid system-wide plugin dependencies */
    PluginManager::Manager<AbstractImporter> _manager{"nonexistent"};
};

constexpr struct {
    const char* name;
    UnsignedInt vertexCount;
    UnsignedInt indexCount;
} Data[] {
    {"axis2D", 8, 12},
    {"axis3D", 12, 18},

    {"capsule2DWireframe", 34, 68},
    {"capsule3DSolid", 98, 576},
    {"capsule3DWireframe", 90, 200},

    {"circle2DSolid", 18, 0},
    {"circle2DWireframe", 32, 0},
    {"circle3DSolid", 18, 0},
    {"circle3DWireframe", 32, 0},

    {"coneSolid", 37, 108},
    {"coneWireframe", 33, 72},

    {"crosshair2D", 4, 0},
    {"crosshair3D", 6, 0},

    {"cubeSolid", 24, 36},
    {"cubeSolidStrip", 14, 0},
    {"cubeWireframe", 8, 24},

    {"cylinderSolid", 50, 144},
    {"cylinderWireframe", 64, 136},

    {"gradient2D", 4, 0},
    {"gradient2DHorizontal", 4, 0},
    {"gradient2DVertical", 4, 0},
    {"gradient3D", 4, 0},
    {"gradient3DHorizontal", 4, 0},
    {"gradient3DVertical", 4, 0},

    {"grid3DSolid", 35, 144},
    {"grid3DWireframe", 35, 116},

    {"icosphereSolid", 42, 240},
    {"icosphereWireframe", 12, 60},

    {"line2D", 2, 0},
    {"line3D", 2, 0},

    {"planeSolid", 4, 0},
    {"planeWireframe", 4, 0},

    {"squareSolid", 4, 0},
    {"squareWireframe", 4, 0},

    {"uvSphereSolid", 114, 672},
    {"uvSphereWireframe", 90, 192}
};

const struct {
    const char* mesh;
    const char* name;
    const char* option;
    const char* validValue;
    const char* value;
    const char* expected;
} MeshInvalidData[]{
    {"capsule2DWireframe", "zero hemisphereRings",
        "hemisphereRings", "1", "0",
        "expected hemisphereRings and cylinderRings to be at least 1 for {} but got 0 and 1"},
    {"capsule2DWireframe", "zero cylinderRings",
        "cylinderRings", "1", "0",
        "expected hemisphereRings and cylinderRings to be at least 1 for {} but got 8 and 0"},
    {"capsule3DSolid", "zero hemisphereRings",
        "hemisphereRings", "1", "0",
        "expected hemisphereRings and cylinderRings to be at least 1 and segments at least 3 for {} but got 0, 1 and 12"},
    {"capsule3DSolid", "zero cylinderRings",
        "cylinderRings", "1", "0",
        "expected hemisphereRings and cylinderRings to be at least 1 and segments at least 3 for {} but got 4, 0 and 12"},
    {"capsule3DSolid", "too little segments",
        "segments", "3", "2",
        "expected hemisphereRings and cylinderRings to be at least 1 and segments at least 3 for {} but got 4, 1 and 2"},
    {"capsule3DWireframe", "zero hemisphereRings",
        "hemisphereRings", "1", "0",
        "expected hemisphereRings and cylinderRings to be at least 1 and segments to be multiples of 4 for {} but got 0, 1 and 16"},
    {"capsule3DWireframe", "zero cylinderRings",
        "cylinderRings", "1", "0",
        "expected hemisphereRings and cylinderRings to be at least 1 and segments to be multiples of 4 for {} but got 8, 0 and 16"},
    {"capsule3DWireframe", "non-multiple-of-four segments",
        "segments", "8", "9",
        "expected hemisphereRings and cylinderRings to be at least 1 and segments to be multiples of 4 for {} but got 8, 1 and 9"},
    {"capsule3DWireframe", "zero segments",
        "segments", "4", "0",
        "expected hemisphereRings and cylinderRings to be at least 1 and segments to be multiples of 4 for {} but got 8, 1 and 0"},
    {"circle2DSolid", "too little segments",
        "segments", "3", "2",
        "expected segments to be at least 3 for {} but got 2"},
    {"circle2DWireframe", "too little segments",
        "segments", "3", "2",
        "expected segments to be at least 3 for {} but got 2"},
    {"circle3DSolid", "too little segments",
        "segments", "3", "2",
        "expected segments to be at least 3 for {} but got 2"},
    {"circle3DWireframe", "too little segments",
        "segments", "3", "2",
        "expected segments to be at least 3 for {} but got 2"},
    {"coneSolid", "zero rings",
        "rings", "1", "0",
        "expected rings to be at least 1 and segments at least 3 for {} but got 0 and 12"},
    {"coneSolid", "too little segments",
        "segments", "3", "2",
        "expected rings to be at least 1 and segments at least 3 for {} but got 1 and 2"},
    {"coneWireframe", "non-multiple-of-four segments",
        "segments", "8", "9",
        "expected segments to be multiples of 4 for {} but got 9"},
    {"coneWireframe", "zero segments",
        "segments", "4", "0",
        "expected segments to be multiples of 4 for {} but got 0"},
    {"cubeSolid", "invalid textureCoordinates",
        "textureCoordinates", "negativeXUpNegativeXDown", "positiveYUpNegativeYDown",
        "unrecognized textureCoordinates value positiveYUpNegativeYDown for {}"},
    {"cubeSolid", "tangents with no textureCoordinates",
        "tangents", "false", "true",
        "cannot enable {} tangents with no textureCoordinates"},
    {"cylinderSolid", "zero rings",
        "rings", "1", "0",
        "expected rings to be at least 1 and segments at least 3 for {} but got 0 and 12"},
    {"cylinderSolid", "too little segments",
        "segments", "3", "2",
        "expected rings to be at least 1 and segments at least 3 for {} but got 1 and 2"},
    {"cylinderWireframe", "zero rings",
        "rings", "1", "0",
        "expected rings to be at least 1 and segments to be multiples of 4 for {} but got 0 and 32"},
    {"cylinderWireframe", "non-multiple-of-four segments",
        "segments", "8", "9",
        "expected rings to be at least 1 and segments to be multiples of 4 for {} but got 1 and 9"},
    {"cylinderWireframe", "zero segments",
        "segments", "4", "0",
        "expected rings to be at least 1 and segments to be multiples of 4 for {} but got 1 and 0"},
    {"uvSphereSolid", "too little rings",
        "rings", "2", "1",
        "expected rings to be at least 2 and segments at least 3 for {} but got 1 and 16"},
    {"uvSphereSolid", "too little segments",
        "segments", "3", "2",
        "expected rings to be at least 2 and segments at least 3 for {} but got 8 and 2"},
    {"uvSphereWireframe", "non-multiple-of-two rings",
        "rings", "4", "5",
        "expected rings to be multiples of 2 and segments multiples of 4 for {} but got 5 and 32"},
    {"uvSphereWireframe", "zero rings",
        "rings", "2", "0",
        "expected rings to be multiples of 2 and segments multiples of 4 for {} but got 0 and 32"},
    {"uvSphereWireframe", "non-multiple-of-four segments",
        "segments", "8", "9",
        "expected rings to be multiples of 2 and segments multiples of 4 for {} but got 16 and 9"},
    {"uvSphereWireframe", "zero segments",
        "segments", "4", "0",
        "expected rings to be multiples of 2 and segments multiples of 4 for {} but got 16 and 0"},
};

PrimitiveImporterTest::PrimitiveImporterTest() {
    addTests({&PrimitiveImporterTest::test});

    addInstancedTests({&PrimitiveImporterTest::mesh},
        Containers::arraySize(Data));

    addInstancedTests({&PrimitiveImporterTest::meshInvalid},
        Containers::arraySize(MeshInvalidData));

    addTests({&PrimitiveImporterTest::scene2D,
              &PrimitiveImporterTest::scene3D});

    /* Load the plugin directly from the build tree. Otherwise it's static and
       already loaded. */
    #ifdef PRIMITIVEIMPORTER_PLUGIN_FILENAME
    CORRADE_INTERNAL_ASSERT_OUTPUT(_manager.load(PRIMITIVEIMPORTER_PLUGIN_FILENAME) & PluginManager::LoadState::Loaded);
    #endif
}

void PrimitiveImporterTest::test() {
    Containers::Pointer<AbstractImporter> importer = _manager.instantiate("PrimitiveImporter");

    /* Due to checks in AbstractImporter, the importer has to manage opened
       state but other than that it doesn't matter what's opened */
    CORRADE_VERIFY(!importer->isOpened());
    CORRADE_VERIFY(importer->openData(nullptr));

    /* We should have all data for the importer */
    CORRADE_COMPARE(Containers::arraySize(Data), importer->meshCount());

    /* Name mapping should work both ways */
    {
        Int icosphere = importer->meshForName("icosphereSolid");
        CORRADE_COMPARE_AS(icosphere, 0, TestSuite::Compare::GreaterOrEqual);
        CORRADE_COMPARE(importer->meshName(icosphere), "icosphereSolid");
        CORRADE_COMPARE(importer->meshForName("bla"), -1);
    } {
        Int icosphere = importer->objectForName("icosphereSolid");
        CORRADE_COMPARE_AS(icosphere, 0, TestSuite::Compare::GreaterOrEqual);
        CORRADE_COMPARE(importer->objectName(icosphere), "icosphereSolid");
        CORRADE_COMPARE(importer->objectForName("bla"), -1);
    }

    /* This should work too */
    importer->close();
}

void PrimitiveImporterTest::mesh() {
    auto&& data = Data[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Containers::Pointer<AbstractImporter> importer = _manager.instantiate("PrimitiveImporter");
    CORRADE_VERIFY(importer->openData({}));

    Containers::Optional<Trade::MeshData> mesh = importer->mesh(data.name);
    CORRADE_VERIFY(mesh);
    CORRADE_COMPARE(mesh->vertexCount(), data.vertexCount);
    if(data.indexCount) {
        CORRADE_VERIFY(mesh->isIndexed());
        CORRADE_COMPARE(mesh->indexCount(), data.indexCount);
    } else CORRADE_VERIFY(!mesh->isIndexed());
}

void PrimitiveImporterTest::meshInvalid() {
    auto&& data = MeshInvalidData[testCaseInstanceId()];
    setTestCaseDescription(Utility::format("{}, {}", data.mesh, data.name));

    Containers::Pointer<AbstractImporter> importer = _manager.instantiate("PrimitiveImporter");
    CORRADE_VERIFY(importer->openData({}));

    Utility::ConfigurationGroup* group = importer->configuration().group(data.mesh);
    CORRADE_VERIFY(group);

    /* Try a value that should work to catch off-by-one errors */
    group->setValue(data.option, data.validValue);
    CORRADE_VERIFY(importer->mesh(data.mesh));
    group->setValue(data.option, data.value);

    Containers::String out;
    Error redirectError{&out};
    CORRADE_VERIFY(!importer->mesh(data.mesh));
    CORRADE_COMPARE_AS(out,
        Utility::format("Trade::PrimitiveImporter::mesh(): {}\n", Utility::format(data.expected, data.mesh)),
        TestSuite::Compare::String);
}

void PrimitiveImporterTest::scene2D() {
    Containers::Pointer<AbstractImporter> importer = _manager.instantiate("PrimitiveImporter");

    /* Due to checks in AbstractImporter, the importer has to manage opened
       state but other than that it doesn't matter what's opened */
    CORRADE_VERIFY(!importer->isOpened());
    CORRADE_VERIFY(importer->openData(nullptr));

    CORRADE_COMPARE(importer->objectCount(), Containers::arraySize(Data));

    /* The default scene is a 3D one to avoid confusing existing code that
       expects just 3D scenes. */
    CORRADE_COMPARE(importer->sceneCount(), 2);
    CORRADE_COMPARE(importer->defaultScene(), 1);

    /* Name mapping should work both ways */
    Int squareSolid = importer->objectForName("squareSolid");
    CORRADE_COMPARE_AS(squareSolid, 0, TestSuite::Compare::GreaterOrEqual);
    CORRADE_COMPARE(importer->objectName(squareSolid), "squareSolid");

    Containers::Optional<Trade::SceneData> scene = importer->scene(0);
    CORRADE_VERIFY(scene);
    CORRADE_VERIFY(scene->is2D());
    /* The 2D and 3D objects are interleaved (sorted by name) so both 2D and 3D
       object count is the total object count */
    CORRADE_COMPARE(importer->objectCount(), Containers::arraySize(Data));
    CORRADE_COMPARE(scene->fieldCount(), 3);

    CORRADE_VERIFY(scene->hasField(SceneField::Parent));
    CORRADE_COMPARE(scene->fieldSize(SceneField::Parent), 11);
    CORRADE_COMPARE(scene->parentFor(squareSolid), -1);

    CORRADE_VERIFY(scene->hasField(SceneField::Translation));
    CORRADE_COMPARE(scene->fieldSize(SceneField::Translation), 11);
    CORRADE_COMPARE(scene->transformation2DFor(squareSolid), Matrix3::translation({-1.5f, 3.0f}));

    CORRADE_VERIFY(scene->hasField(SceneField::Mesh));
    CORRADE_COMPARE(scene->fieldSize(SceneField::Mesh), 11);
    Containers::Array<Containers::Pair<UnsignedInt, Int>> meshesMaterials = scene->meshesMaterialsFor(squareSolid);
    CORRADE_COMPARE(meshesMaterials.size(), 1);
    CORRADE_COMPARE_AS(meshesMaterials.front().first(), 0, TestSuite::Compare::GreaterOrEqual);
    CORRADE_COMPARE_AS(meshesMaterials.front().first(), importer->meshCount(), TestSuite::Compare::Less);
    CORRADE_COMPARE(meshesMaterials.front().second(), -1);
    CORRADE_COMPARE(importer->meshName(meshesMaterials.front().first()), "squareSolid");
}

void PrimitiveImporterTest::scene3D() {
    Containers::Pointer<AbstractImporter> importer = _manager.instantiate("PrimitiveImporter");

    /* Due to checks in AbstractImporter, the importer has to manage opened
       state but other than that it doesn't matter what's opened */
    CORRADE_VERIFY(!importer->isOpened());
    CORRADE_VERIFY(importer->openData(nullptr));

    CORRADE_COMPARE(importer->objectCount(), Containers::arraySize(Data));

    /* The default scene is a 3D one to avoid confusing existing code that
       expects just 3D scenes. */
    CORRADE_COMPARE(importer->sceneCount(), 2);
    CORRADE_COMPARE(importer->defaultScene(), 1);

    /* Name mapping should work both ways */
    Int planeWireframe = importer->objectForName("planeWireframe");
    CORRADE_COMPARE_AS(planeWireframe, 0, TestSuite::Compare::GreaterOrEqual);
    CORRADE_COMPARE(importer->objectName(planeWireframe), "planeWireframe");

    Containers::Optional<Trade::SceneData> scene = importer->scene(1);
    CORRADE_VERIFY(scene);
    CORRADE_VERIFY(scene->is3D());
    /* The 2D and 3D objects are interleaved (sorted by name) so both 2D and 3D
       object count is the total object count */
    CORRADE_COMPARE(importer->objectCount(), Containers::arraySize(Data));
    CORRADE_COMPARE(scene->fieldCount(), 3);

    CORRADE_VERIFY(scene->hasField(SceneField::Parent));
    CORRADE_COMPARE(scene->fieldSize(SceneField::Parent), 25);
    CORRADE_COMPARE(scene->parentFor(planeWireframe), -1);

    CORRADE_VERIFY(scene->hasField(SceneField::Translation));
    CORRADE_COMPARE(scene->fieldSize(SceneField::Translation), 25);
    CORRADE_COMPARE(scene->transformation3DFor(planeWireframe), Matrix4::translation({1.5f, 9.0f, 0.0f}));

    CORRADE_VERIFY(scene->hasField(SceneField::Mesh));
    CORRADE_COMPARE(scene->fieldSize(SceneField::Mesh), 25);
    Containers::Array<Containers::Pair<UnsignedInt, Int>> meshesMaterials = scene->meshesMaterialsFor(planeWireframe);
    CORRADE_COMPARE(meshesMaterials.size(), 1);
    CORRADE_COMPARE_AS(meshesMaterials.front().first(), 0, TestSuite::Compare::GreaterOrEqual);
    CORRADE_COMPARE_AS(meshesMaterials.front().first(), importer->meshCount(), TestSuite::Compare::Less);
    CORRADE_COMPARE(meshesMaterials.front().second(), -1);
    CORRADE_COMPARE(importer->meshName(meshesMaterials.front().first()), "planeWireframe");
}

}}}}

CORRADE_TEST_MAIN(Magnum::Trade::Test::PrimitiveImporterTest)
