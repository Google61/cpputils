#include "unittestframework.h"

#include <signal.h>

#include "fhandle.h"
#include "fhandle.h"
#include <vector>

// TODO: this test just crashes directly on windows.
TEST_CASE("filehandle") {
#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
    SECTION("construct") {

        // check invalid fh
        CHECK_THROWS( filehandle{-1} );

        // will throw on close 
        CHECK_THROWS( filehandle{99}.close() );

        // check default constructor
        CHECK( filehandle{}.empty() );
        CHECK_THROWS( filehandle{}.fh() );

        // copy constructor
        CHECK( filehandle{filehandle{}}.empty() );
        CHECK_THROWS( filehandle{filehandle{99}}.close() );

        filehandle f;
        filehandle g;

        // check assignment
        CHECK_NOTHROW( g = f );
        CHECK_THROWS( f = -1 );
        CHECK_NOTHROW( f = 99 );

        CHECK( f.fh() == 99 );
        CHECK( 99 == f.fh() );

        CHECK_NOTHROW( g = f );
        CHECK( 99 == g.fh() );

        CHECK_THROWS( f.close() ); // because 99 is not a valid fd.
        CHECK_NOTHROW( g.close() );  // now g is also already closed
    }
#ifndef _WIN32
// TODO - create tests which work on windows as well.
    SECTION("pair") {
        filehandle f;
        filehandle g;

        int fpair[2];
        ::pipe(fpair);

        f = fpair[0];
        g = fpair[1];

        CHECK( g.write("abc", 3) == 3 );

        CHECK( f.read(3) == std::vector<uint8_t>{ 'a', 'b', 'c' } );

        g.close();
        CHECK( f.read(3).empty() );
    }
    SECTION("closereader") {

        int fpair[2];
        ::pipe(fpair);

        filehandle f = fpair[0];

        if (1) {
            filehandle g = fpair[1];

            // data can be written and read
            CHECK( g.write("abc", 3) == 3 );
            CHECK( f.read(3) == std::vector<uint8_t>{ 'a', 'b', 'c' } );

            // pipe is unidirectional
#ifndef __FreeBSD__
            // todo - why does this not throw on freebsd?
            CHECK_THROWS( f.write("abc", 3) );
            CHECK_THROWS( g.read(3) );
#endif
            // leaving scope closes 'g'
        }

        if (1) {
            // 'g' is now already closed
            filehandle g = fpair[1];

            // data can no longer be written and read
            CHECK_THROWS( g.write("abc", 3) );  // writing fails, because g is closed.
            // reading returns EOF / empty
            CHECK( f.read(3).empty() );

            // will throw since fh is already closed.
            // need to explicitly close, to avoid an exception in
            // the destructor when leaving scope.
            CHECK_THROWS( g.close() );
        }
    }
    SECTION("closewriter") {

        int fpair[2];
        ::pipe(fpair);

        filehandle g = fpair[1];

        if (1) {
            filehandle f = fpair[0];

            // data can be written and read
            CHECK( g.write("abc", 3) == 3 );
            CHECK( f.read(3) == std::vector<uint8_t>{ 'a', 'b', 'c' } );

            // pipe is unidirectional
#ifndef __FreeBSD__
            // todo - why does this not throw on freebsd?
            CHECK_THROWS( f.write("abc", 3) );
            CHECK_THROWS( g.read(3) );
#endif
            // leaving scope closes 'f'
        }

        if (1) {
            // 'f' is now already closed
            filehandle f = fpair[0];

            // data can no longer be written and read
            CHECK_THROWS( g.write("abc", 3) );  // writing will sigpipe

            CHECK_THROWS( f.read(3) ); // should fail because f is closed.

            // will throw since fh is already closed.
            // need to explicitly close, to avoid an exception in
            // the destructor when leaving scope.
            CHECK_THROWS( f.close() );
        }
    }
    SECTION("copyfh") {

        int fpair[2];
        ::pipe(fpair);

        filehandle f = fpair[0];
        filehandle h = fpair[1];

        if (1) {
            // 'g' is a copy of 'h'
            filehandle g = h;

            // data can be written and read
            CHECK( g.write("abc", 3) == 3 );
            CHECK( f.read(3) == std::vector<uint8_t>{ 'a', 'b', 'c' } );

            // leaving scope does not close 'g', since it is a copy of 'h'.
        }

        if (1) {
            // 'g' is still open
            filehandle g = h;

            // data can still be written
            CHECK( g.write("abc", 3) == 3 );
            CHECK( f.read(3) == std::vector<uint8_t>{ 'a', 'b', 'c' } );
        }

    }
    SECTION("overwritewriter") {
        int fpair[2];
        ::pipe(fpair);

        filehandle f = fpair[0];
        filehandle g = fpair[1];

        int fpair2[2];
        ::pipe(fpair2);

        // overwrite 'g', closing the read end of the pipe.
        g = fpair2[1];

        // reading returns EOF
        CHECK( f.read(3).empty() );
    }
    SECTION("overwritereader") {
        int fpair[2];
        ::pipe(fpair);

        filehandle f = fpair[0];
        filehandle g = fpair[1];

        int fpair2[2];
        ::pipe(fpair2);

        // overwrite 'f', closing the read end of the pipe.
        f = fpair2[0];

        // reading returns EOF
        CHECK_THROWS( g.write("abc", 3) );  // writing fails, because f is closed.
    }
#endif
    // TODO
    //  - read(first,last)
    //  - read(first,size)
    //  - read(vector)
    //  - read(span)
    //  - read(range)
    // 
}
