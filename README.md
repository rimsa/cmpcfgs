# cmpcfgs

Compare two CFGs with different strategies.
The CFGs can be produced either dynamically by CFGgrind (https://github.com/rimsa/CFGgrind)
or statically by dumpcfgs (https://github.com/rimsa/dumpcfgs).

## Build

    $ cmake .
    $ make -j4

## Usage

Compare two control flow graphs specifications using the simple strategy:

    $ ./cmpcfgs -s simple file1.cfgs file2.cfgs 

Compare two control flow graphs specifications using the specific strategy:

    $ ./cmpcfgs -s specific file1.cfgs file2.cfgs 
