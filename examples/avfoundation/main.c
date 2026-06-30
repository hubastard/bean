#include <AudioToolbox/AudioServices.h>
#include <QuartzCore/CABase.h>
#include <limits.h>
#include <libgen.h>
#include <mach-o/dyld.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <bean/avfoundation.h>

static AVAudioEngine g_audio_engine = NULL;
static AVAudioPlayerNode g_audio_player_node = NULL;
static AVAudioFile g_audio_file = NULL;

static NSString bean_avfoundation_sample_path(void) {
    static const char* fallbacks[] = {
        "examples/avfoundation/sample.wav",
        "sample.wav",
        NULL,
    };
    char exe_path[PATH_MAX];
    uint32_t size = (uint32_t)sizeof(exe_path);

    if (_NSGetExecutablePath(exe_path, &size) == 0) {
        char dir_path[PATH_MAX];
        strncpy(dir_path, exe_path, sizeof(dir_path));
        dir_path[sizeof(dir_path) - 1] = '\0';
        {
            char sample_path[PATH_MAX];
            snprintf(sample_path, sizeof(sample_path), "%s/sample.wav", dirname(dir_path));
            if (access(sample_path, R_OK) == 0) {
                return bean_str(sample_path);
            }
        }
    }

    for (size_t i = 0; fallbacks[i] != NULL; ++i) {
        if (access(fallbacks[i], R_OK) == 0) {
            return bean_str(fallbacks[i]);
        }
    }

    return bean_str("examples/avfoundation/sample.wav");
}

static double bean_avfoundation_file_duration(AVAudioFile file) {
    AVAudioFormat format = AVAudioFile_ProcessingFormat(file);
    double sample_rate = (double)AVAudioFormat_SampleRate(format);
    if (sample_rate <= 0.0) {
        return 0.0;
    }
    return (double)AVAudioFile_Length(file) / sample_rate;
}

static void bean_avfoundation_pump_run_loop_until(double deadline) {
    while (CACurrentMediaTime() < deadline) {
        NSRunLoop_RunUntilDate(
            NSRunLoop_CurrentRunLoop(),
            NSDate_DateWithTimeIntervalSinceNow(0.05));
    }
}

static void bean_avfoundation_release_engine(void) {
    if (g_audio_engine) {
        AVAudioEngine_Stop(g_audio_engine);
    }
    g_audio_engine = NULL;
    g_audio_player_node = NULL;
    g_audio_file = NULL;
}

static BOOL bean_avfoundation_play_with_engine(NSURL url, double* out_duration) {
    AVAudioFile file;
    AVAudioEngine engine;
    AVAudioPlayerNode player_node;
    AVAudioFormat format;
    NSError error;
    double duration;
    double deadline;

    error = NULL;
    file = AVAudioFile_InitForReading_Error(
        AVAudioFile_Alloc(),
        url,
        &error);
    if (!file) {
        const char* message = "unknown error";
        if (error) {
            message = bean_nsstring_to_utf8(NSError_LocalizedDescription(error));
        }
        fprintf(stderr, "AVAudioFile init failed: %s\n", message ? message : "unknown error");
        return NO;
    }

    duration = bean_avfoundation_file_duration(file);
    if (duration <= 0.0) {
        fprintf(stderr, "AVAudioFile has zero duration.\n");
        return NO;
    }

    engine = AVAudioEngine_New();
    AVAudioEngine_SetAutoShutdownEnabled(engine, NO);

    player_node = AVAudioPlayerNode_New();

    AVAudioEngine_AttachNode(engine, (AVAudioNode)player_node);
    format = AVAudioFile_ProcessingFormat(file);
    AVAudioEngine_Connect_To_Format(
        engine,
        (AVAudioNode)player_node,
        (AVAudioNode)AVAudioEngine_MainMixerNode(engine),
        format);

    error = NULL;
    if (!AVAudioEngine_StartAndReturnError(engine, &error)) {
        const char* message = "unknown error";
        if (error) {
            message = bean_nsstring_to_utf8(NSError_LocalizedDescription(error));
        }
        fprintf(stderr, "AVAudioEngine start failed: %s\n", message ? message : "unknown error");
        return NO;
    }

    g_audio_engine = engine;
    g_audio_player_node = player_node;
    g_audio_file = file;

    printf(
        "AVFoundation AVAudioEngine: %.2fs, %.0f Hz\n",
        duration,
        (double)AVAudioFormat_SampleRate(format));

    AVAudioPlayerNode_ScheduleFile_AtTime_CompletionHandler(player_node, file, NULL, NULL);
    AVAudioPlayerNode_Play(player_node);

    deadline = CACurrentMediaTime() + duration + 0.1;
    bean_avfoundation_pump_run_loop_until(deadline);
    bean_avfoundation_release_engine();

    if (out_duration) {
        *out_duration = duration;
    }
    return YES;
}

static BOOL bean_avfoundation_play_with_audio_services(NSURL url, double duration) {
    SystemSoundID sound_id = 0;
    OSStatus status = AudioServicesCreateSystemSoundID((CFURLRef)(void*)url, &sound_id);

    if (status != noErr) {
        fprintf(stderr, "AudioServicesCreateSystemSoundID failed: %d\n", (int)status);
        return NO;
    }

    printf("AudioToolbox AudioServices: playing the same WAV via Core Audio services\n");
    AudioServicesPlaySystemSound(sound_id);

    if (duration > 0.0) {
        usleep((useconds_t)(duration * 1000000.0));
    }

    AudioServicesDisposeSystemSoundID(sound_id);
    return YES;
}

int main(void) {
    autoreleasepool {
        NSString path = bean_avfoundation_sample_path();
        NSURL url = NSURL_FileURLWithPath(path);
        double duration = 0.0;

        printf("AVFoundation sample: %s\n", bean_nsstring_to_utf8(path));

        if (!bean_avfoundation_play_with_engine(url, &duration)) {
            return 1;
        }

        if (!bean_avfoundation_play_with_audio_services(url, duration)) {
            return 1;
        }

        printf("Done.\n");
    }
    return 0;
}
