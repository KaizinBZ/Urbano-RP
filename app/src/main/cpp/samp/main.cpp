#include <jni.h>
#include <pthread.h>
#include <atomic>
#include <syscall.h>

#include "main.h"
#include "game/game.h"

#include "net/netgame.h"
#include "gui/gui.h"
#include "gui/nexioncontrols.h"
#include "gui/nexion_minimap.h"
#include "gui/nexiondeath.h"
#include "gui/FilaAcoesHud.h"
#include "gui/DadosVeiculoImperio.h"
#include <cmath>
#include "gui/CicloArmas.h"

#include "playertags.h"
#include "audiostream.h"
#include "java/jniutil.h"
#include <dlfcn.h>
#include <sys/mman.h>
#include <inttypes.h>
#include "StackTrace.h"

// voice
#include "voice_new/Plugin.h"

#include "vendor/armhook/patch.h"
#include "vendor/str_obfuscator/str_obfuscator.hpp"

#include "settings.h"

#include "crashlytics.h"

/*
Peerapol Unarak
*/

static imperio::FilaAcoesHud g_filaAcoesImperio;
static imperio::CaixaVeiculo g_veiculoImperio;

JavaVM* javaVM;

char* g_pszStorage = nullptr;

UI* pUI = nullptr;
CGame *pGame = nullptr;

CNetGame *pNetGame = nullptr;
CPlayerTags* pPlayerTags = nullptr;
CSnapShotHelper* pSnapShotHelper = nullptr;
CAudioStream* pAudioStream = nullptr;
CJavaWrapper* pJavaWrapper = nullptr;
CSettings* pSettings = nullptr;
//CVoice* pVoice = nullptr;

MaterialTextGenerator* pMaterialTextGenerator = nullptr;

bool bDebug = false;
bool bGameInited = false;
bool bNetworkInited = false;

uintptr_t g_libGTASA = 0x00;
uintptr_t g_libSAMP = 0x00;

void ApplyGlobalPatches();
void ApplyPatches_level0();
void ApplyMultiTouchPatches();
void InstallGlobalHooks();
void InstallSpecialHooks();
void InitRenderWareFunctions();
void FLog(const char* fmt, ...);
//void MyLog(const char* fmt, ...);

int work = 0;

void ReadSettingFile()
{
    /*char path[255] = { 0 };
    //sprintf(path, "%ssamp.set", g_pszStorage);
    sprintf(path, "%sNickName.ini", g_pszStorage);

    FILE* fp = fopen(path, "r");
    if (fp == NULL) return;

    char buf[1024];

    // nickname
    if (fgets(buf, 1024, fp) != NULL) {
        buf[strcspn(buf, "\n\r")] = 0;
        strcpy(g_nick, buf);
    }

    fclose(fp);*/

    pSettings = new CSettings();

    firebase::crashlytics::SetUserId(pSettings->Get().szNickName);
}

int hashing(const char* str) {
    int hashing = 5381;
    int c;
    while (c = *str++) {
        hashing = ((hashing << 5) + hashing) + c; /* hash * 33 + c */
        if (hashing < 0) hashing = 100;
    }
    if (hashing < 0) hashing = 100;
    return hashing;
}

void DoDebugLoop()
{
    // ...
}

void DoDebugStuff()
{
    // ...

    RwMatrix mat = pGame->FindPlayerPed()->m_pPed->GetMatrix().ToRwMatrix();

    for (int i = 0; i < 100; i++)
    {
        CPlayerPed* ped = pGame->NewPlayer(i, mat.pos.x + i, mat.pos.y, mat.pos.z, 0.0f, false, false);
        //ped->SetCollisionChecking(false);
        //ped->SetGravityProcessing(false);
    }
}
struct sigaction act_old;
struct sigaction act1_old;
struct sigaction act2_old;
struct sigaction act3_old;

