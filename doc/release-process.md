Release Process
====================

## Branch updates

### Before every release candidate

* Update translations, see [translation_process.md](https://github.com/umkoin/umkoin/blob/master/doc/translation_process.md#synchronising-translations). 
* Update manpages, see [gen-manpages.sh](https://github.com/umkoin/umkoin/blob/master/contrib/devtools/README.md#gen-manpagessh). 
* Update release candidate version in `configure.ac` (`CLIENT_VERSION_RC`).

### Before every major and minor release

* Update [bips.md](bips.md) to account for changes since the last release (don't forget to bump the version number on the first line).
* Update version in `configure.ac` (don't forget to set `CLIENT_VERSION_RC` to `0`).
* Write release notes (see "Write the release notes" below).

### Before every major release

* On both the master branch and the new release branch:
  - update `CLIENT_VERSION_MAJOR` in [`configure.ac`](../configure.ac)
  - update `CLIENT_VERSION_MAJOR`, `PACKAGE_VERSION`, and `PACKAGE_STRING` in [`build_msvc/umkoin_config.h`](/build_msvc/umkoin_config.h)
* On the new release branch in [`configure.ac`](../configure.ac) and [`build_msvc/umkoin_config.h`](/build_msvc/umkoin_config.h):
  - set `CLIENT_VERSION_MINOR` to `0`
  - set `CLIENT_VERSION_BUILD` to `0`
  - set `CLIENT_VERSION_IS_RELEASE` to `true`

#### Before branch-off

* Update hardcoded [seeds](/contrib/seeds/README.md), see [this pull request](https://github.com/bitcoin/bitcoin/pull/7415) for an example.
* Update [`src/chainparams.cpp`](/src/chainparams.cpp) m_assumed_blockchain_size and m_assumed_chain_state_size with the current size plus some overhead (see [this](#how-to-calculate-assumed-blockchain-and-chain-state-size) for information on how to calculate them).
* Update [`src/chainparams.cpp`](/src/chainparams.cpp) chainTxData with statistics about the transaction count and rate. Use the output of the `getchaintxstats` RPC, see
  [this pull request](https://github.com/bitcoin/bitcoin/pull/20263) for an example. Reviewers can verify the results by running `getchaintxstats <window_block_count> <window_final_block_hash>` with the `window_block_count` and `window_final_block_hash` from your output.
* Update `src/chainparams.cpp` nMinimumChainWork and defaultAssumeValid (and the block height comment) with information from the `getblockheader` (and `getblockhash`) RPCs.
  - The selected value must not be orphaned so it may be useful to set the value two blocks back from the tip.
  - Testnet should be set some tens of thousands back from the tip due to reorgs there.
  - This update should be reviewed with a reindex-chainstate with assumevalid=0 to catch any defect
     that causes rejection of blocks in the past history.
- Clear the release notes and move them to the wiki (see "Write the release notes" below).

#### After branch-off (on master)

- Update the version of `contrib/gitian-descriptors/*.yml`.

#### After branch-off (on the major release branch)

- Update the versions.
- Create a pinned meta-issue for testing the release candidate and provide a link to it in the release announcements where useful.

#### Before final release

- Merge the release notes from the wiki into the branch.
- Ensure the "Needs release note" label is removed from all relevant pull requests and issues.


## Building

### First time / New builders

If you're using the automated script (found in [contrib/gitian-build.py](/contrib/gitian-build.py)), then at this point you should run it with the "--setup" command. Otherwise ignore this.

Check out the source code in the following directory hierarchy.

    cd /path/to/your/toplevel/build
    git clone https://github.com/umkoin/gitian.sigs.git
    git clone https://github.com/umkoin/umkoin-detached-sigs.git
    git clone https://github.com/devrandom/gitian-builder.git
    git clone https://github.com/umkoin/umkoin.git

### Write the release notes

Write the release notes. `git shortlog` helps a lot, for example:

    git shortlog --no-merges v(current version, e.g. 0.19.2)..v(new version, e.g. 0.20.0)

Generate list of authors:

    git log --format='- %aN' v(current version, e.g. 0.20.0)..v(new version, e.g. 0.20.1) | sort -fiu

Tag the version (or release candidate) in git:

    git tag -s v(new version, e.g. 0.20.0)

### Setup and perform Gitian builds

If you're using the automated script (found in [contrib/gitian-build.py](/contrib/gitian-build.py)), then at this point you should run it with the "--build" command. Otherwise ignore this.

Setup Gitian descriptors:

    pushd ./umkoin
    export SIGNER="(your Gitian key, ie bluematt, sipa, vmta, etc)"
    export VERSION=(new version, e.g. 0.20.0)
    git fetch
    git checkout v${VERSION}
    popd

Ensure your gitian.sigs are up-to-date if you wish to gverify your builds against other Gitian signatures.

    pushd ./gitian.sigs
    git pull
    popd

Ensure gitian-builder is up-to-date:

    pushd ./gitian-builder
    git pull
    popd

### Fetch and create inputs: (first time, or when dependency versions change)

    pushd ./gitian-builder
    mkdir -p inputs
    wget -O inputs/osslsigncode-2.0.tar.gz https://github.com/mtrojnar/osslsigncode/archive/2.0.tar.gz
    echo '5a60e0a4b3e0b4d655317b2f12a810211c50242138322b16e7e01c6fbb89d92f inputs/osslsigncode-2.0.tar.gz' | sha256sum -c
    popd

Create the macOS SDK tarball, see the [macdeploy instructions](/contrib/macdeploy/README.md#deterministic-macos-dmg-notes) for details, and copy it into the inputs directory.

### Optional: Seed the Gitian sources cache and offline git repositories

NOTE: Gitian is sometimes unable to download files. If you have errors, try the step below.

By default, Gitian will fetch source files as needed. To cache them ahead of time, make sure you have checked out the tag you want to build in umkoin, then:

    pushd ./gitian-builder
    make -C ../umkoin/depends download SOURCES_PATH=`pwd`/cache/common
    popd

Only missing files will be fetched, so this is safe to re-run for each build.

NOTE: Offline builds must use the --url flag to ensure Gitian fetches only from local URLs. For example:

    pushd ./gitian-builder
    ./bin/gbuild --url umkoin=/path/to/umkoin,signature=/path/to/sigs {rest of arguments}
    popd

The gbuild invocations below <b>DO NOT DO THIS</b> by default.

### Build and sign Umkoin Core for Linux, Windows, and macOS:

    pushd ./gitian-builder
    ./bin/gbuild --num-make 2 --memory 3000 --commit umkoin=v${VERSION} ../umkoin/contrib/gitian-descriptors/gitian-linux.yml
    ./bin/gsign --signer "$SIGNER" --release ${VERSION}-linux --destination ../gitian.sigs/ ../umkoin/contrib/gitian-descriptors/gitian-linux.yml
    mv build/out/umkoin-*.tar.gz build/out/src/umkoin-*.tar.gz ../

    ./bin/gbuild --num-make 2 --memory 3000 --commit umkoin=v${VERSION} ../umkoin/contrib/gitian-descriptors/gitian-win.yml
    ./bin/gsign --signer "$SIGNER" --release ${VERSION}-win-unsigned --destination ../gitian.sigs/ ../umkoin/contrib/gitian-descriptors/gitian-win.yml
    mv build/out/umkoin-*-win-unsigned.tar.gz inputs/umkoin-win-unsigned.tar.gz
    mv build/out/umkoin-*.zip build/out/umkoin-*.exe ../

    ./bin/gbuild --num-make 2 --memory 3000 --commit umkoin=v${VERSION} ../umkoin/contrib/gitian-descriptors/gitian-osx.yml
    ./bin/gsign --signer "$SIGNER" --release ${VERSION}-osx-unsigned --destination ../gitian.sigs/ ../umkoin/contrib/gitian-descriptors/gitian-osx.yml
    mv build/out/umkoin-*-osx-unsigned.tar.gz inputs/umkoin-osx-unsigned.tar.gz
    mv build/out/umkoin-*.tar.gz build/out/umkoin-*.dmg ../
    popd

Build output expected:

  1. source tarball (`umkoin-${VERSION}.tar.gz`)
  2. linux 32-bit and 64-bit dist tarballs (`umkoin-${VERSION}-linux[32|64].tar.gz`)
  3. windows 32-bit and 64-bit unsigned installers and dist zips (`umkoin-${VERSION}-win[32|64]-setup-unsigned.exe`, `umkoin-${VERSION}-win[32|64].zip`)
  4. macOS unsigned installer and dist tarball (`umkoin-${VERSION}-osx-unsigned.dmg`, `umkoin-${VERSION}-osx64.tar.gz`)
  5. Gitian signatures (in `gitian.sigs/${VERSION}-<linux|{win,osx}-unsigned>/(your Gitian key)/`)

### Verify other gitian builders signatures to your own. (Optional)

Add other gitian builders keys to your gpg keyring, and/or refresh keys: See `../umkoin/contrib/gitian-keys/README.md`.

Verify the signatures

    pushd ./gitian-builder
    ./bin/gverify -v -d ../gitian.sigs/ -r ${VERSION}-linux ../umkoin/contrib/gitian-descriptors/gitian-linux.yml
    ./bin/gverify -v -d ../gitian.sigs/ -r ${VERSION}-win-unsigned ../umkoin/contrib/gitian-descriptors/gitian-win.yml
    ./bin/gverify -v -d ../gitian.sigs/ -r ${VERSION}-osx-unsigned ../umkoin/contrib/gitian-descriptors/gitian-osx.yml
    popd

### Next steps:

Commit your signature to gitian.sigs:

    pushd gitian.sigs
    git add ${VERSION}-linux/"${SIGNER}"
    git add ${VERSION}-win-unsigned/"${SIGNER}"
    git add ${VERSION}-osx-unsigned/"${SIGNER}"
    git commit -m "Add ${VERSION} unsigned sigs for ${SIGNER}"
    git push  # Assuming you can push to the gitian.sigs tree
    popd

Codesigner only: Create Windows/macOS detached signatures:
- Only one person handles codesigning. Everyone else should skip to the next step.
- Only once the Windows/macOS builds each have 3 matching signatures may they be signed with their respective release keys.

Codesigner only: Sign the macOS binary:

    transfer umkoin-osx-unsigned.tar.gz to macOS for signing
    tar xf umkoin-osx-unsigned.tar.gz
    ./detached-sig-create.sh -s "Key ID"
    Enter the keychain password and authorize the signature
    Move signature-osx.tar.gz back to the gitian host

Codesigner only: Sign the windows binaries:

    tar xf umkoin-win-unsigned.tar.gz
    ./detached-sig-create.sh -key /path/to/codesign.key
    Enter the passphrase for the key when prompted
    signature-win.tar.gz will be created

Codesigner only: Commit the detached codesign payloads:

    cd ~/umkoin-detached-sigs
    checkout the appropriate branch for this release series
    rm -rf *
    tar xf signature-osx.tar.gz
    tar xf signature-win.tar.gz
    git add -A
    git commit -m "point to ${VERSION}"
    git tag -s v${VERSION} HEAD
    git push the current branch and new tag

Non-codesigners: wait for Windows/macOS detached signatures:

- Once the Windows/macOS builds each have 3 matching signatures, they will be signed with their respective release keys.
- Detached signatures will then be committed to the [umkoin-detached-sigs](https://github.com/umkoin/umkoin-detached-sigs) repository, which can be combined with the unsigned apps to create signed binaries.

Create (and optionally verify) the signed macOS binary:

    pushd ./gitian-builder
    ./bin/gbuild -i --commit signature=v${VERSION} ../umkoin/contrib/gitian-descriptors/gitian-osx-signer.yml
    ./bin/gsign --signer "$SIGNER" --release ${VERSION}-osx-signed --destination ../gitian.sigs/ ../umkoin/contrib/gitian-descriptors/gitian-osx-signer.yml
    ./bin/gverify -v -d ../gitian.sigs/ -r ${VERSION}-osx-signed ../umkoin/contrib/gitian-descriptors/gitian-osx-signer.yml
    mv build/out/umkoin-osx-signed.dmg ../umkoin-${VERSION}-osx.dmg
    popd

Create (and optionally verify) the signed Windows binaries:

    pushd ./gitian-builder
    ./bin/gbuild -i --commit signature=v${VERSION} ../umkoin/contrib/gitian-descriptors/gitian-win-signer.yml
    ./bin/gsign --signer "$SIGNER" --release ${VERSION}-win-signed --destination ../gitian.sigs/ ../umkoin/contrib/gitian-descriptors/gitian-win-signer.yml
    ./bin/gverify -v -d ../gitian.sigs/ -r ${VERSION}-win-signed ../umkoin/contrib/gitian-descriptors/gitian-win-signer.yml
    mv build/out/umkoin-*win64-setup.exe ../umkoin-${VERSION}-win64-setup.exe
    popd

Commit your signature for the signed macOS/Windows binaries:

    pushd gitian.sigs
    git add ${VERSION}-osx-signed/"${SIGNER}"
    git add ${VERSION}-win-signed/"${SIGNER}"
    git commit -m "Add ${SIGNER} ${VERSION} signed binaries signatures"
    git push  # Assuming you can push to the gitian.sigs tree
    popd

### After 3 or more people have gitian-built and their results match:

- Create `SHA256SUMS.asc` for the builds, and GPG-sign it:

```bash
sha256sum * > SHA256SUMS
```

The list of files should be:
```
umkoin-${VERSION}-aarch64-linux-gnu.tar.gz
umkoin-${VERSION}-arm-linux-gnueabihf.tar.gz
umkoin-${VERSION}-riscv64-linux-gnu.tar.gz
umkoin-${VERSION}-x86_64-linux-gnu.tar.gz
umkoin-${VERSION}-osx64.tar.gz
umkoin-${VERSION}-osx.dmg
umkoin-${VERSION}.tar.gz
umkoin-${VERSION}-win64-setup.exe
umkoin-${VERSION}-win64.zip
```
The `*-debug*` files generated by the gitian build contain debug symbols
for troubleshooting by developers. It is assumed that anyone that is interested
in debugging can run gitian to generate the files for themselves. To avoid
end-user confusion about which file to pick, as well as save storage
space *do not upload these to the umkoin.org server, nor put them in the torrent*.

- GPG-sign it, delete the unsigned file:
```
gpg --digest-algo sha256 --clearsign SHA256SUMS # outputs SHA256SUMS.asc
rm SHA256SUMS
```
(the digest algorithm is forced to sha256 to avoid confusion of the `Hash:` header that GPG adds with the SHA256 used for the files)
Note: check that SHA256SUMS itself doesn't end up in SHA256SUMS, which is a spurious/nonsensical entry.

- Upload zips and installers, as well as `SHA256SUMS.asc` from last step, to the umkoin.org server
  into `/var/www/bin/umkoin-core-${VERSION}`

- Update umkoin.org version

  - First, check to see if the Umkoin.org maintainers have prepared a
    release: http://github.com/umkoin/umkoin.org/releases

      - If they have, it will have previously failed their Travis CI
        checks because the final release files weren't uploaded.
        Trigger a Travis CI rebuild---if it passes, merge.

  - After the pull request is merged, the website will automatically show the newest version within 15 minutes, as well
    as update the OS download links.

- Announce the release:

  - Archive release notes for the new version to `doc/release-notes/` (branch `master` and branch of the release)

  - Create a [new GitHub release](https://github.com/umkoin/umkoin/releases/new) with a link to the archived release notes.

  - Celebrate

### Additional information

#### How to calculate `m_assumed_blockchain_size` and `m_assumed_chain_state_size`

Both variables are used as a guideline for how much space the user needs on their drive in total, not just strictly for the blockchain.
Note that all values should be taken from a **fully synced** node and have an overhead of 5-10% added on top of its base value.

To calculate `m_assumed_blockchain_size`:
- For `mainnet` -> Take the size of the data directory, excluding `/regtest` and `/testnet3` directories.
- For `testnet` -> Take the size of the `/testnet3` directory.


To calculate `m_assumed_chain_state_size`:
- For `mainnet` -> Take the size of the `/chainstate` directory.
- For `testnet` -> Take the size of the `/testnet3/chainstate` directory.

Notes:
- When taking the size for `m_assumed_blockchain_size`, there's no need to exclude the `/chainstate` directory since it's a guideline value and an overhead will be added anyway.
- The expected overhead for growth may change over time, so it may not be the same value as last release; pay attention to that when changing the variables.
