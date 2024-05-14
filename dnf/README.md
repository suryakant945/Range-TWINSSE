# DNF Expression Evaluation
## Overview

This subdirectory contains the codes to evaluate DNF expressions of the form

$q = (w_1\land w_2)\lor(w_3\land w_4)$.

The main code has two top-level files - [`sse_setup.cpp`](./sse_setup.cpp) and [`sse_search.cpp`](./sse_search.cpp) responsible for executing the SSE setup and search routines, respectively. The [`Makefile`](./Makefile) for this codebase has the routines - `sse_setup` and `sse_search` that generate two executables `sse_setup` and `sse_search`,which execute the SSE setup and search routines, respectively. Additionally, there is a [`db_util.py`](./db_util.py) utility script that generates the keyword-frequency record necessary for query processing (useful for setting/verifying database parameters).

---

## Parameter Setting in Source Files

Please ensure that the following parameters are correctly set before building `sse_setup` and `sse_search`.

Set the following parameters in the database configuration file [here](../configuration/). A configuration file is already present for the supplied database.

```conf
<plain_database_file_path>
<number_of_threads_to_use_with_plain_database>
<number_of_keywords_in_the_plain_database>
<number_of_maximum_document_identifiers_for_a_keyword>
<Bloom_filter_size_as_a_power_of_2_value>
<number_of_bits_required_to_address_the_Bloom_filter>
.
.
.
```

Set `<plain_database_file_path>` to the actual database files path. See the main README.md in the upper directory for available databases and their details. Additional files are available in the Google Drive directory.

Set `<number_of_threads_to_use_with_plain_database>` to the number of threads to use. Since Bloom Filter hashes are individually computed using separate threads, we recommend a minimum of 24 threads to use. Please note that the number of hash functions used for Bloom filter indexing is the same as the number of threads used. In the current codebase, this is set automatically.

Set the values of `<number_of_keywords_in_the_plain_database>` the total number of keywords in the plain database. `<number_of_maximum_document_identifiers_for_a_keyword>` is the number of maximum ids where a keyword appears (or the maximum keyword frequency). This is used to reserve buffer; it need not be exact but should be larger than the actual max frequency.

Change `<Bloom_filter_size_as_a_power_of_2_value>` to the power of two just above the total number of unique meta-keyword-document-id pairs in the meta-database. For example, if there are 80901 unique meta-keyword-id pairs in the database, the power of two just above 80901 is 131072 = $2^{17}$. Hence, set this to 131072. This is used to calculate the total Bloom filter size.

Set `<number_of_bits_required_to_address_the_Bloom_filter>` to the number of bits to specify the range of addresses of the Bloom filter. The number of bits to consider is derived from the above `<Bloom_filter_size_as_a_power_of_2_value>` value. Here `<Bloom_filter_size_as_a_power_of_2_value>` is $2^{17}$, that requires 17 bits. Hence, 17 bits are extracted from the hash digest value to compute an index.

---
### Specific parameters in **sse_search.cpp**

This part is not fully automated yet. Thus, if required, change the following parameters.

```C++
std::string kw_freq_file = "db_kw_freq.csv";
std::string res_query_file = "./results/res_query.csv";
std::string res_id_file = "./results/res_id.csv";
std::string res_time_file = "./results/res_time.csv";
```

The `kw_freq_file` string holds the path to the keyword-frequency file generated by the [`db_util.py`](./db_util.py) script. The files are stored in the working directory. The other three files store the actual queries, result ids, and timing information for each query, respectively. These are overwritten each time the `sse_search` is executed.

```C++
unsigned int NUM_CLAUSE = 2;// Number of conjunctive clauses in DNF expression
```

This line defines the number of conjunctive clauses in DNF test vectors. The number of keywords in each conjunctive clause is set in the following line.

```C++
unsigned int n_q_kw = 2; //Number of keywords in a conjunctive clause
```

---

## Running an Experiment

### First Step

Run [`db_util.py`](./db_util.py) to generate the [`db_kw_freq.csv`](./db_kw_freq.csv) file, and necessary database information, including the number of keywords, total kw-id pairs etc. Change the database name to use a different database and regenerate the above file.

### Clear Redis

Clear Redis and other temporary databases

```bash
$ redis-cli
127.0.0.1:6379> flushall
127.0.0.1:6379> save
127.0.0.1:6379> quit
$ rm -rf eidxdb.csv
$ rm -rf bloom_filter.dat
```

You can also run `make clean_all` to delete the `eidxdb.csv` and `bloom_filter.dat` files along with the Redis database. This does not remove the result files.

### Set the Parameters

Obtain the database parameters from [`db_util.py`](./db_util.py) output and the source file parameters accordingly, as stated above.

### Build the Executables

Before doing this, ensure that all parameters have been set correctly and the paths are correct.

Execute the following command to generate `sse_setup` and `sse_search`.

```bash
make all
```

Both `sse_setup` and `sse_search` can be generated through individual Makefile rules.

```bash
make sse_setup
```

and 

```bash
make sse_search
```

The following command deletes the executable files. Generated databases, including Redis, Bloom filter and results are not deleted.

```bash
make clean 
```

The following command deletes the generated databases (`eidxdb.csv`, `bloom_filter.dat` and Redis database) and the Redis++ database. The generated result files are not removed).

```bash
make clean_all
```

### Run the Executables

Run the setup routine first.

```bash
./sse_setup
```

When setup is complete, execute the search,

```bash
./sse_search
```

The test vectors are automatically generated in this case from [`db_kw_freq.csv`](./db_kw_freq.csv) information, and a random DNF expression of the stated form is generated for searching. The results are stored in [`./results/`](./results/) subdirectory.

## Troubleshooting and Remarks

- Double-check the system parameters. If even one mismatch is there, the result would be incorrect.
- Check if the input files are being read correctly (otherwise, the program will terminate abruptly)
- While executing the `sse_setup`, the Bloom filter is written to disk at the end. This requires a bit of time. Please be patient with this while using large databases.
- While executing the `sse_search`, the Bloom filter is read into memory from the disk first. This requires a bit of time. Please be patient with this while using large databases.
- Timings can widely vary depending upon the system configuration and load (and the version of packages used).