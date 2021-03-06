
/*
 * Copyright (c) 2011 LinBox
 * Written by bds.
 *
 * ========LICENCE========
 * This file is part of the library LinBox.
 *
 * LinBox is free software: you can redistribute it and/or modify
 * it under the terms of the  GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * ========LICENCE========

 */
/** @file tests/checker.C
@brief script to run LinBox tests

Checker is compiled and run by the check macro invoked by "make fullcheck" in the top source dir or in tests/.

Run without args, checker prints an explanation of command line options, then proceeds with default test behaviour.  "checker -h" just prints the options.

Each LinBox test gets a line below.  A LinBox unit/regresssion test has a default behaviour -- when there are no command line file names -- in which nothing is written to any output stream and the return value is 0 for a passing test, nonzero for any failure.

The current convention is that (1) linbox' check macro, checker.C, runs the tests with no command line parameters at all, and (2) if there is a command line file name, verbose diagnostic output is written to that file and more terse output may be written to standard output streams.  The second feature is intended to assist debugging with individual tests.

In future, command line arguments could be used in checking to vary matrix sizes and other parameters in check runs.

There should be a 1-1 correspondence between files tests/test-*.C and calls
here to build_n_run() or no_build_n_run() (possibly commented out).
*/

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
//#include <iomanip>
using namespace std;

#include "fflas-ffpack/fflas-ffpack-config.h"
#include "../linbox/linbox-config.h"

struct counts {
	int      pass;
       	int buildfail;
	int   runfail;
	int   skipped;
	counts() :
		pass(0), buildfail(0), runfail(0), skipped(0)
	{}
	int total()
	{
	       	return pass + buildfail + runfail;
       	}
};

void build_n_run(string s, counts& cnt, int flag = 2, string r = "") ;
void no_build_n_run(string s, counts& cnt, int flag = 2, string r = "") ;

