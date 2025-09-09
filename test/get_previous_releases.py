#!/usr/bin/env python3
#
# Copyright (c) 2018-present The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
#
# Download or build previous releases.
# Needs curl and tar to download a release, or the build dependencies when
# building a release.

import argparse
import contextlib
from fnmatch import fnmatch
import hashlib
import os
from pathlib import Path
import platform
import re
import shutil
import subprocess
import sys
import time
import urllib.request
import zipfile

SHA256_SUMS = {
"a605473e985a0cc372c96cc7bc9f0b8c76bbdaa9c37cc169706c594c7abc62b3": "umkoin-0.19.1-aarch64-linux-gnu.tar.gz",
"908ea674e22400b0119e029680df8f5f57cbe07d18900501dd349ca7554077f8": "umkoin-0.19.1-arm-linux-gnueabihf.tar.gz",
"a82668eeb603c182377affbc8c29c0b352f73426ce675471b99ed6959bbd91d6": "umkoin-0.19.1-i686-pc-linux-gnu.tar.gz",
"20005082dd17a717f0d79ee234f7fd5bdff489e55895b7084b357eec4361bd26": "umkoin-0.19.1-osx64.tar.gz",
"ee3dddecfa5c858856f8006342c484b668f34d5d41f8f1d3237fbf3626a4f075": "umkoin-0.19.1-riscv64-linux-gnu.tar.gz",
"c8b1d803c03e52538d62759caed010e52924c5c0a4f4cf840199c200399dc628": "umkoin-0.19.1-x86_64-linux-gnu.tar.gz",

"ad356f577f3fffe646ffbe73bd3655612f794e9cc57995984f9f88581ea6fbb3": "umkoin-0.20.1-aarch64-linux-gnu.tar.gz",
"2ef0bf4045ecdbd4fc34c1c818c0d4b5f52ca37709d71e8e0388f5272fb17214": "umkoin-0.20.1-arm-linux-gnueabihf.tar.gz",
"6f6411c8409e91b070f54edf76544cdc85cfd2b9ffe0dba200fb68cddb1e3010": "umkoin-0.20.1-osx64.tar.gz",
"e55c32e91800156032dcc2ff9bc654df2068b46bab24e63328755a1c2fd588e2": "umkoin-0.20.1-riscv64-linux-gnu.tar.gz",
"2430e4c813ea0de28ef939170e05a36eaa1d2031589abe3b32491d5835f7b70e": "umkoin-0.20.1-x86_64-linux-gnu.tar.gz",

"25ea92c1318f062da3cc6eee34226a14589afec553dfdc8aef60fc359a55f773": "umkoin-0.21.0-aarch64-linux-gnu.tar.gz",
"c24629badfdaa8cfc357e834c8814480f414cb28fc5f974cbeb14433f050ff19": "umkoin-0.21.0-arm-linux-gnueabihf.tar.gz",
"53ec97ec51de214793172e20bb72b7639981720fa69ae81fb2f7c3e132218dcf": "umkoin-0.21.0-osx64.tar.gz",
"ffcd9504c957f0b0d8ec24911f1a3fd023315edad853e5fa8908d869af0b0769": "umkoin-0.21.0-riscv64-linux-gnu.tar.gz",
"4f0e60e22b501d2494d1cca6f6f507fa9a5ee0639af86901878500df6665d2d1": "umkoin-0.21.0-x86_64-linux-gnu.tar.gz",

"d2b2b69917aa2f3389066e637fb38cd18ea8aa51b0b117edc878e600926a005b": "umkoin-22.0-aarch64-linux-gnu.tar.gz",
"d4a6d99c21d200af3a24d0fad374a12834d44b9885f6677c52d6b293da559fdd": "umkoin-22.0-arm-linux-gnueabihf.tar.gz",
"9e3ad3ea0fd71d7e65af3bda5d08702bcc2f3a7919321d9264ecb196b4787e21": "umkoin-22.0-osx64.tar.gz",
"babb1a1a1b976f6593d692f6ff77f6c27d4fbbba242be7ca773f7d2229f0e52d": "umkoin-22.0-powerpc64le-linux-gnu.tar.gz",
"d2a4b19a71edcddadf97ffd60a9ce0d09fdbe03ae5cb2fb8a30b7d4d0bf8e745": "umkoin-22.0-powerpc64-linux-gnu.tar.gz",
"b1f9c2cff853c1adc2a6601dfce9fc3e7f3aa92d4aac4f08d6a6fc1fc5d1b954": "umkoin-22.0-riscv64-linux-gnu.tar.gz",
"23ad8e83cb5761979f7d53b19814eda0d8e883588e30c758a4ef48fb01c6cc0a": "umkoin-22.0-x86_64-linux-gnu.tar.gz",

"87fc9e3325f1912474ef84a8cdeadff06b00bab870823675418cca18c56b1fef": "umkoin-23.0.1-x86_64-linux-gnu.tar.gz",
"f0006a4bb38d19945251bcefc3315f2903bfafadeaf8ba63a8b89b5bb9274798": "umkoin-23.0.1-arm64-apple-darwin.dmg",
"e77014ed1637aed8c54ec3dbc63c44a5e682dda73ad0c334ea9c533be9204c6a": "umkoin-23.0.1-riscv64-linux-gnu.tar.gz",
"4b979209f344c2b19d27731e253ee18644cb966a630fd350ea6ae12e5526f6ea": "umkoin-23.0.1-x86_64-apple-darwin.dmg",
"364575f19b05c92b4cd5aecd890e43a82b6bc5abf7311182d5e232aea20f83ea": "umkoin-23.0.1-powerpc64-linux-gnu.tar.gz",
"614158b6c71d465a3c70c699d5521d8759ef5517d4f56b69d4e58f47be50b451": "umkoin-23.0.1-arm-linux-gnueabihf.tar.gz",
"1ddef0fa86b3ab7ae971b13093bea496842c6f92def4bd6b9aeceb56f4814c9c": "umkoin-23.0.1-x86_64-apple-darwin.tar.gz",
"53916f01c420e7655762494b11bf3e43d0513b5ed7836bbb62420c4336b3f526": "umkoin-23.0.1-powerpc64le-linux-gnu.tar.gz",
"3cf2f22fd005bf2b8f877c3fb2c2c114a440776d7a1d25a668c10284d879d682": "umkoin-23.0.1-win64-setup.exe",
"ccf081444a29ba6a3183b0ad612512ab891811721117a7a6595b35bc79a2cce7": "umkoin-23.0.1-aarch64-linux-gnu.tar.gz",
"e9d715097126d6b0ec9cd267e07dfcd1dba595588b689b8aaad73ae267526cf5": "umkoin-23.0.1-arm64-apple-darwin.tar.gz",
"26cd05901c333857f117583d20345e26caa2b52b950ec8aa1596a181a3bc8709": "umkoin-23.0.1-win64.zip"
}


