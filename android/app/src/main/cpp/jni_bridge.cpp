#include <jni.h>
#include <cstdlib>
#include <string>

extern "C" {

JNIEXPORT void JNICALL
Java_com_revc_LauncherActivity_setenv(JNIEnv *env, jobject thiz, jstring path) {
    const char *pathStr = env->GetStringUTFChars(path, nullptr);
    if (pathStr) {
        setenv("GAMEFILES", pathStr, 1);
        env->ReleaseStringUTFChars(path, pathStr);
    }
}

}
