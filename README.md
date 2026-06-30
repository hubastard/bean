# Bean

Pure-C99, headers-only wrappers for Apple Cocoa frameworks. Write macOS apps in pure C, no objective-c `.m` or `.mm` files required.

Bean gives you typed C functions for Objective-C selectors like `NSWindow_MakeKeyAndOrderFront` backed by a small runtime layer over `objc_msgSend`.

## Supported frameworks

27 frameworks are generated today, in dependency order:

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
- QuickLook
- StoreKit

## Requirements

- macOS with Xcode Command Line Tools
- Python 3.9+ (codegen only — not needed to consume the headers)
- **C99** — headers and validation use `-std=c99 -Wall -Wextra -Werror`

## Usage

Include the framework headers you need:

```c
#include <bean/appkit.h>

int main(void) {
    bean_autorelease_pool_scope pool = bean_autorelease_pool_push();

    NSApplication app = NSApplication_SharedApplication();
    NSApplication_SetActivationPolicy(app, NSApplicationActivationPolicyRegular);

    CGRect frame = CGRectMake(100, 100, 640, 480);
    NSWindow window = NSWindow_InitWithContentRect_StyleMask_Backing_Defer(
        (NSWindow)bean_alloc(bean_cls("NSWindow")),
        frame,
        NSWindowStyleMaskTitled | NSWindowStyleMaskClosable,
        NSBackingStoreBuffered,
        false);

    NSWindow_setTitle(window, NSString_StringWithUTF8String("Hello"));
    NSWindow_MakeKeyAndOrderFront(window, NULL);
    NSApplication_Run(app);

    bean_autorelease_pool_pop(pool);
    return 0;
}
```

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

When you upgrade Xcode, if the headers did not get updated yet, you can regenerate them yourself by running:

```bash
./scripts/generate_all.sh
```

### Conventions

- Object types are `typedef bean_id NSWindow` — use `NSWindow self`, not `NSWindow*`.
- Class lookup uses `bean_cls("ClassName")`, not `bean_class()` (name collision with the typedef).
- Memory: `bean_alloc`, `bean_retain`, `bean_release`, `bean_autorelease_pool_push/pop`.

### Linking

```bash
clang -Iinclude main.c -framework AppKit
```

Link the Apple frameworks your code uses; Bean itself is header-only.

## Regenerating headers

Prefer **targeted regeneration** when working on a single framework — each AST parse takes tens of seconds:

```bash
cd tools/codegen
python3 bean_gen.py --framework foundation --framework appkit
```

Full regeneration (all 27 frameworks + `unsupported.h` manifest):

```bash
./scripts/generate_all.sh
```

| Flag | Effect |
|------|--------|
| `--framework <name>` | Regenerate one or more frameworks (repeatable) |
| `--sdk <path>` | Pin to a specific MacOSX.sdk (default: `xcrun --show-sdk-path`) |
| `--no-apinotes` | Skip `.apinotes` filtering |
| `--with-docs` | Embed SDK header comments and `@see` links above wrappers |

Skipped APIs are listed with reasons in `include/bean/unsupported.h`.

## Development

```bash
./scripts/validate.sh                   # syntax-check every generated header + smoke tests
cd tools/codegen && python3 -m pytest   # codegen unit tests
```

`validate.sh` prints the SDK path and Clang version in use — pin CI to a specific Xcode and rerun `./scripts/generate_all.sh` after SDK updates.

## Contributing

1. Make codegen changes in `tools/codegen/`.
2. Regenerate affected headers with `bean_gen.py`.
3. Run `./scripts/validate.sh` before opening a PR.

If you add a new framework, update `GENERATION_ORDER` and framework specs in `tools/codegen/bean/config.py`.
