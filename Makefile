# make parameters:
#    D=1        -- disable optimization, for easy debugging
#    COVERAGE=1 -- generate instrumented code
#    DOCTEST=1  -- use doctest instead of catch testing framework
#
# make targets:
#    all  ( default )  - build unittests
#    clean             - delete objects and executables
#    coverage          - run coverage test
#
DOCTEST?=1
CXXFLAGS=-g $(if $(D),-O0,-O3) -std=c++17 -Wall
CXXFLAGS+=-I .
CXXFLAGS+=-I /usr/local/include
LDFLAGS=-g
CDEFS?=$(if $(DOCTEST),-DWITH_DOCTEST,-DWITH_CATCH)
all: cmake

CMAKEARGS+=$(if $(D),-DCMAKE_BUILD_TYPE=Debug,-DCMAKE_BUILD_TYPE=Release)
CMAKEARGS+=$(if $(COV),-DOPT_COV=1)
CMAKEARGS+=$(if $(PROF),-DOPT_PROF=1)
CMAKEARGS+=$(if $(LIBCXX),-DOPT_LIBCXX=1)

CXXFLAGS+=$(if $(COVERAGE),-fprofile-instr-generate -fcoverage-mapping)
LDFLAGS+=$(if $(COVERAGE),-fprofile-instr-generate -fcoverage-mapping)

CXXFLAGS+=-DSUPPORT_POINTERS

unittests: $(notdir $(subst .cpp,.o,$(wildcard tests/*.cpp)))
	$(CXX) $(LDFLAGS) -o $@  $^

%.o: tests/%.cpp
	$(CXX) $(CXXFLAGS) $(CDEFS) -c $^ -o $@

cmake:
	cmake -B build . $(CMAKEARGS)
	$(MAKE) -C build $(if $(V),VERBOSE=1)

llvm:
	CC=clang CXX=clang++ cmake -B build . $(CMAKEARGS)
	$(MAKE) -C build $(if $(V),VERBOSE=1)


vc:
	"C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe" -G"Visual Studio 16 2019" -B build .
	"C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/MSBuild/Current/Bin/amd64/MSBuild.exe" build/cpputils.sln -t:Rebuild


clean:
	$(RM) unittests $(wildcard *.o) $(wildcard *.profdata *.profraw)
	$(RM) -r build CMakeFiles CMakeCache.txt CMakeOutput.log

# list of files checked for coverage
COVERAGEFILES=argparse.h arrayview.h asn1parser.h datapacking.h fhandle.h formatter.h fslibrary.h hexdumper.h mmem.h stringconvert.h string-join.h stringlibrary.h utfconvertor.h string-base.h string-parse.h string-split.h string-strip.h base64encoder.h utfcvutils.h
COVERAGEFILES+=HiresTimer.h asn1_presence.h asn1maker.h b32-alphabet.h b64-alphabet.h base32encoder.h certparser.h crccalc.h lowpass_filter.h mmfile.h string-lineenum.h templateutils.h xmlnodetree.h xmlparser.h

COVOPTIONS+=-show-instantiation-summary
#COVOPTIONS+=-show-functions
COVOPTIONS+=-show-expansions
COVOPTIONS+=-show-regions
COVOPTIONS+=-show-line-counts-or-regions


XCODETOOLCHAINS=$(shell xcode-select -p)/Toolchains/XcodeDefault.xctoolchain
coverage:
	llvm-profdata merge -o unittest.profdata default.profraw
	llvm-cov show ./build/unittests -instr-profile=unittest.profdata $(COVOPTIONS) $(COVERAGEFILES)

