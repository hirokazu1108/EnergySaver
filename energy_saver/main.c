#include "header.h"
#include <SDL2/SDL.h>
#include <joyconlib.h>
#include <stdio.h>
#include <string.h>

//タイマー関数の静的宣言
static Uint32 AniTimer(Uint32 interval, void* param);
static Uint32 SpawnTimer(Uint32 interval, void* param);
static Uint32 SecondTimer(Uint32 interval, void* param);
static Uint32 JoyConTimer(Uint32 interval, void* param);


static SDL_bool InputEvent(void);
static SDL_bool SelectInput(void);
static SDL_bool InputEventJoycon(void);
static SDL_bool SelectInputJoyCon(void);

SDL_TimerID atimer, btimer, ctimer;
int framecnt;

//JoyConのキー入力の連続性を防ぐ変数
int repeatFlag = 1; // trueならキー入力を受付,falseならSecondTimer内のカウントを行う
int repeatNum  = 0; // repeatFlag==falseの時カウントして数秒後repeatFlagをtrueにする

//操作モード
int playMode = 0;   //0ならキーボード,1ならJoyCon

//JoyCon(R)の宣言
joyconlib_t jc_r;



// main関数
int main(int argc, char *argv[])
{   

    //コマンドライン引数によるJoyConモードの設定
    if(argc>1){
        /* joyconモードの設定 */
        if(!strcmp(argv[1],"joycon")){
            playMode = 1;
        }
    }

    /** ゲームシステム初期化 **/
    if (InitSystem("data/chara.data", "data/map.data", "data/item.data") < 0) {
        PrintError("failed to initialize System");
        goto DESTROYSYSTEM;
    }

    /** UI初期化 **/
    if (InitWindow() < 0) {
        PrintError("failed to initialize Windows");
        goto DESTROYWINDOW;
    }
    if (LoadSound() < 0) {
        PrintError("failed to load sounds");
        goto DESTROYAUDIO;
    }

    /** JoyConのオープン **/
    if(playMode == 1){
        if ((joycon_open(&jc_r, JOYCON_R)) == JOYCON_ERR_OPEN_FAILED) {
            PrintError("failed to open JoyCon_R");
            goto DESTROYWINDOW;
        }
    }
    


    //　spawnTimeの定義
    gGame.spawnTime[G_Easy]   = 3000;
    gGame.spawnTime[G_Normal] = 2500;
    gGame.spawnTime[G_Hard]   = 2000;

    /** タイマー起動 **/
    SDL_TimerID dtimer = SDL_AddTimer(400, JoyConTimer, &framecnt);
    if (dtimer == 0) {
        PrintError(SDL_GetError());
        goto DESTROYALL;
    }

    /** メイン処理 **/
    /* ゲーム開始 */
    gGame.stts = GS_Title;
    /* メインループ */
    while (gGame.stts != GS_End) {
        /** ゲーム制御 **/

        /* ゲーム中 */
        while (gGame.stts == GS_Playing) {
            if ((playMode==0 ? InputEvent() : InputEventJoycon()) == SDL_FALSE) {
                gGame.stts = GS_End;
            }
            for (int i = 0; i < gCharaNum; i++) {
                MoveChara(&(gChara[i]), 1);
                UpdateCharaInfo(&(gChara[i]));
            }
            /** 当たり判定 **/
            for (int i = 0; i < gCharaNum; i++) {
                for (int j = i + 1; j < gCharaNum; j++)
                    Collision(&(gChara[i]), &(gChara[j]));
            }
            /* 終了条件の判定 */
            if (Judge()) {
                RenderWindow();
                gGame.clearTime = gGame.secondTime;
                SDL_Delay(2000);
            }

            /** 描画処理 **/
            RenderWindow();
            // TestRender();
            /* 少し待つ
             *  PC環境によって遅く(速く)なる時に調整してください
             *  削除してしまうと，デバッグ時などで動作が重くなるので
             *  消さない方がよいと思います
             */

            SDL_Delay(10);
            framecnt++;

            if (gGame.stts != GS_Playing) {
                SDL_RemoveTimer(atimer);
                SDL_RemoveTimer(btimer);
                SDL_RemoveTimer(ctimer);

                gGame.input.down  = SDL_FALSE;
                gGame.input.up    = SDL_FALSE;
                gGame.input.left  = SDL_FALSE;
                gGame.input.right = SDL_FALSE;
                int num           = gCharaNum - 1;
                while (num--)
                    RemoveChara(1);
                for (int i = 0; i < gItemNum; i++) {
                    if (gObj[i].type == OnPortal)
                        PortalSwitch(i, 0, 1);
                }
                for (int i = 0; i < gItemNum; i++) {
                    gItem[i].isSet = 0;
                    gItem[i].isUse = 1;
                }
                CreateCharaInfo(&(gChara[gGame.player]), 0);
                Mix_HaltMusic(); // 再生中のサウンドを停止
            }
        }
        /* タイトルシーン */
        while (gGame.stts == GS_Title) {
            if ((playMode==0 ? SelectInput() : SelectInputJoyCon()) == SDL_FALSE) {
                gGame.stts = GS_End;
            }
            RenderTitleWindow();
        }
        /* 難易度選択シーン */
        while (gGame.stts == GS_Select) {
            if ((playMode==0 ? SelectInput() : SelectInputJoyCon()) == SDL_FALSE) {
                gGame.stts = GS_End;
            }
            RenderSelectWindow();
        }
        /* リザルトシーン */
        while (gGame.stts == GS_Result) {
            if ((playMode==0 ? SelectInput() : SelectInputJoyCon()) == SDL_FALSE) {
                gGame.stts = GS_End;
            }
            RenderResultWindow();
        }
    }

    /** 終了処理 **/
    SDL_RemoveTimer(atimer);
    SDL_RemoveTimer(btimer);
    SDL_RemoveTimer(ctimer);
    SDL_RemoveTimer(dtimer);

DESTROYALL:
    if(playMode == 1)
        joycon_close(&jc_r);
DESTROYAUDIO:
    for (int i = 0; i < SeNum; i++) {
        Mix_FreeChunk(se[i]);
        se[i] = NULL;
    }
    Mix_CloseAudio(); // Mix_OpenAudioに対応
DESTROYWINDOW:
    DestroyWindow();
DESTROYSYSTEM:
    DestroySystem();
    SDL_Quit();

    return 0;
}

