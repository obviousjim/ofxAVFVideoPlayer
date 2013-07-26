//
//  ofxAVFVideoRenderer.m
//  AVFoundationTest
//
//  Created by Sam Kronick on 5/31/13.
//
//

#import "ofxAVFVideoRenderer.h"

@implementation AVFVideoRenderer
@synthesize player, playerItem, playerLayer, assetReader, layerRenderer;
@synthesize amplitudes, numAmplitudes, maxAmplitude;

int count = 0;

- (id)init
{
    self = [super init];
    if (self) {
        amplitudes = [[NSMutableData data] retain];
    }
    return self;
}

- (void) loadFile:(NSString *)filename {
    loading = YES;
    ready = NO;
    audioReady = NO;
    deallocWhenReady = NO;
    //NSURL *fileURL = [NSURL URLWithString:filename];
    NSURL *fileURL = [NSURL fileURLWithPath:[filename stringByStandardizingPath]];
    
    if (amplitudes) {
        [amplitudes setLength:0];
    }
    numAmplitudes = 0;
    maxAmplitude = 0;
    
    NSLog(@"Trying to load %@", filename);
    
    AVURLAsset *asset = [AVURLAsset URLAssetWithURL:fileURL options:nil];
    NSString *tracksKey = @"tracks";
    
    [asset loadValuesAsynchronouslyForKeys:@[tracksKey] completionHandler: ^{
        static const NSString *ItemStatusContext;
        // Perform the following back on the main thread
        dispatch_async(dispatch_get_main_queue(), ^{
            NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
            // Check to see if the file loaded
            NSError *error;
            AVKeyValueStatus status = [asset statusOfValueForKey:tracksKey error:&error];
            
            
            if(status == AVKeyValueStatusLoaded) {
                // Asset metadata has been loaded. Set up the player
                
                // Extract the video track to get the video size
                AVAssetTrack *videoTrack = [[asset tracksWithMediaType:AVMediaTypeVideo] objectAtIndex:0];
                videoSize = [videoTrack naturalSize];
                videoDuration = asset.duration;
                
                self.playerItem = [AVPlayerItem playerItemWithAsset:asset];
                [self.playerItem addObserver:self forKeyPath:@"status" options:0 context:&ItemStatusContext];
                
                // Notify this object when the player reaches the end
                // This allows us to loop the video
                [[NSNotificationCenter defaultCenter] addObserver:self
                                                         selector:@selector(playerItemDidReachEnd:)
                                                             name:AVPlayerItemDidPlayToEndTimeNotification
                                                           object:self.playerItem];
                
                self.player = [AVPlayer playerWithPlayerItem:self.playerItem];
                
                self.playerLayer = [AVPlayerLayer playerLayerWithPlayer:player];
                
                self.layerRenderer = [CARenderer rendererWithCGLContext:CGLGetCurrentContext() options:nil];
                self.layerRenderer.layer = playerLayer;
                
                // Video is centered on 0,0 for some reason so layer bounds have to start at -width/2,-height/2
                self.layerRenderer.bounds = CGRectMake(-videoSize.width/2, -videoSize.height/2, videoSize.width, videoSize.height);
                self.playerLayer.bounds = self.layerRenderer.bounds;
                                
                AVAssetTrack *audioTrack = [[asset tracksWithMediaType:AVMediaTypeAudio] objectAtIndex:0];
                NSError *error = nil;
                assetReader = [[AVAssetReader alloc] initWithAsset:asset error:&error];
                if (error!=nil) {
                    NSLog(@"Unable to create asset reader %@", [error localizedDescription]);
                } else {
                    NSMutableDictionary *bufferOptions = [NSMutableDictionary dictionary];
                    [bufferOptions setObject:[NSNumber numberWithInt:kAudioFormatLinearPCM] forKey:AVFormatIDKey];
//                                        [bufferOptions setObject:[NSNumber numberWithInt:44100] forKey:AVSampleRateKey];
//                                        [bufferOptions setObject:[NSNumber numberWithInt:2] forKey:AVNumberOfChannelsKey];
                    //  [bufferOptions setObject:[NSData dataWithBytes:&channelLayout length:sizeof(AudioChannelLayout)] forKey:AVChannelLayoutKey];
//                                        [bufferOptions setObject:[NSNumber numberWithInt:16] forKey:AVLinearPCMBitDepthKey];
//                                        [bufferOptions setObject:[NSNumber numberWithBool:NO] forKey:AVLinearPCMIsBigEndianKey];
//                                        [bufferOptions setObject:[NSNumber numberWithBool:NO] forKey:AVLinearPCMIsFloatKey];
//                                        [bufferOptions setObject:[NSNumber numberWithBool:NO] forKey:AVLinearPCMIsNonInterleaved];
                    [assetReader addOutput:[AVAssetReaderTrackOutput assetReaderTrackOutputWithTrack:audioTrack
                                                                                      outputSettings:bufferOptions]];
                    [assetReader startReading];
                }
                if (audioTrack != nil) {
                    periodicTimeObserver = [player addPeriodicTimeObserverForInterval:CMTimeMake(1001, [audioTrack nominalFrameRate] * 1001)
                                                                                queue:dispatch_queue_create("eventQueue", NULL)
                                                                           usingBlock:^(CMTime time) {
                        if ([assetReader status] == AVAssetReaderStatusCompleted) {
                            // Got all the data we need, kill this block.
                            [player removeTimeObserver:periodicTimeObserver];
//                            [self postProcessAmplitude:200];
                            audioReady = YES;
                            
                            return;
                        }
                        
                        if ([assetReader status] == AVAssetReaderStatusReading) {
                            AVAssetReaderTrackOutput *output = [[assetReader outputs] objectAtIndex:0];
                            CMSampleBufferRef sampleBuffer = [output copyNextSampleBuffer];
                            
                            
                            while( sampleBuffer != NULL ) {
                                sampleBuffer = [output copyNextSampleBuffer];
                                
                                if( sampleBuffer == NULL )
                                    continue;
                                
                                CMBlockBufferRef buffer = CMSampleBufferGetDataBuffer( sampleBuffer );
                                
                                size_t lengthAtOffset;
                                size_t totalLength;
                                char* data;
                                
                                if( CMBlockBufferGetDataPointer( buffer, 0, &lengthAtOffset, &totalLength, &data ) != noErr )
                                {
                                    NSLog( @"error!" );
                                    break;
                                }
                                
                                CMItemCount numSamplesInBuffer = CMSampleBufferGetNumSamples(sampleBuffer);
                                
                                AudioBufferList audioBufferList;
                                
                                CMSampleBufferGetAudioBufferListWithRetainedBlockBuffer(
                                                                                        sampleBuffer,
                                                                                        NULL,
                                                                                        &audioBufferList,
                                                                                        sizeof(audioBufferList),
                                                                                        NULL,
                                                                                        NULL,
                                                                                        kCMSampleBufferFlag_AudioBufferList_Assure16ByteAlignment,  // pass in something else
                                                                                        &buffer
                                                                                        );
                                
                                for (int bufferCount=0; bufferCount < audioBufferList.mNumberBuffers; bufferCount++) {
                                    SInt16* samples = (SInt16 *)audioBufferList.mBuffers[bufferCount].mData;
                                    
                                    numAmplitudes += numSamplesInBuffer;
                                    
                                    for (int i = 0; i < numSamplesInBuffer; i++) {
                                        float amp = samples[i];
                                        [amplitudes appendBytes:&amp length:sizeof(float)];
                                        maxAmplitude = MAX(maxAmplitude, ABS(amp));
                                    }
                                }
                                
                                CFRelease( buffer );
                                CFRelease( sampleBuffer );
                            }
                        }
                    }];
                }

                ready = YES;
                loading = NO;
            }
            else {
                loading = NO;
                ready = NO;
                NSLog(@"There was an error loading the file:\n%@", error);
            }
            
            // If dealloc is called immediately after loadFile, we have to defer releasing properties
            if(deallocWhenReady) [self dealloc];
            [pool release];
        });
    }];
}

