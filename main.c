#include <taihen.h>
#include <psp2/ctrl.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/modulemgr.h>

#define DeclareHook(type, func_name, ...)\
    static SceUID func_name ## _UID;\
    static tai_hook_ref_t func_name ## _ref;\
    type func_name ## _hook (__VA_ARGS__)

#define ReleaseHook(func_name)\
    if (func_name ## _UID > 0) taiHookRelease(func_name ## _UID, func_name ## _ref)

const int FRAMES_DELAY = 10; //Minimum number of calls to wait before combos are usable again
const SceCtrlButtons inOutCombo = SCE_CTRL_LTRIGGER | SCE_CTRL_RTRIGGER | SCE_CTRL_CIRCLE;      //L + R + O   => open/close debug menu (if current game mode is 0)
const SceCtrlButtons L3_emu_buttons = SCE_CTRL_LTRIGGER | SCE_CTRL_RTRIGGER | SCE_CTRL_LEFT;    //L + R + <-  => emulate L3 press (does nothing)
const SceCtrlButtons R3_emu_buttons = SCE_CTRL_LTRIGGER | SCE_CTRL_RTRIGGER | SCE_CTRL_RIGHT;   //L + R + ->  => emulate R3 press (does nothing)
int *pGAMEMODE = (int*)0x8197CFC8;
unsigned int* pPRESSED_KEYS = (unsigned int*)0x818D15A4; //unsure tbh

static int ctrl_mtx = 0;
static int counter = 0;
DeclareHook(int, sceCtrlPeekBufferPositive, int port, SceCtrlData* pData, int bufcount){
    int ret = TAI_CONTINUE(int, sceCtrlPeekBufferPositive_ref, port, pData, bufcount);
    if (ret < 0) return ret;
    if (!ctrl_mtx){
        if (counter > 0){
            counter--;
            return ret;
        }
        ctrl_mtx = 1; //lock "mutex"

        SceCtrlData data;
        int ret2 = sceCtrlPeekBufferPositive(0, &data, 1);
        if (ret2 < 0){
            sceClibPrintf("RC1Dbg : couldn't PeekBuffer ! ret = 0x%08X\n", ret2);
            return ret;
        }

        if ( (data.buttons & inOutCombo) == inOutCombo){
            sceClibPrintf("RC1Dbg : in/out combo pressed !\n");
            int gameMode = *pGAMEMODE;
            sceClibPrintf("Current game mode : %i\n",gameMode);
            if (gameMode == 0){
                gameMode = -1;
            } 
            else if (gameMode == -1) {
                    gameMode = 0;
            }
            sceClibPrintf("New game mode : %i\n",gameMode);
            *pGAMEMODE = gameMode;
            counter = FRAMES_DELAY;
        }
        else if ( (data.buttons & L3_emu_buttons) == L3_emu_buttons){
            sceClibPrintf("RC1Dbg : L3 emu combo pressed !\n");
            sceClibPrintf("Currently pressed keys : 0x%08X\n",*pPRESSED_KEYS);
            *pPRESSED_KEYS |= 0x200;
            sceClibPrintf("Patched : 0x%08X\n",*pPRESSED_KEYS);
            data.buttons |= SCE_CTRL_L3;
            counter = FRAMES_DELAY;
        }
        else if ( (data.buttons & R3_emu_buttons) == R3_emu_buttons){
            sceClibPrintf("RC1Dbg : R3 emu combo pressed !\n");
            sceClibPrintf("Currently pressed keys : 0x%08X\n",*pPRESSED_KEYS);
            *pPRESSED_KEYS |= 0x400;
            sceClibPrintf("Patched : 0x%08X\n",*pPRESSED_KEYS);
            data.buttons |= SCE_CTRL_R3;
            counter = FRAMES_DELAY;
        }

        ctrl_mtx = 0; //unlock "mutex"
    }
    return ret;
}

void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize argc, const void* args){
    sceCtrlPeekBufferPositive_UID = taiHookFunctionImport(&sceCtrlPeekBufferPositive_ref, 
                                                                TAI_MAIN_MODULE, 
                                                                0xD197E3C7 /* SceCtrl */, 
                                                                0xA9C3CED6 /* SceCtrlPeekBufferPositive */, 
                                                                sceCtrlPeekBufferPositive_hook);
    sceClibPrintf("RC1Dbg : taiHook return = 0x%08X\n",sceCtrlPeekBufferPositive_UID);
    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void* args){
    ReleaseHook(sceCtrlPeekBufferPositive);
    return SCE_KERNEL_STOP_SUCCESS;
}