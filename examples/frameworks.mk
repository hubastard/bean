# Link flags for each example (passed to clang as -framework …).
# Edit this file when adding an example or changing its dependencies.

EXAMPLE_FRAMEWORKS_foundation := Foundation

EXAMPLE_FRAMEWORKS_appkit := AppKit Foundation CoreGraphics QuartzCore Metal CoreImage
EXAMPLE_FRAMEWORKS_quartzcore := AppKit Foundation CoreGraphics QuartzCore Metal CoreImage
EXAMPLE_FRAMEWORKS_webkit := AppKit WebKit Foundation CoreGraphics QuartzCore Metal CoreImage

EXAMPLE_FRAMEWORKS_avfoundation := AVFoundation Foundation CoreMedia CoreVideo CoreGraphics Metal AudioToolbox CoreMIDI QuartzCore

EXAMPLE_FRAMEWORKS_coredata := CoreData CloudKit Foundation
EXAMPLE_FRAMEWORKS_cloudkit := CloudKit Foundation

EXAMPLE_FRAMEWORKS_contacts := Contacts Foundation AppKit
EXAMPLE_FRAMEWORKS_eventkit := EventKit Foundation AppKit

EXAMPLE_FRAMEWORKS_gameplaykit := GameplayKit SceneKit AppKit Foundation QuartzCore Metal CoreImage CloudKit

EXAMPLE_FRAMEWORKS_mapkit := MapKit AppKit Foundation CoreGraphics QuartzCore Metal CoreImage

EXAMPLE_FRAMEWORKS_metal := Metal AppKit Foundation QuartzCore
EXAMPLE_FRAMEWORKS_metalkit := MetalKit Metal AppKit Foundation QuartzCore CoreImage

EXAMPLE_FRAMEWORKS_pdfkit := PDFKit AppKit Foundation CloudKit CoreGraphics QuartzCore Metal CoreImage
EXAMPLE_FRAMEWORKS_quicklook := QuickLook AppKit Foundation CoreGraphics QuartzCore Metal CoreImage

EXAMPLE_FRAMEWORKS_scenekit := SceneKit AppKit Foundation QuartzCore Metal CoreImage CloudKit
EXAMPLE_FRAMEWORKS_spritekit := SpriteKit AppKit Foundation QuartzCore Metal CoreImage CloudKit

EXAMPLE_FRAMEWORKS_storekit := StoreKit AppKit Foundation CoreGraphics QuartzCore Metal CoreImage
