# TWINSSE

The disjunctive directory of this repository implements the range-SSE using TWINSSE. The range technique implemented is Logarithmic-BRC from the research paper: Demertzis, I., Papadopoulos, S., Papapetrou, O., Deligiannakis, A., & Garofalakis, M. (2016). Practical private range search revisited. In Proceedings of the 2016 International Conference on Management of Data (SIGMOD '16). Association for Computing Machinery, New York, NY, USA, 185–198. https://doi.org/10.1145/2882903.2882911


---

## Table of Contents

1. [Repository Organisation](#repoorg)
2. [System Requirements](#sysreq)
3. [Dependencies](#dependencies)
4. [Database Format](#dbformat)
5. [Primary Makefile](#primake)
6. [Steps to Run Experiments](#runexp)
7. [Troubleshooting and Remarks](#troubleremark)

---

## Repository Organisation <a name="repoorg"></a>

The repository is organised according to the TWINSSE experiments presented in the paper's main body covering experiments on different types of queries, storage and precision evaluation. The following directory tree shows the repository structure with subdirectories of individual experiments.

```
TWINSSE
|--README.md
|--requirements.system
|--Makefile
|____databases
|____configuration
|____conjunctive
|____disjunctive
|____cnf
|____dnf
|____precision
|____scripts
|____storage
|____test_vectors
```

---

## System Requirements <a name="sysreq"></a>

The TWINSSE framework requires heavy CPU-intensive computation and is intended for large multi-core server systems with the minimal additional workload. We recommend the following system specifications.

- Intel(R) Xeon v5 or higher server grade (Xeon Gold or higher) multi-core 64-bit CPUs

- At least 24-48 physical CPU cores (with dual thread) with a base frequency higher than 2 GHz. The codebase should work with a lower number of cores (8-16 physical cores), but the performance is likely to be degraded.

- 64-128 GB RAM or higher

- 256 GB PCIe Gen4 NVMe SSD or higher

- Ubuntu 18.04 LTS or 20.04 LTS 64 bit

- ISA support: SSE, SSE2, SSE3, SSE4, AVX2, AVX512, AES Intrinsics

- Minimum system load; if possible, no GUI and no other CPU/disk access-intensive task running concurrently


Please ensure the number of threads is appropriately set in the configuration file according to the number of available threads. Otherwise, there might be errors while running the executable(s). Also, double-check the values of associated parameters (many of which, in turn, depend on the number of threads, please see below).

---

## Dependencies <a name="dependencies"></a>

The following packages (development versions) must be installed with global linkage.

- POSIX pthread

- GNU GMP Multi-precision library with C++ wrappers (including MPFR)

- Redis base package, hiredis base package, and redis++ C++ wrapper

- Blake3 hash

- CryptoPP (or Crypto++)

[Pthread] `linux-libc-dev` v4.15.0-191.202, `libpthread-stubs0-dev` v0.3-4

[GMP] `libgmp-dev` v6.1.2, `libmpfr-dev` v4.0.1, `libmpfrc++-dev` v3.6.5

[Redis] `redis` v5:4.0.9, `redis-server` v5:4.0.9, `redis-tools` v5:4.0.9, `libhiredis-dev` v0.13.3-2.2

[redis++] Install from https://github.com/sewenew/redis-plus-plus

[Blake3] Supplied with the source code, invoke `make lib` inside **blake3** directory within a subproject directory

[CryptoPP] `libcrypto++-dev` v5.6.4, `libcrypto++-utils` v5.6.4, `libcrypto++6` v5.6.4

---

All dependencies (except __redis++__ and __Blake3__) can be installed by issuing the following command in the TWINSSE home directory. Note that it _requires sudo access_.

```bash
sudo apt update
cat requirements.system | xargs sudo apt -y install
```

The above dependencies must be installed before building and installing __redis++__.

**Please note that __redis++__ has to be installed manually after obtaining the source files from Github [here](https://github.com/sewenew/redis-plus-plus). This process requires _sudo_ access, which is a key reason to segregate __redis++__ installation from the above dependency configuration process. We are working on automating the __redis++__ installation process (combining with the dependency installation process) in a safe way.**

---

## Database Format <a name="dbformat"></a>

The parsed databases are stored in csv format (with `.dat` extension) in the following way (similar to a csv file, with each line ending with a comma).
```csv
kw0, id00,id01,id02,...,id0n,
kw1, id10,id11,id12,...,id1n,
kw2, id20,id21,id22,...,id2n,
.
.
.
```

This applies to the generated meta-keyword databases too.

```csv
mkw0, id00,id01,id02,...,id0m,
mkw1, id10,id11,id12,...,id1m,
mkw2, id20,id21,id22,...,id2m,
.
.
.
```

(All rows are not necessarily of the same length!)

All kw and id values are 4 byte hex values (this can be changed if required) obtained from the hash digest of actual keyword and id strings. This is a representation we chose specifically for experimental purposes. Please keep in mind where `.dat` and `.csv` extensions are used while modifying the source files. Otherwise, the files will not be read, and the program will terminate early.

---

## Primary Makefile <a name="primake"></a>

The following Makefile rules build and clean the subprojects and compile the necessary libraries for the subprojects. The compiled executables in each project subdirectory must be manually executed from the respective project subdirectory.

- __all__  - Builds conjunctive, disjunctive, dnf, cnf and precision executables

- __clean__ - Cleans conjunctive, disjunctive, dnf, cnf and precision executables

- __clean_all__ - Cleans conjunctive, disjunctive, dnf, cnf and precision executables, including generated database files and the Redis database

- __blake_lib__ - Builds blake3 lib for conjunctive, disjunctive, dnf, and cnf subprojects

---

Each subproject has its own Makefile or helper scripts and separate README file. Please browse through the following README files of the subprojects for more details.

- Databases [README](./databases/README.md)
- Configuration [README](./configuration/README.md)
- Conjunctive [README](./conjunctive/README.md)
- Disjunctive [README](./disjunctive/README.md)
- DNF [README](./dnf/README.md)
- CNF [README](./cnf/README.md)
- Precision [README](./precision/README.md)
- Storage [README](./storage/README.md)
- Scripts [README](./scripts/README.md)
- Test vectors [README](./test_vectors/README.md)

---

## Steps to Run Experiments <a name="runexp"></a>

_All commands below should be issued from TWINSSE home directory unless otherwise specified explicitly._

After installing all dependencies and setting up Redis++, the Blake3 library should be compiled (required by the experiments). To compile Blake3 library, invoke the following Makefile rule (this is pertaining to the primary Makefile in the TWINSSE home directory).
```bash
make blake_lib
```

Set the parameters for experiments in the configuration file to be used located in the [configuration](./configuration/) subdirectory. A configuration file for the database in the [databases](./databases/) subdirectory is already supplied with the codebase.

Set the parameters for the subprojects by navigating to each subproject directory and going through the instructions in README files. Generate the meta-keywords databases in project *subdirectories* (not from the top-level TWINSSE directory) by executing the following command in the `database` subdirectory (not the top-level [`databases`](./databases/) directory) within the project directory (_does not apply to conjunctive and DNF_). 

```C++
make all //within the database subdirectory
```

Set and double-check the plain and meta-keyword database parameters in the [configuration file](./configuration/) for the database in use (see associated README files).

If needed, generate the test vectors for particular experiments (a set of test vectors are already supplied inside the repository and respective project subdirectories). Instructions and Makefiles to generate test vectors are present in the project subdirectory and _database_ directory within each project subdirectory.

Double-check all parameters and file paths. Ensure all necessary databases (including test vectors) are generated/available.

Clean the projects by executing the following command in the TWINSSE directory. This clears the generated databases (including the temporary ones) and clears the Redis database too. This does not clear the result files generated in the experiments.

```bash
make clean_all
```

The user can manually flush the Redis server using the following sequence of commands (including commands to execute in the Redis CLI).

```bash
$ redis-cli
127.0.0.1:6379> flushall
127.0.0.1:6379> save
127.0.0.1:6379> quit
```
Alternatively, without going into ``redis-cli`` prompt.

```bash
$ redis-cli flushall
$ redis-cli save
```

Build all executables in all project subdirectories (precision, conjunctive, disjunctive, CNF and DNF).

```bash
make all
```

Go to each project subdirectory and execute `sse_setup` first and then `sse_search`.

```bash
./sse_setup
```

```bash
./sse_search
```

Details of each experiment and how to build each individually are available in respective project READMEs. Each project can be built by executing `make all` in the respective project subdirectory.

**Clean the project and Redis database before switching from one experiment to another!**

---

## Troubleshooting and Remarks <a name="troubleremark"></a>

The codebase has absolutely no error handling, does not follow specific production-grade software development practices, and may contain bugs. Please ensure all parameters are appropriately set, and the required files are available (especially those which need to be generated first) at the specific locations. Additionally, we present stripped-down versions of the main database for quick testing and debugging.

Points related to specific experiments.

- In conjunctive and DNF (or CNF) experiments, the result size might be zero (or empty) for a high percentage of the queries. This is due to the sparse nature of the database, where a very large number of keywords have very low frequency, resulting in zero intersection. The executable output is a correct search result with no id in the final result set.

- The search time can widely vary depending upon the system configuration, parameters and load.

- If an executable is abruptly terminated, check file paths and parameter values are properly set, especially if there is any segmentation fault or similar errors due to invalid memory accesses.

- If an executable is prematurely completed, check the specified file paths specifically. Probably the files are not read properly, and the executable is finished early due to a lack of data.

- If an executable is abruptly terminated, try to execute the program by logging the output to a log file in the following way.

```bash
./sse_setup > log.txt
```

Not sure yet why this happens, but this temporary fix seems to work. We are working on resolving it.

- Ensure that you have a sufficient number of cores as specified in the configuration file(s). Otherwise, you may receive pthread errors, and the program may abruptly terminate.

---

## References

OXT SSE Algorithm

```bib
Cash, David, et al. "Highly-scalable searchable symmetric encryption with support for boolean queries." Annual cryptology conference. Springer, Berlin, Heidelberg, 2013.
```

Blake3 Hash Development Repository

https://github.com/BLAKE3-team/BLAKE3


Crypto++ library

https://cryptopp.com/


Redis++

https://github.com/sewenew/redis-plus-plus


---

## Cite

The full version is available at [IACR ePrint](https://eprint.iacr.org/2022/1096/). To cite, use the following bib entry.

```bib
@misc{cryptoeprint:2022/1096,
      author = {Arnab Bag and Debadrita Talapatra and Ayushi Rastogi and Sikhar Patranabis and Debdeep Mukhopadhyay},
      title = {TWo-IN-one-SSE: Fast, Scalable and Storage-Efficient Searchable Symmetric Encryption for Conjunctive and Disjunctive Boolean Queries},
      howpublished = {Cryptology ePrint Archive, Paper 2022/1096},
      year = {2022},
      note = {\url{https://eprint.iacr.org/2022/1096}},
      url = {https://eprint.iacr.org/2022/1096}
}
```
