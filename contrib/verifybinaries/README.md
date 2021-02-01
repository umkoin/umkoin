### Verify Binaries

#### Preparation:

Make sure you obtain the proper release signing key and verify the fingerprint with several independent sources.

```sh
$ gpg --fingerprint "Umkoin Core Builder"
pub   rsa4096 2019-06-19 [SC] [expires: 2021-03-18]
      A405 D7A3 AACB 75AB 1B8D  C18A 3B59 F7F9 F2B4 FCF0
uid           [ unknown] Umkoin Core Builder <umkoin@umkoin.org>
```

#### Usage:

This script attempts to download the signature file `SHA256SUMS.asc` from http://www.umkoin.org.

It first checks if the signature passes, and then downloads the files specified in the file, and checks if the hashes of these files match those that are specified in the signature file.

The script returns 0 if everything passes the checks. It returns 1 if either the signature check or the hash check doesn't pass. If an error occurs the return value is 2.


```sh
./verify.py umkoin-core-0.18.0
./verify.py umkoin-core-0.19.0
./verify.py umkoin-core-0.20.0
```

If you only want to download the binaries of certain platform, add the corresponding suffix, e.g.:

```sh
./verify.py umkoin-core-0.18.0-osx
./verify.py 0.19.0-linux
./verify.py umkoin-core-0.20.0-win64
```

If you do not want to keep the downloaded binaries, specify anything as the second parameter.

```sh
./verify.py umkoin-core-0.18.0 delete
```
