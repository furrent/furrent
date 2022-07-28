# Furrent
Furrent is a tiny BitTorrent client

## Building
The requirements are:
- libcurl

When you have them set up, building Furrent is just:
```shell
$ mkdir build
$ cd build
$ cmake ..
$ make furrent
# A "furrent" executable should have appeared in the current directory.
# To copy it in the root bin/ directory also do:
$ make install
```
The instructions that follow assume that you have built Furrent and are in the `build/` directory.

## Documentation
You can build the documentation like so:
```shell
$ make doc
# A "doc" folder should have appeared in the current directory.
# To copy it in the root doc/ directory also do:
$ make install
```
Note that you will need to have Doxygen installed on your system.

## Testing
You can run all tests like so:
```shell
$ make furrent_test
$ make test
```
Additionally, if you have Valgrind installed on your system, you can run the tests using that with:
```shell
$ ctest -C valgrind
```

## Coverage
To enable coverage support, add the `-DCOVERAGE=ON` flag when running `cmake`. Then build the `furrent` or `furrent_test`
executables. From now on you can use `make coverage` to spawn a `coverage.html` report. Note that you will need to
run the executables that you're interested in before you can see any actual coverage result (and also run `make coverage`
every time you want to refresh the results).

This step requires you to have `gcov` and `gcovr` installed on your system.

## Sanitizer
We're using the undefined behaviour runtime sanitizer that will print a message to the console in the unfortunate
event that Furrent runs into any UB. To enable it, add the `-DUSAN=ON` flag when running `cmake`.

## Tidy and format
We're using the `clang-format` formatter to maintain a uniform code style and the `clang-tidy` static analyzer (linter)
to catch the most common programming errors right in the editor.

`.clang-format` is taken from the Google convention while `.clang-tidy` is a
[dump](https://gist.github.com/ArnaudValensi/0d36639fb84b80ee57d0c3c977deb70e) from a recent version of the CLion IDE.
