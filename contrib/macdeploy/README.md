# MacOS Deployment

The `macdeployqtplus` script should not be run manually. Instead, after building as usual:

```bash
make deploy
```

When complete, it will have produced `Umkoin-Core.zip`.

## SDK Extraction

### Step 1: Obtaining `Xcode.app`

A free Apple Developer Account is required to proceed.

Our macOS SDK can be extracted from
[Xcode_15.2.xip](https://download.developer.apple.com/Developer_Tools/Xcode_15.2/Xcode_15.2.xip).

Alternatively, after logging in to your account go to 'Downloads', then 'More'
and search for [`Xcode 15`](https://developer.apple.com/download/all/?q=Xcode%2015).

An Apple ID and cookies enabled for the hostname are needed to download this.

The `sha256sum` of the downloaded XIP archive should be `04e93680c6ddbec84666531be412de778afc8eac6ab2037f4c2be7290818b59b`.

To extract the `.xip` on Linux:

```bash
# Install/clone tools needed for extracting Xcode.app
apt install cpio
git clone https://github.com/bitcoin-core/apple-sdk-tools.git

# Unpack the .xip and place the resulting Xcode.app in your current
# working directory
python3 apple-sdk-tools/extract_xcode.py -f Xcode_15.2.xip | cpio -d -i
```

On macOS:

```bash
xip -x Xcode_15.2.xip
```

### Step 2: Generating the SDK tarball from `Xcode.app`

To generate the SDK, run the script [`gen-sdk`](./gen-sdk) with the
path to `Xcode.app` (extracted in the previous stage) as the first argument.

```bash
./contrib/macdeploy/gen-sdk '/path/to/Xcode.app'
```

The generated archive should be: `Xcode-15.2-15C500b-extracted-SDK-with-libcxx-headers.tar.gz`.
The `sha256sum` should be `7c74146250fc6b6ea37b3f00b17a019fcade109cfe609989edecbbd6f228bfce`.

## Deterministic macOS App Notes

macOS Applications are created in Linux using a recent LLVM.

Apple uses `clang` extensively for development and has upstreamed the necessary
functionality so that a vanilla clang can take advantage. It supports the use of `-F`,
`-target`, `-mmacosx-version-min`, and `-isysroot`, which are all necessary when
building for macOS.

To complicate things further, all builds must target an Apple SDK. These SDKs are free to
download, but not redistributable. See the SDK Extraction notes above for how to obtain it.

The Guix process builds 2 sets of files: Linux tools, then Apple binaries which are
created using these tools. The build process has been designed to avoid including the
SDK's files in Guix's outputs. All interim tarballs are fully deterministic and may be freely
redistributed.

As of OS X 10.9 Mavericks, using an Apple-blessed key to sign binaries is a requirement in
order to satisfy the new Gatekeeper requirements. Because this private key cannot be
shared, we'll have to be a bit creative in order for the build process to remain somewhat
deterministic. Here's how it works:

- Builders use Guix to create an unsigned release. This outputs an unsigned ZIP which
  users may choose to bless and run. It also outputs an unsigned app structure in the form
  of a tarball.
- The Apple keyholder uses this unsigned app to create a detached signature, using the
  script that is also included there. Detached signatures are available from this [repository](https://github.com/umkoin/umkoin-detached-sigs).
- Builders feed the unsigned app + detached signature back into Guix. It uses the
  pre-built tools to recombine the pieces into a deterministic ZIP.