int main(int argc, char* argv[])
{
	int f = 0; // f = 1 ==> force recompile all tests.
	int flag = 1; // verbosity flag.

	bool check = false;// true for install time check
	int arg;
	for (arg = 1; arg < argc && argv[arg][0] == '-'; ++ arg)
		switch (argv[arg][1]) {
			case 'r': f = 1; break;
			case 'f': flag = atoi(argv[++arg]); break;
			case 'c': check = true; break;
			default:
		cout << "usage: " << argv[0] << " [-r] [-f n] [-c] [test]*" << endl;
		cout << "  -f 0 for a summary: only 4 lines printed." << endl;
		cout << "  -f 1 for default: also one line per test." << endl;
		cout << "  -f 2 for warnings: shows subtest failures." << endl;
		cout << "  -f 3 for errors: also any build and run output for error cases." << endl;
		cout << "  -f 4 for verbose: also any build and run output for each test." << endl;
		cout << "  -r forces rebuild of all tests." << endl;
		cout << "  -c conducts an install time check (a few selected tests only)." << endl;
		cout << "  if any [test] is present, it is built and run, otherwise all tests are processed." << endl;
		return 0;
	}
	flag = flag+5*f; // clean up later
#if 0
       	else {
		if (argv[1][0] == 's') flag = 0; // summary
		if (argv[1][0] == 'd') flag = 1; // default
		if (argv[1][0] == 'w') flag = 2; // warnings
		if (argv[1][0] == 'e') flag = 3; // errors
		if (argv[1][0] == 'v') flag = 3; // verbose
		if (argc > 2 && argv[2] == "rebuild") flag += 5; // force rebuilds
	}
#endif
	// the setup
	system("rm -f checkdata");
#if 0
	string compiler;
	//system("make -n ");
	system("make -n checkerdata > checkdata");
	ifstream fin("checkdata", ifstream::in);
	do {getline(fin, compiler);}
	while (fin && compiler.find("something") == compiler.npos);
	cout << compiler << endl << endl;
	system("rm -f checkdata");
#endif

	// the tests
	counts counter;

	// if only specific test(s) specified
	if (arg < argc) {
		for (; arg < argc; ++arg)
			build_n_run(argv[arg], counter , flag);
		return counter.buildfail || counter.runfail ? -1 : 0;
	}

/*
Each test gets a line below.  A LinBox unit/regresssion test has a default behaviour -- when there are no command line file names -- in which nothing is written to any output stream and the return value is 0 for a passing test, nonzero for any failure.

The current convention is that (1) linbox' check macro, checker.C, runs the tests with no command line parameters at all, and (2) if there is a command line file name, verbose diagnostic output is written to that file and more terse output may be written to standard output streams.  The second feature is intended to assist debugging with individual tests.

In future, command line arguments could be used in checking to vary matrix sizes and other parameters in check runs.

There should be a 1-1 correspondence between files tests/test-*.C and calls
here to build_n_run or no_build_n_run (possibly commented out).
Thus "ls test-*.C |wc" and "grep test- checker.C |grep \
build |wc" should yield the same number of lines.
*/

  if (check) {
	build_n_run("test-charpoly",                     counter , flag);//, "intermittent inf loop, bb or cp responsible?");
	build_n_run("test-cra",                          counter , flag);
	build_n_run("test-det",                          counter , flag);
	build_n_run("test-frobenius",                    counter , flag);
	build_n_run("test-rank",                         counter , flag);
	build_n_run("test-qlup",                         counter , flag);
	build_n_run("test-solve",                        counter , flag);
	build_n_run("test-nullspace",                    counter , flag);
	build_n_run("test-rat-solve",     counter , flag); // "infinite loop");
	build_n_run("test-rat-minpoly",   counter , flag); // "intermittent failures");
	build_n_run("test-rational-solver",              counter , flag);
	build_n_run("test-smith-form",            counter , flag);
	build_n_run("test-smith-form-adaptive",   counter , flag);
	build_n_run("test-smith-form-iliopoulos", counter , flag);
	build_n_run("test-smith-form-binary",            counter , flag);
	return counter.buildfail || counter.runfail ? -1 : 0;
  } else {
//BASIC_TESTS
	build_n_run("test-bitonic-sort",                 counter , flag);
	build_n_run("test-blackbox-block-container",     counter , flag);
	build_n_run("test-blas-domain",                  counter , flag);
	build_n_run("test-block-ring",                   counter , flag);
	no_build_n_run("test-block-wiedemann",              counter , flag, "bds tracking down BlasMatrix bug");
	no_build_n_run("test-bmseq",                     counter , flag, "under development by George Yuhasz");
	build_n_run("test-butterfly",                    counter , flag);
	build_n_run("test-charpoly",                     counter , flag);//, "intermittent inf loop, bb or cp responsible?");
	build_n_run("test-commentator",                  counter , flag);
	build_n_run("test-companion",                    counter , flag);
	build_n_run("test-cra",                          counter , flag);
	build_n_run("test-cradomain",                    counter , flag,"intermittent failure, as in 1 every 10-20");
	build_n_run("test-dense",                        counter , flag);
	build_n_run("test-det",                          counter , flag);
	build_n_run("test-diagonal",                     counter , flag);
	build_n_run("test-dif",                          counter , flag);
	build_n_run("test-direct-sum",                   counter , flag);
	build_n_run("test-dyadic-to-rational",           counter , flag, "bds responsible");
	build_n_run("test-ffpack",                       counter , flag);
	build_n_run("test-frobenius",                    counter , flag);
	build_n_run("test-getentry",                     counter , flag);
	build_n_run("test-gf2",                          counter , flag);
	build_n_run("test-gmp-rational",                 counter , flag);
	build_n_run("test-hilbert",                      counter , flag);
	build_n_run("test-hom",                          counter , flag);
	build_n_run("test-inverse",                      counter , flag);
	build_n_run("test-isposdef",                     counter , flag);
	build_n_run("test-ispossemidef",                 counter , flag);
	build_n_run("test-last-invariant-factor",        counter , flag);
	build_n_run("test-matrix-domain",                counter , flag);
	build_n_run("test-matrix-stream",                counter , flag);
	no_build_n_run("test-mg-block-lanczos",             counter , flag, "fails");
	build_n_run("test-minpoly",                      counter , flag);
	build_n_run("test-modular",                      counter , flag);
	build_n_run("test-modular-balanced-int",         counter , flag);
	build_n_run("test-modular-balanced-float",       counter , flag);
	build_n_run("test-modular-balanced-double",      counter , flag);
	build_n_run("test-modular-byte",                 counter , flag);
	build_n_run("test-modular-double",               counter , flag);
	build_n_run("test-modular-float",                counter , flag);
	build_n_run("test-modular-int",                  counter , flag);
	build_n_run("test-modular-short",                counter , flag);
	build_n_run("test-moore-penrose",                counter , flag);
	build_n_run("test-nullspace",                    counter , flag);
	build_n_run("test-PID-integer",                  counter , flag);
	build_n_run("test-qlup",                         counter , flag);
	build_n_run("test-randiter-nonzero",             counter , flag);
	build_n_run("test-rank",                         counter , flag);
	build_n_run("test-rational-matrix-factory ",     counter , flag);
	build_n_run("test-rational-reconstruction-base", counter , flag);
	build_n_run("test-rat-charpoly",                 counter , flag);//, "infinite loop, cp responsible?");
	build_n_run("test-scalar-matrix",                counter , flag);
	build_n_run("test-smith-form-binary",            counter , flag);
	build_n_run("test-solve",                        counter , flag);
	build_n_run("test-sparse",                       counter , flag);
	build_n_run("test-subiterator",                  counter , flag);
	build_n_run("test-submatrix",                    counter , flag);
	build_n_run("test-subvector",                    counter , flag);
	build_n_run("test-sum",                          counter , flag);
	build_n_run("test-rational-solver",              counter , flag);
	build_n_run("test-trace",                        counter , flag);
	build_n_run("test-triplesbb",                    counter , flag);
	build_n_run("test-unparametric-field",           counter , flag); //has been useful in num/sym.
	build_n_run("test-vector-domain",                counter , flag);
	build_n_run("test-zero-one",                     counter , flag);

#if __LINBOX_HAVE_GIVARO
//	if (flag > 0) cout << "	Givaro tests" << endl;
	build_n_run("test-givaro-fields",    counter , flag, "may fail on small fields because of supposed non-randomness or failure to find a non trivial element");
	//build_n_run("test-givaro-zpz", counter , flag, "superceded by test-givaro-fields");
	//build_n_run("test-givaro-zpzuns", counter , flag, "superceded by test-givaro-fields");
	build_n_run("test-rat-solve",     counter , flag); // "infinite loop");
	build_n_run("test-rat-minpoly",   counter , flag); // "intermittent failures");
#else
	if (flag > 0) cout << "	not doing Givaro dependent tests" << endl;
	cout << "Configuration problem?  __LINBOX_HAVE_GIVARO is not set, but LinBox requires Givaro" << endl;
	counter.skipped += 4;
#endif

#if __FFLASFFPACK_HAVE_LAPACK
	if (flag > 0) cout << "	Lapack dependent tests" << endl;
	build_n_run("test-rational-solver-adaptive", counter, flag);
	// needs output cleanup.  Resolve whether a benchmark or a test.
	build_n_run("test-solve-nonsingular", counter, flag, "BY responsible");
#else
	if (flag > 0) cout << "	not doing Lapack dependent tests" << endl;
	no_build_n_run("test-rational-solver-adaptive", counter, flag);
	// needs output cleanup.  Resolve whether a benchmark or a test.
	no_build_n_run("test-solve-nonsingular", counter, flag, "BY responsible");
#endif

#if __LINBOX_HAVE_NTL
	if (flag > 0) cout << "	NTL dependent tests" << endl;
	build_n_run("test-ntl-hankel",            counter , flag);
	build_n_run("test-ntl-lzz_p",             counter , flag);
	build_n_run("test-ntl-toeplitz",          counter , flag);
	build_n_run("test-ntl-sylvester",         counter , flag);
	no_build_n_run("test-ntl-RR",                counter , flag, "floating point equality");
	build_n_run("test-ntl-ZZ_p",              counter , flag);
	build_n_run("test-toeplitz-det",          counter , flag);
	build_n_run("test-smith-form",            counter , flag);
	build_n_run("test-smith-form-adaptive",   counter , flag);
	build_n_run("test-smith-form-iliopoulos", counter , flag);
#else
	if (flag > 0) cout << "	not doing NTL dependent tests" << endl;
	no_build_n_run("test-ntl-hankel",            counter , flag);
	no_build_n_run("test-ntl-lzz_p",             counter , flag);
	no_build_n_run("test-ntl-toeplitz",          counter , flag);
	no_build_n_run("test-ntl-RR",                counter , flag, "floating point equality");
	no_build_n_run("test-ntl-sylvester",         counter , flag);
	no_build_n_run("test-ntl-ZZ_p",              counter , flag);
	no_build_n_run("test-toeplitz-det",          counter , flag, "can we have non NTL version?");
	no_build_n_run("test-smith-form",            counter , flag);
	no_build_n_run("test-smith-form-adaptive",   counter , flag);
	no_build_n_run("test-smith-form-iliopoulos", counter , flag);
#endif

#if __LINBOX_HAVE_LIDIA
	if (flag > 0) cout << "	Lidia dependent tests" << endl;
	build_n_run("./test-lidia-gfq", counter, flag);
#else
	if (flag > 0) cout << "	not doing Lidia dependent test" << endl;
	no_build_n_run("test-lidia-gfq", counter, flag);
#endif

#if __LINBOX_HAVE_ATLAS
	if (flag > 0) cout << "	Atlas dependent tests" << endl;
	no_build_n_run("./test-optimization", counter, flag, "not a test");
#else
	if (flag > 0) cout << "	not doing Atlas dependent test" << endl;
	no_build_n_run("test-optimization", counter, flag, "not a test");
#endif

	// tests of "doubles as a field" which doesn't adhere to LinBox field or ring specs.
	if (flag > 0) cout << "	Noncompliant tests" << endl;
	build_n_run("test-param-fuzzy", counter, flag);

#if 1
	if (flag > 0) cout << "	Tests requiring further development" << endl;
	build_n_run("test-smith-form-local", counter, flag, "bds"); //"intermittent failures");
	no_build_n_run("test-ftrmm", counter, flag, "bb/cp");

	if (flag > 0) cout << "	Immature tests" << endl;
	no_build_n_run("test-quad-matrix", counter, flag, "half baked, bds responsible"); no_build_n_run("test-dense-zero-one", counter, flag, "half baked, bds responsible"); build_n_run("test-zo", counter, flag, "half baked, BY responsible");
	// test-integer-tools -- there is no test-integer-tools.C file
	// no one has taken these on.
	no_build_n_run("test-la-block-lanczos",counter,flag,"segfaults");
#endif

#if 0
	if (flag > 0) cout << "	Non tests" << endl;
	no_build_n_run("test-template", counter, flag, "yup, model for tests, not an actual test");
	// test-common.C is included in test-common.h,
	//  but could be separately compiled (no templates).
	no_build_n_run("test-common", counter, flag, "not a test");
	no_build_n_run("test-fields", counter, flag, "deprecated");
	no_build_n_run("test-matrix", counter, flag, "deprecated?");
	no_build_n_run("test-image-field", counter, flag, "deprecated");

//BENCHMARKS =
	build_n_run("benchmark-fields", counter, flag, "not a test");
	build_n_run("benchmark-blas-domain", counter, flag, "not a test");

# I put test-gssv_rank in hmrg:~saunders/gen_superlu.  It is not linbox until and unless it is made to work
# test-gssv is an ntl test

# no explicit test needed, I guess...
#        Transpose is tested in test-triplesbb
#        Compose is tested in test-butterfly

#endif
  }
	// the summary
	cout << "--------------------------------------------------------------------" << endl;
	if (counter.buildfail || counter.runfail)
		cout << "Of " << counter.total() << " tests, "
			<< counter.buildfail << " failed to compile and "
			<< counter.runfail << " failed at runtime."
			<< "  (" << counter.skipped << " skipped tests.)" << endl;
	else
		cout << endl << "All " << counter.total() << " tests pass."
			<< "  (" << counter.skipped << " skipped tests.)" << endl;
	cout << "  File tests/checkdata contains any output from the builds and runs." << endl;
	cout << "--------------------------------------------------------------------" << endl;

	return counter.buildfail || counter.runfail ? -1 : 0;
}

