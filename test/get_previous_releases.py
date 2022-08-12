#!/usr/bin/env python3
#
# Copyright (c) 2018-2021 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
#
# Download or build previous releases.
# Needs curl and tar to download a release, or the build dependencies when
# building a release.

import argparse
import contextlib
from fnmatch import fnmatch
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import hashlib


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


def download_binary(tag, args) -> int:
    if Path(tag).is_dir():
        if not args.remove_dir:
            print('Using cached {}'.format(tag))
            return 0
        shutil.rmtree(tag)
    Path(tag).mkdir()
    bin_path = 'bin/umkoin-core-{}'.format(tag[1:])
    match = re.compile('v(.*)(rc[0-9]+)$').search(tag)
    if match:
        bin_path = 'bin/umkoin-core-{}/test.{}'.format(
            match.group(1), match.group(2))
    platform = args.platform
    if tag < "v23" and platform in ["x86_64-apple-darwin", "arm64-apple-darwin"]:
        platform = "osx64"
    tarball = 'umkoin-{tag}-{platform}.tar.gz'.format(
        tag=tag[1:], platform=platform)
    tarballUrl = 'http://www.umkoin.org/{bin_path}/{tarball}'.format(
        bin_path=bin_path, tarball=tarball)

    print('Fetching: {tarballUrl}'.format(tarballUrl=tarballUrl))

    header, status = subprocess.Popen(
        ['curl', '--head', tarballUrl], stdout=subprocess.PIPE).communicate()
    if re.search("404 Not Found", header.decode("utf-8")):
        print("Binary tag was not found")
        return 1

    curlCmds = [
        ['curl', '--remote-name', tarballUrl]
    ]

    for cmd in curlCmds:
        ret = subprocess.run(cmd).returncode
        if ret:
            return ret

    hasher = hashlib.sha256()
    with open(tarball, "rb") as afile:
        hasher.update(afile.read())
    tarballHash = hasher.hexdigest()

    if tarballHash not in SHA256_SUMS or SHA256_SUMS[tarballHash]['tarball'] != tarball:
        if tarball in SHA256_SUMS.values():
            print("Checksum did not match")
            return 1

        print("Checksum for given version doesn't exist")
        return 1
    print("Checksum matched")

    # Extract tarball
    ret = subprocess.run(['tar', '-zxf', tarball, '-C', tag,
                          '--strip-components=1',
                          'umkoin-{tag}'.format(tag=tag[1:])]).returncode
    if ret:
        return ret

    Path(tarball).unlink()
    return 0


def build_release(tag, args) -> int:
    githubUrl = "https://github.com/umkoin/umkoin"
    if args.remove_dir:
        if Path(tag).is_dir():
            shutil.rmtree(tag)
    if not Path(tag).is_dir():
        # fetch new tags
        subprocess.run(
            ["git", "fetch", githubUrl, "--tags"])
        output = subprocess.check_output(['git', 'tag', '-l', tag])
        if not output:
            print('Tag {} not found'.format(tag))
            return 1
    ret = subprocess.run([
        'git', 'clone', githubUrl, tag
    ]).returncode
    if ret:
        return ret
    with pushd(tag):
        ret = subprocess.run(['git', 'checkout', tag]).returncode
        if ret:
            return ret
        host = args.host
        if args.depends:
            with pushd('depends'):
                ret = subprocess.run(['make', 'NO_QT=1']).returncode
                if ret:
                    return ret
                host = os.environ.get(
                    'HOST', subprocess.check_output(['./config.guess']))
        config_flags = '--prefix={pwd}/depends/{host} '.format(
            pwd=os.getcwd(),
            host=host) + args.config_flags
        cmds = [
            './autogen.sh',
            './configure {}'.format(config_flags),
            'make',
        ]
        for cmd in cmds:
            ret = subprocess.run(cmd.split()).returncode
            if ret:
                return ret
        # Move binaries, so they're in the same place as in the
        # release download
        Path('bin').mkdir(exist_ok=True)
        files = ['umkoind', 'umkoin-cli', 'umkoin-tx']
        for f in files:
            Path('src/'+f).rename('bin/'+f)
    return 0


def check_host(args) -> int:
    args.host = os.environ.get('HOST', subprocess.check_output(
        './depends/config.guess').decode())
    if args.download_binary:
        platforms = {
            'aarch64-*-linux*': 'aarch64-linux-gnu',
            'x86_64-*-linux*': 'x86_64-linux-gnu',
            'x86_64-apple-darwin*': 'x86_64-apple-darwin',
            'aarch64-apple-darwin*': 'arm64-apple-darwin',
        }
        args.platform = ''
        for pattern, target in platforms.items():
            if fnmatch(args.host, pattern):
                args.platform = target
        if not args.platform:
            print('Not sure which binary to download for {}'.format(args.host))
            return 1
    return 0


def main(args) -> int:
    Path(args.target_dir).mkdir(exist_ok=True, parents=True)
    print("Releases directory: {}".format(args.target_dir))
    ret = check_host(args)
    if ret:
        return ret
    if args.download_binary:
        with pushd(args.target_dir):
            for tag in args.tags:
                ret = download_binary(tag, args)
                if ret:
                    return ret
        return 0
    args.config_flags = os.environ.get('CONFIG_FLAGS', '')
    args.config_flags += ' --without-gui --disable-tests --disable-bench'
    with pushd(args.target_dir):
        for tag in args.tags:
            ret = build_release(tag, args)
            if ret:
                return ret
    return 0


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-r', '--remove-dir', action='store_true',
                        help='remove existing directory.')
    parser.add_argument('-d', '--depends', action='store_true',
                        help='use depends.')
    parser.add_argument('-b', '--download-binary', action='store_true',
                        help='download release binary.')
    parser.add_argument('-t', '--target-dir', action='store',
                        help='target directory.', default='releases')
    parser.add_argument('tags', nargs='*', default=set(
                            [v['tag'] for v in SHA256_SUMS.values()]
                        ),
                        help='release tags. e.g.: v0.18.1 v0.20.0rc2 '
                        '(if not specified, the full list needed for'
                        'backwards compatibility tests will be used)'
                        )
    args = parser.parse_args()
    sys.exit(main(args))