extern int g_iLastProcessedSkinCollision, g_iLastProcessedEntityCollision, g_iLastRenderedObject;
extern uintptr_t g_dwLastRetAddrCrash;
void handler(int signum, siginfo_t *info, void* contextPtr)
{
    ucontext* context = (ucontext_t*)contextPtr;

    if (act_old.sa_sigaction)
    {
        act_old.sa_sigaction(signum, info, contextPtr);
    }

    if(info->si_signo == SIGSEGV)
    {
        FLog("SIGSEGV | Fault address: 0x%x", info->si_addr);

        PRINT_CRASH_STATES(context);

        CStackTrace::printBacktrace();
    }

    return;
}

void handler1(int signum, siginfo_t *info, void* contextPtr)
{
    ucontext* context = (ucontext_t*)contextPtr;

    if (act1_old.sa_sigaction)
    {
        act1_old.sa_sigaction(signum, info, contextPtr);
    }

    if(info->si_signo == SIGABRT)
    {
        FLog("SIGABRT | Fault address: 0x%x", info->si_addr);

        PRINT_CRASH_STATES(context);

        CStackTrace::printBacktrace();
    }

    return;
}

void handler2(int signum, siginfo_t *info, void* contextPtr)
{
    ucontext* context = (ucontext_t*)contextPtr;

    if (act2_old.sa_sigaction)
    {
        act2_old.sa_sigaction(signum, info, contextPtr);
    }

    if(info->si_signo == SIGFPE)
    {
        FLog("SIGFPE | Fault address: 0x%x", info->si_addr);

        PRINT_CRASH_STATES(context);

        CStackTrace::printBacktrace();
    }

    return;
}

void handler3(int signum, siginfo_t *info, void* contextPtr)
{
    ucontext* context = (ucontext_t*)contextPtr;

    if (act3_old.sa_sigaction)
    {
        act3_old.sa_sigaction(signum, info, contextPtr);
    }

    if(info->si_signo == SIGBUS)
    {
        FLog("SIGBUS | Fault address: 0x%x", info->si_addr);

        PRINT_CRASH_STATES(context);

        CStackTrace::printBacktrace();
    }

    return;
}

void RestartGameConnection()
{
    // O jogador escolheu reiniciar: volta para a tela inicial do launcher.
    // NÃO escondemos o painel de "CONEXÃO PERDIDA" aqui: o processo vai ser
    // encerrado (System.exit) no Java e o painel continua cobrindo a tela até
    // o app sair — assim o mundo do GTA não aparece atrás da launcher.
    if (pJavaWrapper)
    {
        pJavaWrapper->GoToLauncher();
    }
}

void DoInitStuff()
{
    if (bGameInited == false)
    {
        // Start the loading sequence on Java side
        if (pJavaWrapper) {
            pJavaWrapper->StartLoadingSequence();
        }

        pPlayerTags = new CPlayerTags();
        pSnapShotHelper = new CSnapShotHelper();
        pMaterialTextGenerator = new MaterialTextGenerator();
        pAudioStream = new CAudioStream();
        pAudioStream->Initialize();

        pUI->splashscreen()->setVisible(false);
        pUI->chat()->setVisible(true);
        pUI->buttonpanel()->setVisible(true);

        pGame->Initialize();
        pGame->SetMaxStats();
        pGame->ToggleThePassingOfTime(false);

        // Notify Java that game resources are ready
        if (pJavaWrapper) {
            pJavaWrapper->OnGameReady();
        }

        // voice
        /*LogVoice("[dbg:samp:load] : module loading...");

        for (const auto& loadCallback : Samp::loadCallbacks) {
            if (loadCallback != nullptr) {
                loadCallback();
            }
        }

        Samp::loadStatus = true;*/

        LogVoice("[dbg:samp:load] : module loaded");

        if (bDebug)
        {
            CCamera& TheCamera = *reinterpret_cast<CCamera*>(g_libGTASA + (VER_x32 ? 0x00951FA8 : 0xBBA8D0));
            //TheCamera.Restore();
            CCamera::SetBehindPlayer();
                        // Mesmo no caminho de depuração, nunca restaurar a camada HUD/mapa nativa.
            pGame->DisplayHUD(false);

            pGame->EnableClock(false);

            DoDebugStuff();
        }

        bGameInited = true;
    }

    if (!bNetworkInited && !bDebug)
    {
        // Notify Java that network connection is starting
        if (pJavaWrapper) {
            pJavaWrapper->OnNetworkReady();
        }

        //ReadSettingFile();

        static const unsigned char encodedHost[] = {107, 109, 99, 116, 107, 99, 98, 116, 107, 106, 111, 116, 107, 108, 109};
        std::string serverHost;
        serverHost.reserve(sizeof(encodedHost));
        for (unsigned char value : encodedHost) serverHost.push_back(static_cast<char>(value ^ 0x5A));
        pNetGame = new CNetGame(serverHost.c_str(), 7101, pSettings->Get().szNickName, pSettings->Get().szPassword);
        bNetworkInited = true;

        FLog("DoInitStuff end");
    }
}