/* エラーメッセージ表示
 *
 * 引数
 *   str: エラーメッセージ
 *
 * 返値: -1
 */
int PrintError(const char* str)
{
    fprintf(stderr, "%s\n", str);
    return -1;
}

/** タイマー処理(アニメーションの更新) **/
Uint32 AniTimer(Uint32 interval, void* param)
{
    /* 時間増分の更新
     *  1フレームの時間を簡易計算する
     */
    if (*(int*)param > 0) {
        gGame.timeStep = 0.1 / *(int*)param;
        printf("FPS: %d\r", *(int*)param * 10);
        *(int*)param = 1;
    }

    /* 転送元範囲の更新(アニメーション) */
    for (int i = 0; i < gCharaNum; i++) {
        /* アニメーションパターンの更新 */
        gChara[i].anipat = (gChara[i].anipat + 1) % (gChara[i].image->anipatnum);
    }

    return interval;
}

/** タイマー処理(スポナー) **/
Uint32 SpawnTimer(Uint32 interval, void* param)
{
    if (gCharaNum < 11) {
        if (CreateCharaInfo(&(gChara[gCharaNum]), rand() % (gBaseCharaNum - 1) + 1) == 1)
            gCharaNum++;
    }
    return interval;
}
/** タイマー処理(1秒ごとに呼ばれる) **/
Uint32 SecondTimer(Uint32 interval, void* param)
{
    gGame.secondTime++;

    // アイテムの処理
    for (int i = 0; i < gItemNum; i++) {
        if (!gItem[i].isUse) {
            if (gGame.secondTime - gItem[i].setTime >= itemEffectTime[gItem[i].type]) {
                gItem[i].isSet = 0;
            }
            if (gGame.secondTime - gItem[i].setTime >= itemReloadTime[gItem[i].type] + gGame.mode) {
                gItem[i].isUse = 1;
                itemNum[gItem[i].type]++;
            }
        }
    }
    return interval;
}

