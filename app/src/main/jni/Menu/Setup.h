#include <sstream>
#include "Menu/Menu.h"
#include "Menu/get_device_api_level_inlines.h"

//Jni stuff from MrDarkRX https://github.com/MrDarkRXx/DarkMod-Floating
void setDialog(jobject ctx, JNIEnv *env, const char *title, const char *msg){
    jclass Alert = env->FindClass(AY_OBFUSCATE("android/app/AlertDialog$Builder"));
    jmethodID AlertCons = env->GetMethodID(Alert, AY_OBFUSCATE("<init>"), AY_OBFUSCATE("(Landroid/content/Context;)V"));

    jobject MainAlert = env->NewObject(Alert, AlertCons, ctx);

    jmethodID setTitle = env->GetMethodID(Alert, AY_OBFUSCATE("setTitle"), AY_OBFUSCATE("(Ljava/lang/CharSequence;)Landroid/app/AlertDialog$Builder;"));
    env->CallObjectMethod(MainAlert, setTitle, env->NewStringUTF(title));

    jmethodID setMsg = env->GetMethodID(Alert, AY_OBFUSCATE("setMessage"), AY_OBFUSCATE("(Ljava/lang/CharSequence;)Landroid/app/AlertDialog$Builder;"));
    env->CallObjectMethod(MainAlert, setMsg, env->NewStringUTF(msg));

    jmethodID setCa = env->GetMethodID(Alert, AY_OBFUSCATE("setCancelable"), AY_OBFUSCATE("(Z)Landroid/app/AlertDialog$Builder;"));
    env->CallObjectMethod(MainAlert, setCa, false);

    jmethodID setPB = env->GetMethodID(Alert, AY_OBFUSCATE("setPositiveButton"), AY_OBFUSCATE("(Ljava/lang/CharSequence;Landroid/content/DialogInterface$OnClickListener;)Landroid/app/AlertDialog$Builder;"));
    env->CallObjectMethod(MainAlert, setPB, env->NewStringUTF("Ok"), static_cast<jobject>(NULL));

    jmethodID create = env->GetMethodID(Alert, AY_OBFUSCATE("create"), AY_OBFUSCATE("()Landroid/app/AlertDialog;"));
    jobject creaetob = env->CallObjectMethod(MainAlert, create);

    jclass AlertN = env->FindClass(AY_OBFUSCATE("android/app/AlertDialog"));

    jmethodID show = env->GetMethodID(AlertN, AY_OBFUSCATE("show"), AY_OBFUSCATE("()V"));
    env->CallVoidMethod(creaetob, show);
}

void Toast(JNIEnv *env, jobject thiz, const char *text, int length) {
    jstring jstr = env->NewStringUTF(text);
    jclass toast = env->FindClass(AY_OBFUSCATE("android/widget/Toast"));
    jmethodID methodMakeText =env->GetStaticMethodID(toast,AY_OBFUSCATE("makeText"),AY_OBFUSCATE("(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;"));
    jobject toastobj = env->CallStaticObjectMethod(toast, methodMakeText,thiz, jstr, length);
    jmethodID methodShow = env->GetMethodID(toast, AY_OBFUSCATE("show"), AY_OBFUSCATE("()V"));
    env->CallVoidMethod(toastobj, methodShow);
}