// Apenas na thread do jogo. Sem offsets fixos: se o símbolo não estiver visível,
// não chama nada. Os offsets são validados contra as duas libGTASA desta base;
// mesmo assim, o comportamento real ainda requer teste em Android.
static_assert(__builtin_offsetof(CPedGTA, m_aWeapons)==(VER_x32 ? 0x5a4 : 0x730), "layout de armas divergente");
static_assert(__builtin_offsetof(CPedGTA, m_nActiveWeaponSlot)==(VER_x32 ? 0x71c : 0x8dc), "layout de slot divergente");
static bool ImperioProximaArma() {
    if (!pGame || !g_libGTASA) return false;
    CPlayerPed* jogador = pGame->FindPlayerPed();
    if (!jogador || !jogador->m_pPed || jogador->IsDead()) return false;
    CPedGTA* ped=jogador->m_pPed;
    const int ativo=ped->m_nActiveWeaponSlot;
    if (ativo<0 || ativo>=13) return false;
    std::array<imperio::SlotArma,13> slots{};
    for (int i=0;i<13;++i) {
        slots[i].tipo=static_cast<uint32_t>(ped->m_aWeapons[i].dwType);
        slots[i].total=ped->m_aWeapons[i].dwAmmo;
    }
    const int escolhido=imperio::proximoSlotArma(ativo,slots);
    if(escolhido<0 || escolhido==ativo) return false;
    using Mudar=void (*)(CPedGTA*,int);
    if (!CHook::lib) return false;
    auto mudar=reinterpret_cast<Mudar>(dlsym(CHook::lib,"_ZN4CPed16SetCurrentWeaponEi"));
    if(!mudar)return false;
    mudar(ped,escolhido);
    return true;
}