/** タイマー処理(JoyConの連続認識の防止) **/
Uint32 JoyConTimer(Uint32 interval, void* param)
{
    // JoyConのキー入力の連続性を防ぐ
    if (!repeatFlag) {
        repeatNum++;
        if (repeatNum >= 1) {
            repeatNum  = 0;
            repeatFlag = 1;
        }
    }
    return interval;
}

/* ゲーム中の入力（イベント）読取
 *
 * 返値
 *   終了させる: 0
 *   継続する  : 1
 */
SDL_bool InputEvent(void)
{
    //SDL_Eventの定義
    SDL_Event event;

    int index = -1;

    while(SDL_PollEvent(&event)){
        switch (event.type) {
            case SDL_KEYDOWN:
                switch(event.key.keysym.sym){
                    case SDLK_a:
                        gGame.input.left = SDL_TRUE;
                        gGame.input.right = SDL_FALSE;
                        break;
                    case SDLK_d:
                        gGame.input.right = SDL_TRUE;
                        gGame.input.left = SDL_FALSE;
                        break;
                    case SDLK_w:
                        gGame.input.up = SDL_TRUE;
                        gGame.input.down = SDL_FALSE;
                        break;
                    case SDLK_s:
                        gGame.input.down = SDL_TRUE;
                        gGame.input.up = SDL_FALSE;
                        break;
                    case SDLK_RETURN:
                        index = IsTriggerByObject(&(gChara[gGame.player]), &overlap, 0.0);
                        if (index != -1) {
                            if (gObj[index].type == OnPortal || gObj[index].type == OffPortal) {
                                if (CulcAngle(DiffVec(gChara[gGame.player].pos, gObj[index].pos), gChara[gGame.player].dir) <= M_PI / 3) {
                                    Mix_PlayChannel(2, se[SE_SwitchPortal], 0); //スイッチ音を1回再生
                                    PortalSwitch(index, -1, 1);
                                }
                            }
                            if (gObj[index].type == Crystal) {
                                gGame.energy += gGame.isOn;
                                if (gGame.isOn > 0) {
                                    Mix_PlayChannel(2, se[SE_Energy], 0); //エネルギ貯蓄音を1回再生
                                }
                                gGame.isOn = 0;
                                for (int i = 0; i < ObjNum; i++)
                                    if (gObj[i].type == OnPortal || gObj[i].type == OffPortal) {
                                        PortalSwitch(i, 0, 0);
                                    }
                            }
                        }
                        break;
                    case SDLK_SPACE:
                        UseItem();
                        break;
                    case SDLK_KP_7:
                        gGame.selectItemNum = 0;
                        UseItem();
                        break;
                    case SDLK_KP_8:
                        gGame.selectItemNum = 1;
                        UseItem();
                        break;
                    case SDLK_KP_9:
                        gGame.selectItemNum = 2;
                        UseItem();
                        break;
                    case SDLK_KP_4:
                        gGame.selectItemNum = 3;
                        UseItem();
                        break;
                    case SDLK_KP_5:
                        gGame.selectItemNum = 4;
                        UseItem();
                        break;
                    case SDLK_RIGHT:
                        ChangeItem(+1);
                        break;
                    case SDLK_LEFT:
                        ChangeItem(-1);
                        break;
                    case SDLK_q:
                        gGame.stts = GS_Title;
                        break;
                    case SDLK_ESCAPE:
                        gGame.stts = GS_Title;
                        break;      
                }
                break;
            case SDL_KEYUP:
                gGame.input.up = SDL_FALSE;
                gGame.input.down = SDL_FALSE;
                gGame.input.right = SDL_FALSE;
                gGame.input.left = SDL_FALSE;
                break;
        }
    }

    return SDL_TRUE;
}

