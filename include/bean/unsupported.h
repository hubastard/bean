/* Generated manifest of APIs requiring special handling. */
#ifndef BEAN_UNSUPPORTED_H
#define BEAN_UNSUPPORTED_H

/*
 * [appkit] NSAppleEventManager appleEventForSuspensionID: — internal type
 * [appkit] NSAppleEventManager replyAppleEventForSuspensionID: — internal type
 * [appkit] NSAppleEventManager resumeWithSuspensionID: — internal type
 * [appkit] NSAppleEventManager setCurrentAppleEventAndReplyEventWithSuspensionID: — internal type
 * [appkit] NSAppleEventManager suspendCurrentAppleEvent — internal type
 * [appkit] NSBundle loadNibNamed:owner:topLevelObjects: — internal type
 * [appkit] NSNib instantiateWithOwner:topLevelObjects: — internal type
 * [avfoundation] AVAudioInputNode setVoiceProcessingOtherAudioDuckingConfiguration: — incomplete struct parameter type
 * [avfoundation] AVAudioInputNode voiceProcessingOtherAudioDuckingConfiguration — incomplete struct return type
 * [avfoundation] AVCaptionRegion origin — incomplete struct return type
 * [avfoundation] AVCaptionRegion size — incomplete struct return type
 * [avfoundation] AVCaptureAudioDataOutput new — apinotes nonswift
 * [avfoundation] AVCaptureAudioFileOutput new — apinotes nonswift
 * [avfoundation] AVCaptureAudioPreviewOutput new — apinotes nonswift
 * [avfoundation] AVCaptureDepthDataOutput new — apinotes nonswift
 * [avfoundation] AVCaptureMetadataOutput new — apinotes nonswift
 * [avfoundation] AVCaptureMovieFileOutput new — apinotes nonswift
 * [avfoundation] AVCapturePhotoOutput new — apinotes nonswift
 * [avfoundation] AVCaptureStillImageOutput new — apinotes nonswift
 * [avfoundation] AVCaptureVideoDataOutput new — apinotes nonswift
 * [avfoundation] AVMusicTrack clearEventsInRange: — incomplete struct parameter type
 * [avfoundation] AVMusicTrack copyAndMergeEventsInRange:fromTrack:mergeAtBeat: — incomplete struct parameter type
 * [avfoundation] AVMusicTrack copyEventsInRange:fromTrack:insertAtBeat: — incomplete struct parameter type
 * [avfoundation] AVMusicTrack cutEventsInRange: — incomplete struct parameter type
 * [avfoundation] AVMusicTrack enumerateEventsInRange:usingBlock: — incomplete struct parameter type
 * [avfoundation] AVMusicTrack loopRange — incomplete struct return type
 * [avfoundation] AVMusicTrack moveEventsInRange:byAmount: — incomplete struct parameter type
 * [avfoundation] AVMusicTrack setLoopRange: — incomplete struct parameter type
 * [avfoundation] AVMutableCaptionRegion origin — incomplete struct return type
 * [avfoundation] AVMutableCaptionRegion setOrigin: — incomplete struct parameter type
 * [avfoundation] AVMutableCaptionRegion setSize: — incomplete struct parameter type
 * [avfoundation] AVMutableCaptionRegion size — incomplete struct return type
 * [avfoundation] AVSemanticSegmentationMatte semanticSegmentationMatteFromImageSourceAuxiliaryDataType:dictionaryRepresentation:error: — internal type
 * [avfoundation] CIContext createCGLayerWithSize:info: — internal type
 * [avfoundation] NSAppleEventManager appleEventForSuspensionID: — internal type
 * [avfoundation] NSAppleEventManager replyAppleEventForSuspensionID: — internal type
 * [avfoundation] NSAppleEventManager resumeWithSuspensionID: — internal type
 * [avfoundation] NSAppleEventManager setCurrentAppleEventAndReplyEventWithSuspensionID: — internal type
 * [avfoundation] NSAppleEventManager suspendCurrentAppleEvent — internal type
 * [carbon] NSAppleEventManager appleEventForSuspensionID: — internal type
 * [carbon] NSAppleEventManager replyAppleEventForSuspensionID: — internal type
 * [carbon] NSAppleEventManager resumeWithSuspensionID: — internal type
 * [carbon] NSAppleEventManager setCurrentAppleEventAndReplyEventWithSuspensionID: — internal type
 * [carbon] NSAppleEventManager suspendCurrentAppleEvent — internal type
 * [cloudkit] NSAppleEventManager appleEventForSuspensionID: — internal type
 * [cloudkit] NSAppleEventManager replyAppleEventForSuspensionID: — internal type
 * [cloudkit] NSAppleEventManager resumeWithSuspensionID: — internal type
 * [cloudkit] NSAppleEventManager setCurrentAppleEventAndReplyEventWithSuspensionID: — internal type
 * [cloudkit] NSAppleEventManager suspendCurrentAppleEvent — internal type
 * [contacts] NSAppleEventManager appleEventForSuspensionID: — internal type
 * [contacts] NSAppleEventManager replyAppleEventForSuspensionID: — internal type
 * [contacts] NSAppleEventManager resumeWithSuspensionID: — internal type
 * [contacts] NSAppleEventManager setCurrentAppleEventAndReplyEventWithSuspensionID: — internal type
 * [contacts] NSAppleEventManager suspendCurrentAppleEvent — internal type
 * [coredata] NSAppleEventManager appleEventForSuspensionID: — internal type
 * [coredata] NSAppleEventManager replyAppleEventForSuspensionID: — internal type
 * [coredata] NSAppleEventManager resumeWithSuspensionID: — internal type
 * [coredata] NSAppleEventManager setCurrentAppleEventAndReplyEventWithSuspensionID: — internal type
 * [coredata] NSAppleEventManager suspendCurrentAppleEvent — internal type
 * [coremedia] NSAppleEventManager appleEventForSuspensionID: — internal type
 * [coremedia] NSAppleEventManager replyAppleEventForSuspensionID: — internal type
 * [coremedia] NSAppleEventManager resumeWithSuspensionID: — internal type
 * [coremedia] NSAppleEventManager setCurrentAppleEventAndReplyEventWithSuspensionID: — internal type
 * [coremedia] NSAppleEventManager suspendCurrentAppleEvent — internal type
 * [corevideo] NSAppleEventManager appleEventForSuspensionID: — internal type
 * [corevideo] NSAppleEventManager replyAppleEventForSuspensionID: — internal type
 * [corevideo] NSAppleEventManager resumeWithSuspensionID: — internal type
 * [corevideo] NSAppleEventManager setCurrentAppleEventAndReplyEventWithSuspensionID: — internal type
 * [corevideo] NSAppleEventManager suspendCurrentAppleEvent — internal type
 * [eventkit] NSAppleEventManager appleEventForSuspensionID: — internal type
 * [eventkit] NSAppleEventManager replyAppleEventForSuspensionID: — internal type
 * [eventkit] NSAppleEventManager resumeWithSuspensionID: — internal type
 * [eventkit] NSAppleEventManager setCurrentAppleEventAndReplyEventWithSuspensionID: — internal type
 * [eventkit] NSAppleEventManager suspendCurrentAppleEvent — internal type
 * [foundation] NSAppleEventManager appleEventForSuspensionID: — internal type
 * [foundation] NSAppleEventManager replyAppleEventForSuspensionID: — internal type
 * [foundation] NSAppleEventManager resumeWithSuspensionID: — internal type
 * [foundation] NSAppleEventManager setCurrentAppleEventAndReplyEventWithSuspensionID: — internal type
 * [foundation] NSAppleEventManager suspendCurrentAppleEvent — internal type
 * [foundation] NSCoder decodeObjectOfClass:forKey: — apinotes nonswift
 * [foundation] NSError errorWithDomain:code:userInfo: — apinotes nonswift
 * [foundation] NSKeyedArchiver encodeInt:forKey: — apinotes nonswift
 * [foundation] NSKeyedUnarchiver decodeIntForKey: — apinotes nonswift
 * [foundation] NSNumber initWithLong: — apinotes nonswift
 * [foundation] NSNumber initWithUnsignedLong: — apinotes nonswift
 * [foundation] NSNumber longValue — apinotes nonswift
 * [foundation] NSNumber numberWithBool: — apinotes nonswift
 * [foundation] NSNumber numberWithChar: — apinotes nonswift
 * [foundation] NSNumber numberWithDouble: — apinotes nonswift
 * [foundation] NSNumber numberWithFloat: — apinotes nonswift
 * [foundation] NSNumber numberWithInt: — apinotes nonswift
 * [foundation] NSNumber numberWithInteger: — apinotes nonswift
 * [foundation] NSNumber numberWithLong: — apinotes nonswift
 * [foundation] NSNumber numberWithLongLong: — apinotes nonswift
 * [foundation] NSNumber numberWithShort: — apinotes nonswift
 * [foundation] NSNumber numberWithUnsignedChar: — apinotes nonswift
 * [foundation] NSNumber numberWithUnsignedInt: — apinotes nonswift
 * [foundation] NSNumber numberWithUnsignedInteger: — apinotes nonswift
 * [foundation] NSNumber numberWithUnsignedLong: — apinotes nonswift
 * [foundation] NSNumber numberWithUnsignedLongLong: — apinotes nonswift
 * [foundation] NSNumber numberWithUnsignedShort: — apinotes nonswift
 * [foundation] NSNumber unsignedLongValue — apinotes nonswift
 * [foundation] NSURL URLByResolvingBookmarkData:options:relativeToURL:bookmarkDataIsStale:error: — apinotes nonswift
 * [gameplaykit] CIContext createCGLayerWithSize:info: — internal type
 * [gameplaykit] NSAppleEventManager appleEventForSuspensionID: — internal type
 * [gameplaykit] NSAppleEventManager replyAppleEventForSuspensionID: — internal type
 * [gameplaykit] NSAppleEventManager resumeWithSuspensionID: — internal type
 * [gameplaykit] NSAppleEventManager setCurrentAppleEventAndReplyEventWithSuspensionID: — internal type
 * [gameplaykit] NSAppleEventManager suspendCurrentAppleEvent — internal type
 * [mapkit] NSAppleEventManager appleEventForSuspensionID: — internal type
 * [mapkit] NSAppleEventManager replyAppleEventForSuspensionID: — internal type
 * [mapkit] NSAppleEventManager resumeWithSuspensionID: — internal type
 * [mapkit] NSAppleEventManager setCurrentAppleEventAndReplyEventWithSuspensionID: — internal type
 * [mapkit] NSAppleEventManager suspendCurrentAppleEvent — internal type
 * [metal] NSAppleEventManager appleEventForSuspensionID: — internal type
 * [metal] NSAppleEventManager replyAppleEventForSuspensionID: — internal type
 * [metal] NSAppleEventManager resumeWithSuspensionID: — internal type
 * [metal] NSAppleEventManager setCurrentAppleEventAndReplyEventWithSuspensionID: — internal type
 * [metal] NSAppleEventManager suspendCurrentAppleEvent — internal type
 * [metalkit] MDLTexture writeToURL:type: — internal type
 * [metalkit] MDLTexture writeToURL:type:level: — internal type
 * [metalkit] MTKTextureLoader newTextureWithCGImage:options:completionHandler: — invalid generated type
 * [metalkit] MTKTextureLoader newTextureWithCGImage:options:error: — invalid generated type
 * [metalkit] MTKTextureLoader newTextureWithContentsOfURL:options:completionHandler: — invalid generated type
 * [metalkit] MTKTextureLoader newTextureWithContentsOfURL:options:error: — invalid generated type
 * [metalkit] MTKTextureLoader newTextureWithData:options:completionHandler: — invalid generated type
 * [metalkit] MTKTextureLoader newTextureWithData:options:error: — invalid generated type
 * [metalkit] MTKTextureLoader newTextureWithMDLTexture:options:completionHandler: — invalid generated type
 * [metalkit] MTKTextureLoader newTextureWithMDLTexture:options:error: — invalid generated type
 * [metalkit] MTKTextureLoader newTextureWithName:scaleFactor:bundle:options:completionHandler: — invalid generated type
 * [metalkit] MTKTextureLoader newTextureWithName:scaleFactor:bundle:options:error: — invalid generated type
 * [metalkit] MTKTextureLoader newTextureWithName:scaleFactor:displayGamut:bundle:options:completionHandler: — invalid generated type
 * [metalkit] MTKTextureLoader newTextureWithName:scaleFactor:displayGamut:bundle:options:error: — invalid generated type
 * [metalkit] MTKTextureLoader newTexturesWithContentsOfURLs:options:completionHandler: — invalid generated type
 * [metalkit] MTKTextureLoader newTexturesWithContentsOfURLs:options:error: — invalid generated type
 * [metalkit] MTKTextureLoader newTexturesWithNames:scaleFactor:bundle:options:completionHandler: — invalid generated type
 * [metalkit] MTKTextureLoader newTexturesWithNames:scaleFactor:displayGamut:bundle:options:completionHandler: — invalid generated type
 * [metalkit] NSAppleEventManager appleEventForSuspensionID: — internal type
 * [metalkit] NSAppleEventManager replyAppleEventForSuspensionID: — internal type
 * [metalkit] NSAppleEventManager resumeWithSuspensionID: — internal type
 * [metalkit] NSAppleEventManager setCurrentAppleEventAndReplyEventWithSuspensionID: — internal type
 * [metalkit] NSAppleEventManager suspendCurrentAppleEvent — internal type
 * [pdfkit] NSAppleEventManager appleEventForSuspensionID: — internal type
 * [pdfkit] NSAppleEventManager replyAppleEventForSuspensionID: — internal type
 * [pdfkit] NSAppleEventManager resumeWithSuspensionID: — internal type
 * [pdfkit] NSAppleEventManager setCurrentAppleEventAndReplyEventWithSuspensionID: — internal type
 * [pdfkit] NSAppleEventManager suspendCurrentAppleEvent — internal type
 * [pdfkit] PDFDocument documentRef — incomplete struct return type
 * [quartzcore] CIContext createCGLayerWithSize:info: — internal type
 * [quartzcore] NSAppleEventManager appleEventForSuspensionID: — internal type
 * [quartzcore] NSAppleEventManager replyAppleEventForSuspensionID: — internal type
 * [quartzcore] NSAppleEventManager resumeWithSuspensionID: — internal type
 * [quartzcore] NSAppleEventManager setCurrentAppleEventAndReplyEventWithSuspensionID: — internal type
 * [quartzcore] NSAppleEventManager suspendCurrentAppleEvent — internal type
 * [quicklook] NSAppleEventManager appleEventForSuspensionID: — internal type
 * [quicklook] NSAppleEventManager replyAppleEventForSuspensionID: — internal type
 * [quicklook] NSAppleEventManager resumeWithSuspensionID: — internal type
 * [quicklook] NSAppleEventManager setCurrentAppleEventAndReplyEventWithSuspensionID: — internal type
 * [quicklook] NSAppleEventManager suspendCurrentAppleEvent — internal type
 * [scenekit] CIContext createCGLayerWithSize:info: — internal type
 * [scenekit] NSAppleEventManager appleEventForSuspensionID: — internal type
 * [scenekit] NSAppleEventManager replyAppleEventForSuspensionID: — internal type
 * [scenekit] NSAppleEventManager resumeWithSuspensionID: — internal type
 * [scenekit] NSAppleEventManager setCurrentAppleEventAndReplyEventWithSuspensionID: — internal type
 * [scenekit] NSAppleEventManager suspendCurrentAppleEvent — internal type
 * [spritekit] NSAppleEventManager appleEventForSuspensionID: — internal type
 * [spritekit] NSAppleEventManager replyAppleEventForSuspensionID: — internal type
 * [spritekit] NSAppleEventManager resumeWithSuspensionID: — internal type
 * [spritekit] NSAppleEventManager setCurrentAppleEventAndReplyEventWithSuspensionID: — internal type
 * [spritekit] NSAppleEventManager suspendCurrentAppleEvent — internal type
 * [storekit] NSAppleEventManager appleEventForSuspensionID: — internal type
 * [storekit] NSAppleEventManager replyAppleEventForSuspensionID: — internal type
 * [storekit] NSAppleEventManager resumeWithSuspensionID: — internal type
 * [storekit] NSAppleEventManager setCurrentAppleEventAndReplyEventWithSuspensionID: — internal type
 * [storekit] NSAppleEventManager suspendCurrentAppleEvent — internal type
 * [webkit] NSAppleEventManager appleEventForSuspensionID: — internal type
 * [webkit] NSAppleEventManager replyAppleEventForSuspensionID: — internal type
 * [webkit] NSAppleEventManager resumeWithSuspensionID: — internal type
 * [webkit] NSAppleEventManager setCurrentAppleEventAndReplyEventWithSuspensionID: — internal type
 * [webkit] NSAppleEventManager suspendCurrentAppleEvent — internal type
 */

#endif