extern "C" {
JNIEXPORT void JNICALL Java_com_nexion_online_SAMP_initializeSAMP(JNIEnv *pEnv, jobject thiz)
{
    pJavaWrapper = new CJavaWrapper(pEnv, thiz);

}
JNIEXPORT jboolean JNICALL Java_com_nexion_online_SAMP_nativeEnfileirarAcaoImperio(JNIEnv*, jobject, jint id)
{
    using A = imperio::FilaAcoesHud::Acao;
    A a;
    switch (id) {
        case 0: a=A::Menu; break;
        case 1: a=A::Entrar; break;
        case 2: a=A::Sair; break;
        case 3: a=A::Interagir; break;
        case 4: a=A::Obra; break;
        case 5: a=A::TrocarArma; break;
        case 6: a=A::Motor; break;
        case 7: a=A::Farol; break;
        default: return JNI_FALSE;
    }
    return g_filaAcoesImperio.adicionar(a) ? JNI_TRUE : JNI_FALSE;
}
JNIEXPORT jfloatArray JNICALL Java_com_nexion_online_SAMP_nativeLerVeiculoImperio(JNIEnv* env, jobject)
{
    const auto d=g_veiculoImperio.snapshot();
    const jfloat valores[7]={d.presente?1.0f:0.0f,d.motorista?1.0f:0.0f,d.velocidade,d.saude,static_cast<float>(d.motor),static_cast<float>(d.farol),static_cast<float>(d.id)};
    jfloatArray resultado=env->NewFloatArray(7);
    if (resultado) env->SetFloatArrayRegion(resultado,0,7,valores);
    return resultado;
}
JNIEXPORT jboolean JNICALL Java_com_nexion_online_SAMP_nativeVisibilidadeRadarImperio(JNIEnv*, jobject, jlong session, jboolean visivel)
{
    return NexionMiniMap::SetImperioRadarVisivel(session,visivel==JNI_TRUE) ? JNI_TRUE : JNI_FALSE;
}
JNIEXPORT jboolean JNICALL Java_com_nexion_online_SAMP_nativeChatImperio(JNIEnv*, jobject, jlong session, jboolean oculto)
{
    return NexionMiniMap::SetImperioChatRecolhido(session,oculto==JNI_TRUE) ? JNI_TRUE : JNI_FALSE;
}
JNIEXPORT jboolean JNICALL Java_com_nexion_online_SAMP_nativeIconesImperio(JNIEnv* env, jobject, jlong session, jfloatArray arr)
{
    if (!arr) return JNI_FALSE;
    const jsize n=env->GetArrayLength(arr);
    if(n<0 || n>3600 || n%3) return JNI_FALSE;
    std::vector<float> values(static_cast<size_t>(n));
    if(n) env->GetFloatArrayRegion(arr,0,n,values.data());
    if(env->ExceptionCheck()) return JNI_FALSE;
    return NexionMiniMap::SetImperioPontos(session,values.data(),values.size()) ? JNI_TRUE : JNI_FALSE;
}
JNIEXPORT jlong JNICALL Java_com_nexion_online_SAMP_nativeAssetsImperio(JNIEnv* env, jobject, jobject assets, jlong session)
{
    return NexionMiniMap::SetImperioAssets(env, assets, session);
}
JNIEXPORT void JNICALL Java_com_nexion_online_SAMP_nativeRadarImperio(JNIEnv*, jobject, jlong session, jint x, jint y, jint r, jint w, jint h)
{
    NexionMiniMap::SetImperioViewport(session,x,y,r,w,h);
}
JNIEXPORT void JNICALL Java_com_nexion_online_SAMP_nativeEstadoSistemaImperio(JNIEnv*, jobject, jboolean ativo)
{
    g_filaAcoesImperio.sistema(ativo == JNI_TRUE);
    g_veiculoImperio.sistema(ativo == JNI_TRUE);
}
JNIEXPORT void JNICALL Java_com_nexion_online_GTASA_onApplicationPaused(JNIEnv *pEnv, jobject thiz)
{
    g_filaAcoesImperio.sistema(false);
    g_veiculoImperio.sistema(false);
    NexionControls::Reset();
}

JNIEXPORT void JNICALL Java_com_nexion_online_SAMP_onInputEnd(JNIEnv *pEnv, jobject thiz, jbyteArray str)
{
    if(pUI)
    {
        pUI->keyboard()->sendForGB(pEnv, thiz, str);
    }
}
JNIEXPORT void JNICALL Java_com_nexion_online_SAMP_onEventBackPressed(JNIEnv *pEnv, jobject thiz)
{
    if(pSettings)
    {
        if(pSettings->Get().iAndroidKeyboard)
            pJavaWrapper->HideKeyboard();
    }
}

JNIEXPORT void JNICALL Java_com_nexion_online_SAMP_nativeSetKeyboardOffset(JNIEnv *pEnv, jobject thiz, jint offset)
{
    if(pUI)
    {
        pUI->setKeyboardOffset((int)offset);
    }
}

JNIEXPORT void JNICALL Java_com_nexion_online_SAMP_nativeUpdateKeyboardInput(JNIEnv *pEnv, jobject thiz, jstring text)
{
    if(!pUI || !pUI->keyboard() || !pUI->keyboard()->input()) return;

    const char* chars = pEnv->GetStringUTFChars(text, nullptr);
    if(!chars) return;

    std::string input(chars);
    pUI->keyboard()->input()->setInputString(input);

    pEnv->ReleaseStringUTFChars(text, chars);
}
JNIEXPORT void JNICALL Java_com_nexion_online_ui_dialog_DialogManager_sendDialogResponse(JNIEnv* pEnv, jobject thiz, jint i3, jint i, jint i2, jbyteArray str)
{
    jboolean isCopy = true;

    jbyte* pMsg = pEnv->GetByteArrayElements(str, &isCopy);
    jsize length = pEnv->GetArrayLength(str);

    std::string szStr((char*)pMsg, length);

    if(pNetGame) {
        pNetGame->SendDialogResponse(i, i3, i2, (char*)szStr.c_str());
        //pGame->FindPlayerPed()->TogglePlayerControllableWithoutLock(true);
    }

    pEnv->ReleaseByteArrayElements(str, pMsg, JNI_ABORT);
}
}