/* メインゲームシーン以外のシーンでの入力 */
SDL_bool SelectInput(void)
{
    //SDL_Eventの定義
    SDL_Event event;

    while(SDL_PollEvent(&event)){
        //タイトルシーン
        if (gGame.stts == GS_Title) {
            switch (event.type) {
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_w:
                            if (manualFlag == 0) {
                                SelectButton(&titleButtonNum, -1, 3); //上に移動
                            }else if (manualFlag == 1){
                                if (manualPos.y > 0)
                                    manualPos.y -= 100; //上に移動
                            }
                            break;
                        case SDLK_UP:
                            if (manualFlag == 0) {
                                SelectButton(&titleButtonNum, -1, 3); //上に移動
                            }else if (manualFlag == 1){
                                if (manualPos.y > 0)
                                    manualPos.y -= 100; //上に移動
                            }
                            break;
                        case SDLK_s:
                            if (manualFlag == 0) {
                                SelectButton(&titleButtonNum, +1, 3); //下に移動
                            }else if (manualFlag == 1){
                                if (450 + manualPos.y < 1632)
                                    manualPos.y += 100; //下に移動
                            }
                            break;
                        case SDLK_DOWN:
                            if (manualFlag == 0) {
                                SelectButton(&titleButtonNum, +1, 3); //下に移動
                            }else if (manualFlag == 1){
                                if (450 + manualPos.y < 1632)
                                    manualPos.y += 100; //下に移動
                            }
                            break;
                        case SDLK_RETURN:
                            if (manualFlag == 0) {
                                if (titleButtonNum == 0) {
                                    gGame.stts = GS_Select;
                                }
                                if (titleButtonNum == 1) {
                                    manualFlag = 1;
                                }
                                if (titleButtonNum == 2) {
                                    SDL_Delay(1000);
                                    gGame.stts = GS_End;
                                }
                                Mix_PlayChannel(1, se[SE_Click], 0); //クリックを1回再生
                            }else if (manualFlag == 1){
                                manualFlag  = 0;
                                manualPos.x = manualPos.y = 0;
                            }
                            break;
                        case SDLK_q:
                            gGame.stts = GS_End;
                            break;
                        case SDLK_ESCAPE:
                            gGame.stts = GS_End;
                            break;     
                        
                }
            }
        }else if (gGame.stts == GS_Select){ //難易度選択シーン
            switch (event.type) {
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_w:
                            SelectButton(&modeButtonNum, -1, 3); //上に移動
                            break;
                        case SDLK_UP:
                            SelectButton(&modeButtonNum, -1, 3); //上に移動
                            break;
                        case SDLK_s:
                            SelectButton(&modeButtonNum, +1, 3); //下に移動
                            break;
                        case SDLK_DOWN:
                            SelectButton(&modeButtonNum, +1, 3); //下に移動
                            break;
                        case SDLK_RETURN:
                            if (modeButtonNum == 0) {
                                gGame.mode = G_Easy;
                            } else if (modeButtonNum == 1) {
                                gGame.mode = G_Normal;
                            } else if (modeButtonNum == 2) {
                                gGame.mode = G_Hard;
                            }
                            Mix_PlayChannel(1, se[SE_Click], 0); //クリックを1回再生
                            SDL_Delay(1000);
                            gGame.stts = GS_Playing;
                            InitInfo(&gGame);
                            for (int i = 0; i < ItemImgNum; i++)
                                itemNum[i] = itemMaxNum[i];
                            atimer   = SDL_AddTimer(100, AniTimer, &framecnt);
                            btimer   = SDL_AddTimer(gGame.spawnTime[gGame.mode], SpawnTimer, NULL);
                            ctimer   = SDL_AddTimer(1000, SecondTimer, NULL);
                            framecnt = 0;
                            RenderWindow();
                            Mix_RewindMusic(); // 先頭まで巻き戻す
                            int randNum = rand() % 2;
                            Mix_PlayMusic(bgm[randNum], -1); // Music型サウンドを無限に再生
                            Mix_VolumeMusic(MIX_MAX_VOLUME);
                            break;
                        case SDLK_q:
                            gGame.stts = GS_Title;
                            break;
                        case SDLK_ESCAPE:
                            gGame.stts = GS_Title;
                            break;
                    }         
            }
        }else if (gGame.stts == GS_Result){ //リザルトシーン
            switch (event.type) {
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_a:
                            SelectButton(&resultButtonNum, -1, 2); //左に移動
                            break;
                        case SDLK_LEFT:
                            SelectButton(&resultButtonNum, -1, 2); //左に移動
                            break;
                        case SDLK_d:
                            SelectButton(&resultButtonNum, +1, 2); //右に移動
                            break;
                        case SDLK_RIGHT:
                            SelectButton(&resultButtonNum, +1, 2); //右に移動
                            break;
                        case SDLK_RETURN:
                            if (resultButtonNum == 0) {
                                gGame.stts = GS_Title;
                            }
                            if (resultButtonNum == 1) {
                                gGame.stts = GS_End;
                            }
                            Mix_PlayChannel(1, se[SE_Click], 0); //クリックを1回再生
                            break;
                        case SDLK_q:
                            gGame.stts = GS_Title;
                            break;
                        case SDLK_ESCAPE:
                            gGame.stts = GS_Title;
                            break; 
                    }    
            }
        }
    }

    return SDL_TRUE;
}