@contextlib.contextmanager
def pushd(new_dir) -> None:
    previous_dir = os.getcwd()
    os.chdir(new_dir)
    try:
        yield
    finally:
        os.chdir(previous_dir)


def download_from_url(url, archive):
    last_print_time = time.time()

    def progress_hook(progress_bytes, total_size):
        nonlocal last_print_time
        now = time.time()
        percent = min(100, (progress_bytes * 100) / total_size)
        bar_length = 40
        filled_length = int(bar_length * percent / 100)
        bar = '#' * filled_length + '-' * (bar_length - filled_length)
        if now - last_print_time >= 1 or percent >= 100:
            print(f'\rDownloading: [{bar}] {percent:.1f}%', flush=True, end="")
            last_print_time = now

    with urllib.request.urlopen(url) as response:
        if response.status != 200:
            raise RuntimeError(f"HTTP request failed with status code: {response.status}")

        total_size = int(response.getheader('Content-Length', 0))
        progress_bytes = 0

        with open(archive, 'wb') as file:
            while True:
                chunk = response.read(8192)
                if not chunk:
                    break
                file.write(chunk)
                progress_bytes += len(chunk)
                progress_hook(progress_bytes, total_size)

    print('\n', flush=True, end="") # Flush to avoid error output on the same line.