void startActivityPermisson(JNIEnv *env, jobject ctx){
    jclass native_context = env->GetObjectClass(ctx);
    jmethodID startActivity = env->GetMethodID(native_context, AY_OBFUSCATE("startActivity"),AY_OBFUSCATE("(Landroid/content/Intent;)V"));

    jmethodID pack = env->GetMethodID(native_context, AY_OBFUSCATE("getPackageName"),AY_OBFUSCATE("()Ljava/lang/String;"));
    jstring packageName = static_cast<jstring>(env->CallObjectMethod(ctx, pack));

    const char *pkg = env->GetStringUTFChars(packageName, 0);

    std::stringstream pkgg;
    pkgg << AY_OBFUSCATE("package:");
    pkgg << pkg;
    std::string pakg = pkgg.str();

    jclass Uri = env->FindClass(AY_OBFUSCATE("android/net/Uri"));
    jmethodID Parce = env->GetStaticMethodID(Uri, AY_OBFUSCATE("parse"), AY_OBFUSCATE("(Ljava/lang/String;)Landroid/net/Uri;"));
    jobject UriMethod = env->CallStaticObjectMethod(Uri, Parce, env->NewStringUTF(pakg.c_str()));

    jclass intentclass = env->FindClass(AY_OBFUSCATE("android/content/Intent"));
    jmethodID newIntent = env->GetMethodID(intentclass, AY_OBFUSCATE("<init>"), AY_OBFUSCATE("(Ljava/lang/String;Landroid/net/Uri;)V"));
    jobject intent = env->NewObject(intentclass,newIntent,env->NewStringUTF(AY_OBFUSCATE("android.settings.action.MANAGE_OVERLAY_PERMISSION")), UriMethod);

    env->CallVoidMethod(ctx, startActivity, intent);
}

void startService(JNIEnv *env, jobject ctx){
    jclass native_context = env->GetObjectClass(ctx);
    jclass intentClass = env->FindClass(AY_OBFUSCATE("android/content/Intent"));
    jclass actionString = env->FindClass(AY_OBFUSCATE("com/android/support/Launcher"));
    jmethodID newIntent = env->GetMethodID(intentClass, AY_OBFUSCATE("<init>"), AY_OBFUSCATE("(Landroid/content/Context;Ljava/lang/Class;)V"));
    jobject intent = env->NewObject(intentClass,newIntent,ctx,actionString);
    jmethodID startActivityMethodId = env->GetMethodID(native_context, AY_OBFUSCATE("startService"), AY_OBFUSCATE("(Landroid/content/Intent;)Landroid/content/ComponentName;"));
    env->CallObjectMethod(ctx, startActivityMethodId, intent);
}

void *exit_thread(void *) {
    sleep(5);
    exit(0);
}

//Needed jclass parameter because this is a static java method
void CheckOverlayPermission(JNIEnv *env, jclass thiz, jobject ctx){
    //If overlay permission option is greyed out, make sure to add android.permission.SYSTEM_ALERT_WINDOW in manifest

    LOGI(AY_OBFUSCATE("Check overlay permission"));

    int sdkVer = api_level();
    if (sdkVer >= 23){ //Android 6.0
        jclass Settings = env->FindClass(AY_OBFUSCATE("android/provider/Settings"));
        jmethodID canDraw =env->GetStaticMethodID(Settings, AY_OBFUSCATE("canDrawOverlays"), AY_OBFUSCATE("(Landroid/content/Context;)Z"));
        if (!env->CallStaticBooleanMethod(Settings, canDraw, ctx)){
            Toast(env,ctx,AY_OBFUSCATE("Overlay permission is required in order to show mod menu."),1);
            startActivityPermisson(env, ctx);

            pthread_t ptid;
            pthread_create(&ptid, NULL, exit_thread, NULL);
            return;
        }
    }


    LOGI(AY_OBFUSCATE("Start service"));

    //StartMod Normal
    startService(env, ctx);
}

void Init(JNIEnv *env, jobject thiz, jobject ctx, jobject title, jobject subtitle){
    //Set sub title
    setText(env, title, AY_OBFUSCATE("<b>Hearthstone MixMod</b>"));

    //Set sub title
    setText(env, subtitle, AY_OBFUSCATE("<b><marquee><p style=\"font-size:30\">"
                                     "<p style=\"color:green;\">Hearthstone MixMod</p> | "
                                     "https://t.me/HsMixMod | Official telegram channel for updates</p>"
                                     "</marquee></b>"));

    //Dialog Example
    //setDialog(ctx,env,AY_OBFUSCATE("Title"),AY_OBFUSCATE("Message Example"));

    //Toast Example
    //Toast(env,ctx,AY_OBFUSCATE("Modded by YOU"),ToastLength::LENGTH_LONG);

    initValid = true;
}