/* JoyCon用のゲーム中の入力（イベント）読取
 *
 * 返値
 *   終了させる: 0
 *   継続する  : 1
 */
SDL_bool InputEventJoycon(void)
{
    int index = -1;
    // JoyConの情報を更新
    joycon_get_state(&jc_r);
    if (gGame.stts != GS_End) {
        if (jc_r.stick.x > DZONE) {
            gGame.input.down = SDL_TRUE;
        } else if (jc_r.stick.x < -DZONE) {
            gGame.input.up = SDL_TRUE;
        } else {
            gGame.input.up   = SDL_FALSE;
            gGame.input.down = SDL_FALSE;
        }
        if (jc_r.stick.y < -DZONE) {
            gGame.input.right = SDL_TRUE;
        } else if (jc_r.stick.y > DZONE) {
            gGame.input.left = SDL_TRUE;
        } else {
            gGame.input.left  = SDL_FALSE;
            gGame.input.right = SDL_FALSE;
        }
        if (jc_r.button.btn.RStick) {
            if (repeatFlag) {
                index = IsTriggerByObject(&(gChara[gGame.player]), &overlap, 0.0);
                if (index != -1) {
                    if (gObj[index].type == OnPortal || gObj[index].type == OffPortal) {
                        if (CulcAngle(DiffVec(gChara[gGame.player].pos, gObj[index].pos), gChara[gGame.player].dir) <= M_PI / 3) {
                            Mix_PlayChannel(2, se[SE_SwitchPortal], 0); //スイッチ音を1回再生
                            PortalSwitch(index, -1, 1);
                        }
                    }
                    if (gObj[index].type == Crystal) {
                        gGame.energy += gGame.isOn;
                        if (gGame.isOn > 0) {
                            Mix_PlayChannel(2, se[SE_Energy], 0); //エネルギ貯蓄音を1回再生
                        }
                        gGame.isOn = 0;
                        for (int i = 0; i < ObjNum; i++)
                            if (gObj[i].type == OnPortal || gObj[i].type == OffPortal) {
                                PortalSwitch(i, 0, 0);
                            }
                    }
                }
                repeatFlag = 0;
            }
        }
        if (jc_r.button.btn.Y) {
            if (repeatFlag) {
                UseItem();
                repeatFlag = 0;
            }
        }
        if (jc_r.button.btn.X) {
            if (repeatFlag) {
                ChangeItem(+1);
                repeatFlag = 0;
            }
        }

        if (jc_r.button.btn.B) {
            if (repeatFlag) {
                ChangeItem(-1);
                repeatFlag = 0;
            }
        }
    }
    if (jc_r.button.btn.Home && repeatFlag) {
        gGame.stts = GS_Title;
        repeatFlag = 0;
    }

    return SDL_TRUE;
}

