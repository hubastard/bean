# Bean

Pure-C99, headers-only wrappers for Apple Cocoa frameworks. Write macOS apps in pure C, no objective-c `.m` or `.mm` files required.

Bean gives you typed C functions for Objective-C selectors like `NSWindow_MakeKeyAndOrderFront` backed by a small runtime layer over `objc_msgSend`.

> **Note:** This project is a work in progress and may not be production ready. The API is still evolving — I'm actively working on improving it.

## Requirements

- macOS with Xcode Command Line Tools
- Python 3.9+ (codegen only — not needed to consume the headers)

## Usage

Include the framework headers you need:

```c
#include <bean/appkit.h>

int main(void) {
    autoreleasepool {
        NSApplication app = NSApplication_SharedApplication();
        NSApplication_SetActivationPolicy(app, NSApplicationActivationPolicyRegular);

        CGRect frame = CGRectMake(100, 100, 640, 480);
        NSWindow window = NSWindow_InitWithContentRect_StyleMask_Backing_Defer(
            NSWindow_Alloc(),
            frame,
            NSWindowStyleMaskTitled | NSWindowStyleMaskClosable,
            NSBackingStoreBuffered,
            NO);

        NSWindow_SetTitle(window, NSString_StringWithUTF8String("Hello"));
        NSWindow_MakeKeyAndOrderFront(window, NULL);
        NSApplication_Run(app);
    }
    return 0;
}
```

## Supported frameworks

26 frameworks are generated today, in dependency order:

- CoreFoundation
- CoreGraphics
- CoreText
- CoreVideo
- CoreMedia
- Foundation
- Security
- ImageIO
- IOKit
- Carbon
- CloudKit
- CoreData
- Contacts
- EventKit
- Metal
- QuartzCore
- AppKit
- WebKit
- AVFoundation
- SceneKit
- SpriteKit
- MetalKit
- GameplayKit
- MapKit
- PDFKit
- StoreKit

## How the headers are generated

**The framework headers in `include/bean/` are generated, not hand-written.**

Bean's codegen tool (`tools/codegen/bean_gen.py`) parses each Apple SDK **umbrella header** with Clang and walks the resulting **AST JSON** (`clang -ast-dump=json`). From that tree it extracts:

- `@interface` methods and properties
- enums and struct layouts
- selector names and type encodings

Those declarations are mapped to C types and emitted as one header per framework (`foundation.h`, `appkit.h`, …). A separate pass merges Apple's **`.apinotes`** files to apply type overrides and skip APIs marked `Availability: nonswift`. Anything that cannot be expressed cleanly in C currently is recorded in `include/bean/unsupported.h` and I'm working on reducing this unsupported list until 100% is supported.

Including the codegen scripts, these 2 header files are also maintained by hand:

| File | Purpose |
|------|---------|
| `include/bean/runtime.h` | `objc_msgSend` helpers, autorelease pools, class lookup |
| `include/bean/types.h` | Shared struct definitions and typedefs used across frameworks |

When you upgrade Xcode, if the headers are not up to date, you can regenerate them yourself:

```bash
make generate
```

## Make targets

```bash
make help                             # list commands
make venv                             # create .venv + install codegen deps
make generate                         # regenerate all framework headers
make test                             # unit tests, smoke tests, examples, header checks
make examples                         # list example names
make examples <name>                  # build and run one example
make clean                            # remove build/
```

### Linking example

```bash
clang -Iinclude main.c -framework AppKit
```

Link the Apple frameworks your code uses; Bean itself is header-only.

## Regenerating headers

Full regeneration (all 26 frameworks + `unsupported.h` manifest):

```bash
make generate
```

You can pass extra arguments to the generate command by doing this:

```bash
make generate GEN_ARGS="--framework foundation --framework appkit"
```

| Flag | Effect |
|------|--------|
| `--framework <name>` | Regenerate only one or more frameworks (repeatable) |
| `--sdk <path>` | Pin to a specific MacOSX.sdk (default: `xcrun --show-sdk-path`) |
| `--no-apinotes` | Skip `.apinotes` filtering |
| `--with-docs` | Embed SDK header comments and `@see` links above wrappers |

Skipped APIs are listed with reasons in `include/bean/unsupported.h`. My goal is to be able to overtime reduce this unsupported list to zero to have full frameworks support.

## Examples

`examples/` has one small program to showcase framework usage. See [examples/README.md](examples/README.md) for the full list.

```bash
make examples   # list names

make examples foundation
make examples appkit
```

Low-level frameworks (CoreFoundation, CoreGraphics, CoreText, …) are skipped — they are covered indirectly as dependencies.

## Development

```bash
make test    # unit tests, smoke tests, all examples, header syntax checks
```

Pin CI to a specific Xcode and rerun `make generate` after SDK updates.

## Contributing

1. Make codegen changes in `tools/codegen/`.
2. Regenerate affected headers: `make generate GEN_ARGS="--framework …"`.
3. Run `make test` before opening a PR.

If you add a new framework, update `GENERATION_ORDER` and framework specs in `tools/codegen/bean/config.py`.
