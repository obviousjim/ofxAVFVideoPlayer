

//
//  ofxAVFVideoRenderer.h
//  AVFoundationTest
//
//  Created by Sam Kronick on 5/31/13.
//
//


#import <Cocoa/Cocoa.h>
#import <Quartz/Quartz.h>
#import <AVFoundation/AVFoundation.h>
#import <OpenGL/OpenGL.h>

@interface AVFVideoRenderer : NSObject
{
    AVPlayer *player;
    AVPlayerItem *playerItem;
    AVPlayerLayer *playerLayer;
    AVAssetReader *assetReader;
    
    NSMutableData *amplitudes;
    int numAmplitudes;
    
    CARenderer *layerRenderer;
    
    CGSize videoSize;
    CMTime videoDuration;
    
    BOOL loading;
    BOOL ready;
    BOOL audioReady;
    BOOL deallocWhenReady;
    
    id periodicTimeObserver;
}
    @property (nonatomic, retain) AVPlayer *player;
    @property (nonatomic, retain) AVPlayerItem *playerItem;
    @property (nonatomic, retain) AVPlayerLayer *playerLayer;
    @property (nonatomic, retain) AVAssetReader *assetReader;
    @property (nonatomic, retain) CARenderer *layerRenderer;

    @property (nonatomic, retain) NSMutableData *amplitudes;
    @property (nonatomic, assign) int numAmplitudes;

- (void) loadFile:(NSString *)filename;
- (void) play;
- (void) stop;
- (void) playerItemDidReachEnd:(NSNotification *) notification;
//- (void) update;
- (BOOL) isReady;
- (BOOL) isAudioReady;
- (BOOL) isLoading;
- (void) render;

//- (void)postProcessAmplitude:(float)damping;

- (CGSize) getVideoSize;
- (CMTime) getVideoDuration;

@end