/* JoyCon用のメインゲームシーン以外のシーンでの入力 */
SDL_bool SelectInputJoyCon(void)
{
    // JoyConの情報を更新
    joycon_get_state(&jc_r);

    //タイトルシーン
    if (gGame.stts == GS_Title) {
        if (manualFlag == 0) {
            if (jc_r.stick.x > DZONE && repeatFlag) {
                SelectButton(&titleButtonNum, +1, 3); //下に移動
                repeatFlag = 0;
            } else if (jc_r.stick.x < -DZONE && repeatFlag) {
                SelectButton(&titleButtonNum, -1, 3); //上に移動
                repeatFlag = 0;
            }
            if (jc_r.button.btn.X || jc_r.button.btn.A) {
                if (repeatFlag) {
                    if (titleButtonNum == 0) {
                        gGame.stts = GS_Select;
                    }
                    if (titleButtonNum == 1) {
                        manualFlag = 1;
                    }
                    if (titleButtonNum == 2) {

                        SDL_Delay(1000);
                        gGame.stts = GS_End;
                    }
                    Mix_PlayChannel(1, se[SE_Click], 0); //クリックを1回再生
                    repeatFlag = 0;
                }
            }
        } else if (manualFlag == 1) {
            if (jc_r.stick.x > DZONE && repeatFlag) {
                if (450 + manualPos.y < 1632)
                    manualPos.y += 100; //下に移動
                repeatFlag = 0;
            } else if (jc_r.stick.x < -DZONE && repeatFlag) {
                if (manualPos.y > 0)
                    manualPos.y -= 100; //上に移動
                repeatFlag = 0;
            }
            if (jc_r.button.btn.X || jc_r.button.btn.A) {
                if (repeatFlag) {
                    manualFlag  = 0;
                    manualPos.x = manualPos.y = 0;
                    repeatFlag                = 0;
                }
            }
        }
        if (jc_r.button.btn.Home && repeatFlag) {
            gGame.stts = GS_End;
            repeatFlag = 0;
        }

    }

    //難易度選択シーン
    else if (gGame.stts == GS_Select) {
        if (jc_r.stick.x > DZONE && repeatFlag) {
            SelectButton(&modeButtonNum, +1, 3); //上に移動
            repeatFlag = 0;
        } else if (jc_r.stick.x < -DZONE && repeatFlag) {
            SelectButton(&modeButtonNum, -1, 3); //下に移動
            repeatFlag = 0;
        }
        if (jc_r.button.btn.X || jc_r.button.btn.A) {
            if (repeatFlag) {
                if (modeButtonNum == 0) {
                    gGame.mode = G_Easy;
                } else if (modeButtonNum == 1) {
                    gGame.mode = G_Normal;
                } else if (modeButtonNum == 2) {
                    gGame.mode = G_Hard;
                }
                Mix_PlayChannel(1, se[SE_Click], 0); //クリックを1回再生
                SDL_Delay(1000);
                gGame.stts = GS_Playing;
                InitInfo(&gGame);
                for (int i = 0; i < ItemImgNum; i++)
                    itemNum[i] = itemMaxNum[i];
                atimer   = SDL_AddTimer(100, AniTimer, &framecnt);
                btimer   = SDL_AddTimer(gGame.spawnTime[gGame.mode], SpawnTimer, NULL);
                ctimer   = SDL_AddTimer(1000, SecondTimer, NULL);
                framecnt = 0;
                RenderWindow();
                repeatFlag = 0;
                Mix_RewindMusic(); // 先頭まで巻き戻す
                int randNum = rand() % 2;
                Mix_PlayMusic(bgm[randNum], -1); // Music型サウンドを無限に再生
                Mix_VolumeMusic(MIX_MAX_VOLUME);
            }
        }

        if (jc_r.button.btn.Home && repeatFlag) {
            gGame.stts = GS_Title;
            repeatFlag = 0;
        }
    }

    //リザルトシーン
    else if (gGame.stts == GS_Result) {

        if (jc_r.stick.y > DZONE && repeatFlag) {
            SelectButton(&resultButtonNum, -1, 2); //左に移動
            repeatFlag = 0;
        } else if (jc_r.stick.y < -DZONE && repeatFlag) {
            SelectButton(&resultButtonNum, +1, 2); //右に移動
            repeatFlag = 0;
        }
        if (jc_r.button.btn.X || jc_r.button.btn.A) {
            if (repeatFlag) {
                if (resultButtonNum == 0) {
                    gGame.stts = GS_Title;
                }
                if (resultButtonNum == 1) {
                    gGame.stts = GS_End;
                }
                repeatFlag = 0;
            }
            Mix_PlayChannel(1, se[SE_Click], 0); //クリックを1回再生
        }

        if (jc_r.button.btn.Home && repeatFlag) {
            gGame.stts = GS_Title;
            repeatFlag = 0;
        }
    }

    return SDL_TRUE;
}