void no_build_n_run(string s, counts& cnt, int flag, string r)
{
	if (flag >= 5) flag -= 5;
	if (flag > 0) {
		cout.width(35); cout << left << s;
		if (r.length() > 0) cout << "skipped, " << r << "." << endl;
		else cout << "skipped." << endl;
	}
	cnt.skipped++;
}

void build_n_run(string s, counts& cnt, int flag, string r)
{
// ignore r.
	string cmd;
	if (flag >= 5) { // force build
		flag -= 5;
		cmd = "touch " + s + ".C";
		system(cmd.c_str());
	}
	if (flag >= 1) {
		cout.width(35); cout << left << s;
		cout << "build"; cout.flush();
	}
	cmd = "make " + s + " 2>> checkdata >> checkdata";
	int status = system(cmd.c_str());
	if (status == 2) goto abort; // valid on at least one platform.
	if (status != 0) { // build failure
		if (flag >= 1) cout << " FAILS" << endl;
		if (flag >= 3) system ("cat checkdata; rm checkdata");
		cnt.buildfail++;
		// if (flag == 2) could grep for first "error" in compiler output

	}
       	else { // build success
		if (flag >= 1) { cout << "\b\b\b\b\b  run"; cout.flush(); }
		std::ostringstream prog ;
		prog << "./" << s ;
		status = system(prog.str().c_str());
		if (status == 2) goto abort; // valid on at least one platform.
		if (status != 0) {
			if (flag >= 1) cout << " FAILS" << endl;
			if (flag >= 3) system ("cat checkdata; rm checkdata");
			cnt.runfail++;
		}
	       	else {
			if (flag >= 1) cout << "\b\b\b\bOK  " << endl;
			if (flag >= 2) system ("grep \"warn\" checkdata; rm checkdata");
			cnt.pass++;
			//cerr << "ok" << endl;
		}
	}
	if (flag >= 3) system("rm -f checkdata");
	return;
abort:
	cout << endl << "Interrupted, aborting." << endl;
	exit(-1);
}


// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

