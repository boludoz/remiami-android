#include <jni.h>
#include <unistd.h>
#include <cstdlib>
#include <android/log.h>
#if defined(LIBRW_SDL3)
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_hints.h>
#endif
#include "android.h"
#define JNI_WRAPPER extern "C" __attribute__ ((visibility("default")))
FILE* logfile = nullptr;

extern void InitCrashHandler();
extern int main(int argc, char *argv[]);

extern "C" JNIEXPORT void JNICALL
Java_com_sh0zer_revc_LauncherActivity_setenv(JNIEnv *env, jobject obj, jstring value)
{
    __android_log_print(ANDROID_LOG_DEBUG,"REVC-DEBUG", "Java_com_sh0zer_LauncherActivity_setenv %s",env->GetStringUTFChars(value, NULL));
    setenv("GAMEFILES", env->GetStringUTFChars(value, NULL), 1);
}

JNI_WRAPPER int LaunchAndroid(){
    InitCrashHandler();
    char logPath[256];
    snprintf(logPath, sizeof(logPath), "%s/gamelog.txt", getenv("GAMEFILES"));
    logfile = fopen(logPath, "w+");
    setvbuf(logfile, nullptr, _IONBF, 0);
    __android_log_print(ANDROID_LOG_DEBUG, "REVC", "TRYING TO START");
    int argc = 0;
    char *argv[1] = { nullptr };
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
    int result = main(argc, argv);
    return result;
}