void MainLoop()
{
    if (pGame->bIsGameExiting) { g_veiculoImperio.limpar(); return; }
    DoInitStuff();

    if (bDebug) {
        DoDebugLoop();
    }

    if (pNetGame) {
        pNetGame->Process();
    }
    // Cópia segura na thread do jogo; a UI apenas lê snapshot protegido.
    static uint32_t ultimaAmostra=0;
    const uint32_t agora=GetTickCount();
    if (agora-ultimaAmostra >= 100) {
        ultimaAmostra=agora;
        bool ok=pGame && pNetGame && pNetGame->GetGameState()==GAMESTATE_CONNECTED &&
            pNetGame->GetVehiclePool() && pNetGame->GetPlayerPool() &&
            pNetGame->GetPlayerPool()->GetLocalPlayer() &&
            pNetGame->GetPlayerPool()->GetLocalPlayer()->m_bIsActive && !NexionDeath::IsPlayerDead();
        CPlayerPed* jogador=ok?pGame->FindPlayerPed():nullptr;
        CVehicle* v=nullptr;
        int id=-1;
        if (jogador && jogador->m_pPed && jogador->IsInVehicle()) {
            auto* gta=jogador->GetGtaVehicle();
            auto* pool=pNetGame->GetVehiclePool();
            if (gta && pool) for (int i=1;i<MAX_VEHICLES;i++) {
                CVehicle* candidato=pool->GetAt(static_cast<VEHICLEID>(i));
                if (candidato && candidato->m_pVehicle==gta) {v=candidato;id=i;break;}
            }
        }
        if (v && v->m_pVehicle) {
            CVector vel=v->m_pVehicle->GetMoveSpeed();
            float kmh=std::sqrt(vel.x*vel.x+vel.y*vel.y+vel.z*vel.z)*180.0f;
            g_veiculoImperio.atualizar(true,!jogador->IsAPassenger(),id,kmh,v->GetHealth(),v->GetEngineState(),v->GetLightState());
        } else g_veiculoImperio.limpar();
    }
    // Envio apenas na thread do jogo, após rede processada. Não aceita strings
    // do servidor e descarta comandos ao deixar a sessão ou morrer.
    if (pNetGame && pNetGame->GetGameState() == GAMESTATE_CONNECTED &&
        pNetGame->GetPlayerPool() && pNetGame->GetPlayerPool()->GetLocalPlayer() &&
        pNetGame->GetPlayerPool()->GetLocalPlayer()->m_bIsActive && !NexionDeath::IsPlayerDead()) {
        g_filaAcoesImperio.jogo(true);
        for (const auto acao : g_filaAcoesImperio.retirar()) {
            if (acao == imperio::FilaAcoesHud::Acao::TrocarArma) {
                ImperioProximaArma();
                continue;
            }
            if (acao == imperio::FilaAcoesHud::Acao::Motor || acao == imperio::FilaAcoesHud::Acao::Farol) {
                CPlayerPed* jogador=pGame?pGame->FindPlayerPed():nullptr;
                if (!jogador || !jogador->IsInVehicle() || jogador->IsAPassenger()) continue;
            }
            const char* comando = imperio::FilaAcoesHud::comando(acao);
            if (comando) pNetGame->SendChatCommand(comando);
        }
    } else g_filaAcoesImperio.jogo(false);

    if (pAudioStream) {
        pAudioStream->Process();
    }
}