- (void) dealloc {
    if(loading) {
        deallocWhenReady = YES;
    }
    else {
        [self stop];
        
        // SK: Releasing the CARenderer is slow for some reason
        //     It will freeze the main thread for a few dozen mS.
        //     If you're swapping in and out videos a lot, the loadFile:
        //     method should be re-written to just re-use and re-size
        //     these layers/objects rather than releasing and reallocating
        //     them every time a new file is needed.
        
        if(self.layerRenderer) [self.layerRenderer release];
        
        [[NSNotificationCenter defaultCenter] removeObserver:self];
        if(self.playerItem) [self.playerItem removeObserver:self forKeyPath:@"status"];
        
        if(self.player) [self.player release];
        if(self.playerItem) [self.playerItem release];
        if(self.playerLayer) [self.playerLayer release];
        
        if (amplitudes) [amplitudes release];
        numAmplitudes = 0;
        
        if(!deallocWhenReady) [super dealloc];
    }
}

- (BOOL) isLoading { return loading; }
- (BOOL) isReady { return ready; }
- (BOOL) isAudioReady { return audioReady; }

- (CGSize) getVideoSize {
    return videoSize;
}

- (CMTime) getVideoDuration {
    return videoDuration;
}

- (void) play {
    [self.player play];
}

- (void) stop {
    [self.player pause];
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
    
}

- (void) playerItemDidReachEnd:(NSNotification *) notification {
    [self.player seekToTime:kCMTimeZero];
    // if(loop)
    //[self.player play];
}

- (void) render {
    // From https://qt.gitorious.org/qt/qtmultimedia/blobs/700b4cdf42335ad02ff308cddbfc37b8d49a1e71/src/plugins/avfoundation/mediaplayer/avfvideoframerenderer.mm
    
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_DEPTH_TEST);
    
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glViewport(0, 0, videoSize.width, videoSize.height);
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    
    glOrtho(0.0f, videoSize.width, videoSize.height, 0.0f, 0.0f, 1.0f);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glTranslatef(videoSize.width/2,videoSize.height/2,0);
    
    [layerRenderer beginFrameAtTime:CACurrentMediaTime() timeStamp:NULL];
    [layerRenderer addUpdateRect:layerRenderer.layer.bounds];
    [layerRenderer render];
    [layerRenderer endFrame];
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glPopAttrib();
    
    glFinish(); //Rendering needs to be done before passing texture to video frame
}

//- (void) postProcessAmplitude:(float)damping
//{
//    float newmaxAmplitude = 0;
//    
//    for (int i = 0; i < numAmplitudes; i++) {
//        float avg = 0;
//        if (i < damping / 2) {
//            for (int j = 0; j < damping; j++) {
//                avg += numAmplitudes;
//            }
//        }
//        else if (i > numAmplitudes - damping / 2 - 1) {
//            for (int j = numAmplitudes - 1 - damping; j < numAmplitudes - 1; j++) {
//                avg += amplitudes[j];
//            }
//        }
//        else {
//            for (int j = i - damping / 2; j <  i + damping / 2; j++) {
//                avg += amplitudes[j];
//            }
//        }
//        
//        avg /= damping;
//        
//        newmaxAmplitude = MAX(newmaxAmplitude, ABS(avg));
//        amplitudes[i] = avg;
//    }
//    
//    dispatch_sync(dispatch_get_main_queue(), ^{
//        maxAmplitude = newmaxAmplitude;
//    });
//}

@end