def download_binary(tag, args) -> int:
    if Path(tag).is_dir():
        if not args.remove_dir:
            print('Using cached {}'.format(tag))
            return 0
        shutil.rmtree(tag)

    bin_path = 'bin/umkoin-core-{}'.format(tag[1:])

    match = re.compile('v(.*)(rc[0-9]+)$').search(tag)
    if match:
        bin_path = 'bin/umkoin-core-{}/test.{}'.format(
            match.group(1), match.group(2))

    host = args.host
    if tag < "v23" and host in ["x86_64-apple-darwin", "arm64-apple-darwin"]:
        host = "osx64"

    archive_format = 'tar.gz'
    if host == 'win64':
        archive_format = 'zip'

    archive = f'umkoin-{tag[1:]}-{host}.{archive_format}'
    archive_url = f'https://bitcoincore.org/{bin_path}/{archive}'

    print(f'Fetching: {archive_url}')

    try:
        download_from_url(archive_url, archive)
    except Exception as e:
        print(f"\nDownload failed: {e}", file=sys.stderr)
        return 1

    hasher = hashlib.sha256()
    with open(archive, "rb") as afile:
        hasher.update(afile.read())
    archiveHash = hasher.hexdigest()

    if archiveHash not in SHA256_SUMS or SHA256_SUMS[archiveHash]['archive'] != archive:
        if archive in [v['archive'] for v in SHA256_SUMS.values()]:
            print(f"Checksum {archiveHash} did not match", file=sys.stderr)
        else:
            print("Checksum for given version doesn't exist", file=sys.stderr)
        return 1

    print("Checksum matched")
    Path(tag).mkdir()

    # Extract archive
    if host == 'win64':
        try:
            with zipfile.ZipFile(archive, 'r') as zip:
                zip.extractall(tag)
            # Remove the top level directory to match tar's --strip-components=1
            extracted_items = os.listdir(tag)
            top_level_dir = os.path.join(tag, extracted_items[0])
            # Move all files & subdirectories up one level
            for item in os.listdir(top_level_dir):
                shutil.move(os.path.join(top_level_dir, item), tag)
            # Remove the now-empty top-level directory
            os.rmdir(top_level_dir)
        except Exception as e:
            print(f"Zip extraction failed: {e}", file=sys.stderr)
            return 1
    else:
        ret = subprocess.run(['tar', '-zxf', archive, '-C', tag,
                              '--strip-components=1',
                              'umkoin-{tag}'.format(tag=tag[1:])]).returncode
        if ret != 0:
            print(f"Failed to extract the {tag} tarball", file=sys.stderr)
            return ret

    Path(archive).unlink()

    if tag >= "v23" and tag < "v28.2" and args.host == "arm64-apple-darwin":
        # Starting with v23 there are arm64 binaries for ARM (e.g. M1, M2) mac.
        # Until v28.2 they had to be signed to run.
        binary_path = f'{os.getcwd()}/{tag}/bin/'

        for arm_binary in os.listdir(binary_path):
            # Is it already signed?
            ret = subprocess.run(
                ['codesign', '-v', binary_path + arm_binary],
                stderr=subprocess.DEVNULL,  # Suppress expected stderr output
            ).returncode
            if ret == 1:
                # Have to self-sign the binary
                ret = subprocess.run(
                    ['codesign', '-s', '-', binary_path + arm_binary]
                ).returncode
                if ret != 0:
                    print(f"Failed to self-sign {tag} {arm_binary} arm64 binary", file=sys.stderr)
                    return 1

                # Confirm success
                ret = subprocess.run(
                    ['codesign', '-v', binary_path + arm_binary]
                ).returncode
                if ret != 0:
                    print(f"Failed to verify the self-signed {tag} {arm_binary} arm64 binary", file=sys.stderr)
                    return 1

    return 0


def set_host(args) -> int:
    if platform.system().lower() == 'windows':
        if platform.machine() != 'AMD64':
            print('Only 64bit Windows supported', file=sys.stderr)
            return 1
        args.host = 'win64'
        return 0
    host = os.environ.get('HOST', subprocess.check_output(
        './depends/config.guess').decode())
    platforms = {
        'aarch64-*-linux*': 'aarch64-linux-gnu',
        'powerpc64le-*-linux-*': 'powerpc64le-linux-gnu',
        'riscv64-*-linux*': 'riscv64-linux-gnu',
        'x86_64-*-linux*': 'x86_64-linux-gnu',
        'x86_64-apple-darwin*': 'x86_64-apple-darwin',
        'aarch64-apple-darwin*': 'arm64-apple-darwin',
    }
    args.host = ''
    for pattern, target in platforms.items():
        if fnmatch(host, pattern):
            args.host = target
    if not args.host:
        print('Not sure which binary to download for {}'.format(host), file=sys.stderr)
        return 1
    return 0


def main(args) -> int:
    Path(args.target_dir).mkdir(exist_ok=True, parents=True)
    print("Releases directory: {}".format(args.target_dir))
    ret = set_host(args)
    if ret:
        return ret
    with pushd(args.target_dir):
        for tag in args.tags:
            ret = download_binary(tag, args)
            if ret:
                return ret
    return 0


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
        epilog='''
        HOST can be set to any of the `host-platform-triplet`s from
        depends/README.md for which a release exists.
        ''',
    )
    parser.add_argument('-r', '--remove-dir', action='store_true',
                        help='remove existing directory.')
    parser.add_argument('-t', '--target-dir', action='store',
                        help='target directory.', default='releases')
    all_tags = sorted([*set([v['tag'] for v in SHA256_SUMS.values()])])
    parser.add_argument('tags', nargs='*', default=all_tags,
                        help='release tags. e.g.: v0.18.1 v0.20.0rc2 '
                        '(if not specified, the full list needed for '
                        'backwards compatibility tests will be used)'
                        )
    args = parser.parse_args()
    sys.exit(main(args))