void InitGui()
{
    // new voice
    Plugin::OnPluginLoad();
    Plugin::OnSampLoad();

    std::string font_path = string_format("%sSAMP/fonts/%s", g_pszStorage, FONT_NAME);
    pUI = new UI(ImVec2(RsGlobal->maximumWidth, RsGlobal->maximumHeight), font_path.c_str());
    pUI->initialize();
    pUI->performLayout();
}

#include "game/multitouch.h"
#include "armhook/patch.h"
#include "util/CUtil.h"
void SetUpGLHooks();
jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    javaVM = vm;
    LOGI("SA-MP library loaded! Build time: " __DATE__ " " __TIME__);

    g_libGTASA = CUtil::FindLib("libGTASA.so");
    if (g_libGTASA == 0x00) {
        LOGE("libGTASA.so address was not found! ");
        return JNI_VERSION_1_6;
    }

    g_libSAMP = CUtil::FindLib("libsamp.so");
    if (g_libSAMP == 0x00) {
        LOGE("libsamp.so address was not found! ");
        return JNI_VERSION_1_6;
    }

    firebase::crashlytics::Initialize();

    uintptr_t libgtasa = CUtil::FindLib("libGTASA.so");
    uintptr_t libsamp = CUtil::FindLib("libsamp.so");
    uintptr_t libc = CUtil::FindLib("libc.so");

    FLog("libGTASA.so: 0x%x", libgtasa);
    FLog("libsamp.so: 0x%x", libsamp);
    FLog("libc.so: 0x%x", libc);

    char str[100];

    sprintf(str, "0x%x", libgtasa);
    firebase::crashlytics::SetCustomKey("libGTASA.so", str);

    sprintf(str, "0x%x", libsamp);
    firebase::crashlytics::SetCustomKey("libsamp.so", str);

    sprintf(str, "0x%x", libc);
    firebase::crashlytics::SetCustomKey("libc.so", str);

    CHook::InitHookStuff();
    InstallSpecialHooks();
    ApplyPatches_level0();
    //SetUpGLHooks();
    InitRenderWareFunctions();
    MultiTouch::initialize();

    pGame = new CGame();

    //pVoice = new CVoice();
    //pVoice->Initialize(VOICE_FREQUENCY, CODEC_FREQUENCY, VOICE_SENDRRATE);

    //pthread_t thread;
    //pthread_create(&thread, 0, Init, 0);

    struct sigaction act;
    act.sa_sigaction = handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &act, &act_old);

    struct sigaction act1;
    act1.sa_sigaction = handler1;
    sigemptyset(&act1.sa_mask);
    act1.sa_flags = SA_SIGINFO;
    sigaction(SIGABRT, &act1, &act1_old);

    struct sigaction act2;
    act2.sa_sigaction = handler2;
    sigemptyset(&act2.sa_mask);
    act2.sa_flags = SA_SIGINFO;
    sigaction(SIGFPE, &act2, &act2_old);

    struct sigaction act3;
    act3.sa_sigaction = handler3;
    sigemptyset(&act3.sa_mask);
    act3.sa_flags = SA_SIGINFO;
    sigaction(SIGBUS, &act3, &act3_old);

    return JNI_VERSION_1_6;
}

