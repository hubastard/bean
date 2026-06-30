# Bean examples

One small program per showcase framework. Each example is built with `-std=c99 -Wall -Wextra -Werror` via the root `Makefile`.

## Running an example

```bash
make example foundation
make example appkit
make example NAME=webkit
make examples    # list all names
```

GUI examples open a window and run `NSApplication` until you close it or press **Esc**. CLI examples print to stdout and exit.

## Examples

| Directory | Framework | What it shows |
|-----------|-----------|---------------|
| `foundation/` | Foundation | `NSString`, `NSArray` |
| `appkit/` | AppKit | Minimal window + event loop |
| `quartzcore/` | QuartzCore | `CALayer` background on a view |
| `webkit/` | WebKit | `WKWebView` loading the Bean GitHub repo (with a loading label) |
| `avfoundation/` | AVFoundation | Camera authorization status |
| `coredata/` | CoreData | In-memory model + default store URL |
| `cloudkit/` | CloudKit | Default container identifier |
| `contacts/` | Contacts | Contacts authorization status |
| `eventkit/` | EventKit | Calendar authorization status |
| `metal/` | Metal | `MTLTextureDescriptor` factory |
| `metalkit/` | MetalKit | `MTKView` in a window |
| `scenekit/` | SceneKit | `SCNView` with a box |
| `spritekit/` | SpriteKit | `SKView` + empty scene |
| `gameplaykit/` | GameplayKit | Random die roll |
| `mapkit/` | MapKit | `MKMapView` centered on SF |
| `pdfkit/` | PDFKit | `PDFView` displaying `sample.pdf` |
| `storekit/` | StoreKit | `canMakePayments` |

Shared window helpers live in `common/app_shell.h`.

## Skipped (low-level / no useful demo)

These frameworks have generated headers but no standalone example:

CoreFoundation, CoreGraphics, CoreText, CoreVideo, CoreMedia, Security, ImageIO, IOKit, Carbon

They are dependencies of the examples above.

## Adding an example

1. Create `examples/<name>/main.c` and add a line to `examples/frameworks.mk`.
2. Run `make example <name>` to build and run.
3. `make test` compiles every example automatically.
