# Fluent Bit Plugin pgsql

## Build Pluging
The following repository provides a plugin to send logs to a PostgreSQL database.

### Requirements

- [Fluent Bit](https://fluentbit.io) Source code, version >= 1.2
- C compiler: GCC or Clang
- CMake3

### Getting Started

In the following steps we will build the plugin. As a first step get into the _build/_ directory:

```bash
$ cd build/
```

Now we will provide CMake (our build system) two important variables:

- FLB\_SOURCE: absolute path to source code of Fluent Bit.
- PLUGIN\_NAME: _directory_ name of the project that we aim to build. Note that any plugin name must have it proper prefix as the example mentioned above.

Assuming that Fluent Bit source code is located at /tmp/fluent-bit, we will run CMake with the following options:

```bash
$ cmake -DFLB_SOURCE=/tmp/fluent-bit -DPLUGIN_NAME=out_pgsql ../
```

then type 'make' to build the plugin:

```
$ make
Scanning dependencies of target flb-out_pgsql
[ 50%] Building C object out_pgsql/CMakeFiles/flb-out_pgsql.dir/pgsql.c.o
[100%] Linking C shared library ../flb-out_pgsql.so
[100%] Built target flb-out_pgsql
```

If you query the content of the current directory you will find the new shared library created:

```
$ ls -l *.so
-rwxr-xr-x 1 zeus zeus 37464 Aug  9 13:52 flb-out_pgsql.so
```

now it can be loaded from Fluent Bit through the [plugins configuration](https://github.com/fluent/fluent-bit/blob/master/conf/plugins.conf) file.

# Configure

The pgsql plugin allows to send data from fluentbit to a PostgreSQL instances 9.4 or above. To store the date we use the json type
so make sure that your PostgreSQL instance supports this type.

## Configuration Parameters

| Key           | Description                       | Default      |
| :--           | :---                              | :--          |
| Host          | Hostname of PostgreSQL instance   | 127.0.0.1    |
| Port          | PostgreSQL port                   | 5432         |
| Database      | Database name to connect          | fluentbit    |
| Table         | Table name where to store data    | fluentbit    |
| User          | PostgreSQL username               | current user |
| Password      | Password of PostgreSLQ username   |              |
| Timestmap_Key | Key to store the record timestmap | date         |
|               |                                   |              |

## Configuration Example

In your main configuration file add the following section:

```text
[OUTPUT]
    Name  pgsql
    Match *
    Host 172.17.0.2
    Port 5432
    Database fluentbit
    Table fluentbit
    User postgres
    Password YourCrazySecurePassword
    Timestamp_Key timestamp

```


# License

This program is under the terms of the [Apache License v2.0](http://www.apache.org/licenses/LICENSE-2.0).