uint32_t GetTickCount()
{
    return CTimer::m_snTimeInMillisecondsNonClipped;
}

void FLog(const char* fmt, ...)
{
    char buffer[0xFF];
    static FILE* flLog = nullptr;
    const char* pszStorage = g_pszStorage;


    if (flLog == nullptr && pszStorage != nullptr)
    {
        sprintf(buffer, "%s/samp_log.txt", pszStorage);
        //LOGI("buffer: %s", buffer);
        flLog = fopen(buffer, "a");
    }

    memset(buffer, 0, sizeof(buffer));

    va_list arg;
    va_start(arg, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, arg);
    va_end(arg);

    LOGI("%s", buffer);
    firebase::crashlytics::Log(buffer);

    if (flLog == nullptr) return;
    fprintf(flLog, "%s\n", buffer);
    fflush(flLog);

    return;
}

void ChatLog(const char* fmt, ...)
{
    char buffer[0xFF];
    static FILE* flLog = nullptr;
    const char* pszStorage = g_pszStorage;


    if (flLog == nullptr && pszStorage != nullptr)
    {
        sprintf(buffer, "%s/chat_log.txt", pszStorage);
        flLog = fopen(buffer, "a");
    }

    memset(buffer, 0, sizeof(buffer));

    va_list arg;
    va_start(arg, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, arg);
    va_end(arg);

    if (flLog == nullptr) return;
    fprintf(flLog, "%s\n", buffer);
    fflush(flLog);

    return;
}

void MyLog(const char* fmt, ...)
{
    char buffer[0xFF];
    static FILE* flLog = nullptr;
    const char* pszStorage = g_pszStorage;


    if (flLog == nullptr && pszStorage != nullptr)
    {
        sprintf(buffer, "%s/samp_log.txt", pszStorage);
        //LOGI("buffer: %s", buffer);
        flLog = fopen(buffer, "a");
    }

    memset(buffer, 0, sizeof(buffer));

    va_list arg;
    va_start(arg, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, arg);
    va_end(arg);

    if (flLog == nullptr) return;
    fprintf(flLog, "%s\n", buffer);
    fflush(flLog);

    return;
}

void MyLog2(const char* fmt, ...)
{
    char buffer[0xFF];
    static FILE* flLog = nullptr;
    const char* pszStorage = g_pszStorage;


    if (flLog == nullptr && pszStorage != nullptr)
    {
        sprintf(buffer, "%s/samp_log.txt", pszStorage);
        //LOGI("buffer: %s", buffer);
        flLog = fopen(buffer, "a");
    }

    memset(buffer, 0, sizeof(buffer));

    va_list arg;
    va_start(arg, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, arg);
    va_end(arg);

    if (pUI) pUI->chat()->addDebugMessage(buffer);

    if (flLog == nullptr) return;
    fprintf(flLog, "%s\n", buffer);
    fflush(flLog);
    return;
}

void LogVoice(const char* fmt, ...)
{
    char buffer[0xFF];
    static FILE* flLog = nullptr;
    const char* pszStorage = g_pszStorage;

    if (flLog == nullptr && pszStorage != nullptr)
    {
        sprintf(buffer, "%sSAMP/%s", pszStorage, SV::kLogFileName);
        flLog = fopen(buffer, "w");
    }

    memset(buffer, 0, sizeof(buffer));

    va_list arg;
    va_start(arg, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, arg);
    va_end(arg);

    __android_log_write(ANDROID_LOG_INFO, "AXL", buffer);

    if (flLog == nullptr) return;
    fprintf(flLog, "%s\n", buffer);
    fflush(flLog);

    return;
}