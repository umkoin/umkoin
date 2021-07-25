#!/usr/bin/env python3
#
# Copyright (c) 2018-2020 The Bitcoin Core developers
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

"71603c2015becc90c0efd91d6b4f91e1c2ae9a2344bd672e9beee01bce83b7b2": "umkoin-0.20.0-aarch64-linux-gnu.tar.gz",
"78afc01e0e8ccf8ca90b139b9891c3a5adbfecbf1111cca82e613462e3d841d6": "umkoin-0.20.0-arm-linux-gnueabihf.tar.gz",
"218bcea3eb9e42ce57ccfa26711827600a1b7164fbd02469513f11f023b29090": "umkoin-0.20.0-osx64.tar.gz",
"5df8ef0cc548c3a69b9e475403ed5bbb5cadba964c4c3d873f1fda96afcd15b1": "umkoin-0.20.0-riscv64-linux-gnu.tar.gz",
"c4d8896d53160ab039ab250d7e70c2822bb02aaaeac2f8f07fb73eff4635a87e": "umkoin-0.20.0-x86_64-linux-gnu.tar.gz",

"ad356f577f3fffe646ffbe73bd3655612f794e9cc57995984f9f88581ea6fbb3": "umkoin-0.20.1-aarch64-linux-gnu.tar.gz",
"2ef0bf4045ecdbd4fc34c1c818c0d4b5f52ca37709d71e8e0388f5272fb17214": "umkoin-0.20.1-arm-linux-gnueabihf.tar.gz",
"6f6411c8409e91b070f54edf76544cdc85cfd2b9ffe0dba200fb68cddb1e3010": "umkoin-0.20.1-osx64.tar.gz",
"e55c32e91800156032dcc2ff9bc654df2068b46bab24e63328755a1c2fd588e2": "umkoin-0.20.1-riscv64-linux-gnu.tar.gz",
"2430e4c813ea0de28ef939170e05a36eaa1d2031589abe3b32491d5835f7b70e": "umkoin-0.20.1-x86_64-linux-gnu.tar.gz"
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
    tarball = 'umkoin-{tag}-{platform}.tar.gz'.format(
        tag=tag[1:], platform=args.platform)
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

    if tarballHash not in SHA256_SUMS or SHA256_SUMS[tarballHash] != tarball:
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
            'x86_64-apple-darwin*': 'osx64',
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
    parser.add_argument('tags', nargs='+',
                        help="release tags. e.g.: v0.18.1 v0.20.0rc2")
    args = parser.parse_args()
    sys.exit(main(args))
