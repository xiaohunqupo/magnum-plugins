/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
                2020, 2021, 2022, 2023, 2024, 2025
              Vladimír Vondruš <mosra@centrum.cz>
    Copyright © 2019 Guillaume Jacquemin <williamjcm@users.noreply.github.com>

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

#include <string> /** @todo remove once AbstractImporter is <string>-free */
#include <Corrade/Containers/Array.h>
#include <Corrade/Containers/String.h>
#include <Corrade/Containers/StringStl.h> /** @todo remove once AbstractImporter is <string>-free */
#include <Corrade/TestSuite/Compare/Container.h>
#include <Corrade/TestSuite/Compare/Numeric.h>
#include <Corrade/TestSuite/Tester.h>
#include <Corrade/Utility/Path.h>
#include <Magnum/Audio/AbstractImporter.h>

#include "configure.h"

namespace Magnum { namespace Audio { namespace Test { namespace {

struct DrMp3ImporterTest: TestSuite::Tester {
    explicit DrMp3ImporterTest();

    void empty();

    void zeroSamples();

    void mono16();
    void stereo16();

    void openTwice();
    void importTwice();

    /* Explicitly forbid system-wide plugin dependencies */
    PluginManager::Manager<AbstractImporter> _manager{"nonexistent"};
};

DrMp3ImporterTest::DrMp3ImporterTest() {
    addTests({&DrMp3ImporterTest::empty,

              &DrMp3ImporterTest::zeroSamples,

              &DrMp3ImporterTest::mono16,
              &DrMp3ImporterTest::stereo16,

              &DrMp3ImporterTest::openTwice,
              &DrMp3ImporterTest::importTwice});

    /* Load the plugin directly from the build tree. Otherwise it's static and
       already loaded. */
    #ifdef DRMP3AUDIOIMPORTER_PLUGIN_FILENAME
    CORRADE_INTERNAL_ASSERT_OUTPUT(_manager.load(DRMP3AUDIOIMPORTER_PLUGIN_FILENAME) & PluginManager::LoadState::Loaded);
    #endif
}

void DrMp3ImporterTest::empty() {
    Containers::Pointer<AbstractImporter> importer = _manager.instantiate("DrMp3AudioImporter");

    Containers::String out;
    Error redirectError{&out};
    char a{};
    /* Explicitly checking non-null but empty view */
    CORRADE_VERIFY(!importer->openData({&a, 0}));
    CORRADE_COMPARE(out, "Audio::DrMp3Importer::openData(): failed to open and decode MP3 data\n");
}

void DrMp3ImporterTest::zeroSamples() {
    Containers::Pointer<AbstractImporter> importer = _manager.instantiate("DrMp3AudioImporter");

    /* No error should happen, it should just give an empty buffer back */
    {
        CORRADE_EXPECT_FAIL("dr_mp3 treats 0 frames as an error, because it returns 0 also for malloc failure and such.");
        CORRADE_VERIFY(importer->openFile(Utility::Path::join(DRMP3AUDIOIMPORTER_TEST_DIR, "zeroSamples.mp3")));
        if(!importer->isOpened()) return;
    }
    CORRADE_COMPARE(importer->format(), BufferFormat::Mono16);
    CORRADE_COMPARE(importer->frequency(), 44100);
    CORRADE_VERIFY(importer->data().isEmpty());
}

void DrMp3ImporterTest::mono16() {
    Containers::Pointer<AbstractImporter> importer = _manager.instantiate("DrMp3AudioImporter");
    CORRADE_VERIFY(importer->openFile(Utility::Path::join(DRMP3AUDIOIMPORTER_TEST_DIR, "mono16.mp3")));

    CORRADE_COMPARE(importer->format(), BufferFormat::Mono16);
    CORRADE_COMPARE(importer->frequency(), 44100);

    Containers::Array<char> data = importer->data();
    CORRADE_COMPARE(data.size(), 13824);
    CORRADE_COMPARE_AS(Containers::arrayCast<UnsignedShort>(data.slice(6720, 6724)),
        Containers::arrayView<UnsignedShort>({
            0x0332, 0x099c
        }), TestSuite::Compare::Container);
}

void DrMp3ImporterTest::stereo16() {
    Containers::Pointer<AbstractImporter> importer = _manager.instantiate("DrMp3AudioImporter");
    CORRADE_VERIFY(importer->openFile(Utility::Path::join(DRMP3AUDIOIMPORTER_TEST_DIR, "stereo16.mp3")));

    CORRADE_COMPARE(importer->format(), BufferFormat::Stereo16);
    CORRADE_COMPARE(importer->frequency(), 44100);

    Containers::Array<char> data = importer->data();
    CORRADE_COMPARE(data.size(), 13824*2);
    CORRADE_COMPARE_AS(Containers::arrayCast<UnsignedShort>(data.slice(9730, 9734)),
        Containers::arrayView<UnsignedShort>({
            0x99a6, 0x99b1
        }), TestSuite::Compare::Container);
}

void DrMp3ImporterTest::openTwice() {
    Containers::Pointer<AbstractImporter> importer = _manager.instantiate("DrMp3AudioImporter");

    CORRADE_VERIFY(importer->openFile(Utility::Path::join(DRMP3AUDIOIMPORTER_TEST_DIR, "mono16.mp3")));
    CORRADE_VERIFY(importer->openFile(Utility::Path::join(DRMP3AUDIOIMPORTER_TEST_DIR, "mono16.mp3")));

    /* Shouldn't crash, leak or anything */
}

void DrMp3ImporterTest::importTwice() {
    Containers::Pointer<AbstractImporter> importer = _manager.instantiate("DrMp3AudioImporter");
    CORRADE_VERIFY(importer->openFile(Utility::Path::join(DRMP3AUDIOIMPORTER_TEST_DIR, "mono16.mp3")));

    CORRADE_COMPARE(importer->format(), BufferFormat::Mono16);
    CORRADE_COMPARE(importer->frequency(), 44100);

    /* Verify that everything is working the same way on second use */
    {
        Containers::Array<char> data = importer->data();
        CORRADE_COMPARE(data.size(), 13824);
        CORRADE_COMPARE_AS(Containers::arrayCast<UnsignedShort>(data.slice(6720, 6724)),
            Containers::arrayView<UnsignedShort>({
                0x0332, 0x099c
            }), TestSuite::Compare::Container);
    } {
        Containers::Array<char> data = importer->data();
        CORRADE_COMPARE(data.size(), 13824);
        CORRADE_COMPARE_AS(Containers::arrayCast<UnsignedShort>(data.slice(6720, 6724)),
            Containers::arrayView<UnsignedShort>({
                0x0332, 0x099c
            }), TestSuite::Compare::Container);
    }
}

}}}}

CORRADE_TEST_MAIN(Magnum::Audio::Test::DrMp3ImporterTest)
