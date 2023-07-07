#include "header.h"
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>

char gMapFile[]    = "images/map.png";
char *gObjFile[]   = { "images/crystal.png", "images/offportal.png", "images/onportal.png", "images/spawner.png" };
char *gCharaFile[] = { "images/player.png", "images/bom.png" };
char *gItemFile[]  = { "images/item.png", "images/itemicon.png" };
char *gUiFile[]    = { "images/ui.png", "images/gauge_green.png", "images/gauge_blue.png", "images/gauge_flame.png", "images/title.png", "images/clear.png", "images/gameover.png", "images/difficulty.png", "images/manual.png" };

SDL_Texture *iconImg[ItemImgNum];          /*アイコンの画像*/
SDL_Texture *numTexture[11];               //数字のテキストテクスチャを格納
SDL_Rect size_numTexture = { 0, 0, 0, 0 }; //数字のテキストテクスチャの大きさを格納

/* 色 */
static const SDL_Color gWhite = { 255, 255, 255, 255 };
static const SDL_Color gBlue  = { 0, 0, 255, 255 };
static const SDL_Color gBlack = { 0, 0, 0, 255 }; // フォントの色を黒に

/* メッセージ */
static char *gMsgStr[MSG_NUM] = { " ", "Clear", "GameOver" };

int GetPatFromDir(const CharaInfo *chara);

/* メインウインドウの表示，設定
 *
 * 返値
 *   正常終了: 0
 *   エラー  : 負数
 */
int InitWindow(void)
{
    /* SDL_image初期化 */
    if (IMG_INIT_PNG != IMG_Init(IMG_INIT_PNG)) {
        return PrintError("failed to initialize SDL_image");
    }

    /** メインのウインドウ(表示画面)とレンダラーの作成 **/
    gGame.window = SDL_CreateWindow("Energy Saver", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WD_Width + UIWD_Width, WD_Height, 0);
    if (gGame.window == NULL)
        return PrintError(SDL_GetError());

    gGame.render = SDL_CreateRenderer(gGame.window, -1, 0);
    if (gGame.render == NULL)
        return PrintError(SDL_GetError());

    /*マップの背景画像の読み込み*/
    SDL_Surface *s = IMG_Load(gMapFile);
    if (s == NULL) {
        return PrintError("failed to open map image.");
    }
    gMap.mapImg = SDL_CreateTextureFromSurface(gGame.render, s);
    SDL_FreeSurface(s);
    if (gMap.mapImg == NULL) {
        return PrintError(SDL_GetError());
    }

    for (int i = 0; i < uiImgNum; i++) {
        /* UI画像の読み込み */
        SDL_Surface *s = IMG_Load(gUiFile[i]);
        if (s == NULL) {
            return PrintError("failed to open ui image.");
        }
        uiImg[i] = SDL_CreateTextureFromSurface(gGame.render, s);
        SDL_FreeSurface(s);
        if (uiImg[i] == NULL) {
            return PrintError(SDL_GetError());
        }
    }

    /*オブジェクト画像の読み込み*/
    for (int i = 0; i < ObjImgNum; i++) {
        SDL_Surface *s = IMG_Load(gObjFile[i]);
        if (s == NULL) {
            return PrintError("failed to open object image.");
        }
        gObjImg[i].img = SDL_CreateTextureFromSurface(gGame.render, s); //画像情報とシステムの関連付け
        SDL_FreeSurface(s);
        if (gObjImg[i].img == NULL) {
            return PrintError(SDL_GetError());
        }
    }
    /** オブジェクト情報設定 **/
    for (int i = 0; i < ObjNum; i++) {
        gObj[i].image = &(gObjImg[gObj[i].type]);
        //オブジェクト座標の格納
        gObj[i].pos.x = gObj[i].pos.x * WD_Width - gObj[i].image->mask.w / 2;
        gObj[i].pos.y = gObj[i].pos.y * WD_Height - gObj[i].image->mask.h / 2;
    }

    /** キャラ画像の読み込み **/
    /* 画像は，
     *  横方向にアニメーションパターン
     *  縦方向に向きパターン（時計回り，最初が↑）
     *  があると想定
     */
    for (int i = 0; i < CHARAIMG_NUM; i++) {
        SDL_Surface *s = IMG_Load(gCharaFile[i]);
        if (NULL == s) {
            return PrintError("failed to open character image.");
        }
        gCharaImg[i].anipatnum = s->w / gCharaImg[i].w;
        gCharaImg[i].dirpatnum = s->h / gCharaImg[i].h;
        if (i == 1)
            gCharaImg[i].dirpatnum = (s->h - 90) / gCharaImg[i].h;
        gCharaImg[i].img = SDL_CreateTextureFromSurface(gGame.render, s);
        /* サーフェイス解放(テクスチャに転送後はゲーム中に使わないので) */
        SDL_FreeSurface(s);
        if (gCharaImg[i].img == NULL) {
            return PrintError(SDL_GetError());
        }
    }

    //キャラ元情報の更新
    for (int i = 0; i < gBaseCharaNum; i++)
        InitBaseCharaInfo(&(gBaseChara[i]));

    //プレイヤーの情報の作成
    CreateCharaInfo(&(gChara[0]), Player);
    gCharaNum++;

    /*アイテム画像の読み込み*/
    s = IMG_Load(gItemFile[0]);
    if (NULL == s) {
        return PrintError("failed to open item image.");
    }
    for (int i = 0; i < ItemImgNum; i++) {
        SDL_Surface *blits = SDL_CreateRGBSurface(0, ItemImgSize, ItemImgSize, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
        SDL_Rect src_rect  = { i * ItemImgSize, 0, ItemImgSize, ItemImgSize };
        SDL_Rect dst_rect  = { 0, 0 };
        SDL_BlitSurface(s, &src_rect, blits, &dst_rect);
        if (NULL == blits) {
            return PrintError("failed to open item image2.");
        }
        gItemImg[i].img = SDL_CreateTextureFromSurface(gGame.render, blits);
        if (gItemImg[i].img == NULL) {
            return PrintError(SDL_GetError());
        }
    }
    /* サーフェイス解放(テクスチャに転送後はゲーム中に使わないので) */
    SDL_FreeSurface(s);
    for (int i = 0; i < ItemImgNum; i++)
        if (gItemImg[i].img == NULL) {
            return PrintError(SDL_GetError());
        }
    //アイテム画像情報をgItemに入れる
    for (int i = 0; i < gItemNum; i++) {
        gItem[i].image = &(gItemImg[gItem[i].type]);
        gItem[i].isSet = 0;
        gItem[i].isUse = 1;
    }

    /* アイテムアイコン画像の読み込み */
    s = IMG_Load(gItemFile[1]);
    if (NULL == s) {
        return PrintError("failed to open item icon image.");
    }
    for (int i = 0; i < ItemImgNum; i++) {
        SDL_Surface *blits = SDL_CreateRGBSurface(0, ItemImgSize, ItemImgSize, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
        SDL_Rect src_rect  = { i * ItemImgSize, 0, ItemImgSize, ItemImgSize };
        SDL_Rect dst_rect  = { 0, 0 };
        SDL_BlitSurface(s, &src_rect, blits, &dst_rect);
        if (NULL == blits) {
            return PrintError("failed to open item icon image2.");
        }
        iconImg[i] = SDL_CreateTextureFromSurface(gGame.render, blits);
        if (iconImg[i] == NULL) {
            return PrintError(SDL_GetError());
        }
    }
    /* サーフェイス解放(テクスチャに転送後はゲーム中に使わないので) */
    SDL_FreeSurface(s);

    /*文字テクスチャの生成*/
    if (MakeText() < 0) {
        return PrintError("failed to make text");
    }

    /* image利用終了(テクスチャに転送後はゲーム中に使わないので) */
    IMG_Quit();
    return 0;
}

/* ウインドウの終了処理 */
void DestroyWindow(void)
{
    /* テクスチャなど */
    for (int i = 0; i < CHARAIMG_NUM; i++)
        SDL_DestroyTexture(gCharaImg[i].img);
    for (int i = 0; i < ObjImgNum; i++)
        SDL_DestroyTexture(gObjImg[i].img);
    SDL_DestroyRenderer(gGame.render);
    SDL_DestroyWindow(gGame.window);
}

/* 必要な文字のテクスチャを生成 */
int MakeText(void)
{
    int ret            = 0;
    const char num[11] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':' };

    if (TTF_Init() < 0) {
        ret = PrintError("failed to init ttf");
    }

    TTF_Font *font = TTF_OpenFont("fonts-japanese-mincho.ttf", 48);
    if (font == NULL) {
        ret = PrintError("not find font data.");
    }

    /* 数字の作成 */
    for (int i = 0; i < 11; i++) {
        SDL_Surface *s = TTF_RenderUTF8_Blended(font, &(num[i]), gBlack);
        if (NULL == s) {
            ret = PrintError("failed to open figure image.");
        }
        size_numTexture.w = s->w * 2;
        size_numTexture.h = s->h;
        numTexture[i]     = SDL_CreateTextureFromSurface(gGame.render, s);
        SDL_FreeSurface(s);
        if (numTexture[i] == NULL) {
            ret = PrintError(SDL_GetError());
        }
    }

    /* メッセージ作成 */

    SDL_Color cols[MSG_NUM] = { gBlue, gBlue, gWhite };
    for (int i = 0; i < MSG_NUM && font; i++) {
        SDL_Surface *sf;
        /* フォントと文字列，色からサーフェイス作成 */
        sf = TTF_RenderUTF8_Blended(font, gMsgStr[i], cols[i]);
        if (NULL == sf) {
            ret = PrintError(TTF_GetError());
        } else {
            /* テクスチャへ */
            gGame.msgTexts[i] = SDL_CreateTextureFromSurface(gGame.render, sf);
            if (NULL == gGame.msgTexts[i]) {
                ret = PrintError(SDL_GetError());
            }
            /* サーフェイス解放(テクスチャに転送後はゲーム中に使わないので) */
            SDL_FreeSurface(sf);
        }
    }

    TTF_Quit();

    return ret;
}

/* 向きパターン取得
 *  キャラの方向から画像の向きパターンを取得する
 *
 * 引数
 *   chara: キャラ情報
 *
 * 返値
 *  パターン番号
 */
int GetPatFromDir(const CharaInfo *chara)
{
    float arc = atan2f(chara->dir.y, chara->dir.x);
    /* 画像の向きの境界角度算出（向きが4種類の時は45度，8種類の時は22.5度） */
    float d = 2.0 * M_PI / (chara->image->dirpatnum * 2);
    int ret = 6;
    /* 向きパターンの算出（4種類の時は90度ずつ，小さい順にキャラ方向と比較） */
    for (float f = -M_PI + d; f <= M_PI; f += 2.0 * d) {
        if (arc < f)
            break;
        ret++;
    }
    return ret % chara->image->dirpatnum;
}

void RenderWindow(void)
{
    /* マップ */
    SDL_Rect src = { 0, 0, 880, 880 };
    SDL_Rect dst = { 0, 0, WD_Width, WD_Height };
    if (0 > SDL_RenderCopy(gGame.render, gMap.mapImg, &src, &dst)) {
        PrintError(SDL_GetError());
    }

    /* UI背景 */
    SDL_Rect src_2 = { 0, 0, 400, 960 };
    SDL_Rect dst_2 = { WD_Width, 0, UIWD_Width, WD_Height };
    if (0 > SDL_RenderCopy(gGame.render, uiImg[UI_Back], &src_2, &dst_2)) {
        PrintError(SDL_GetError());
    }

    /* アイテムアイコン */
    for (int i = 0; i < ItemImgNum; i++) {
        SDL_Rect src = { 0, 0, ItemImgSize, ItemImgSize };
        SDL_Rect dst = { WD_Width + 50 * (2.2 * i + 1) - ((i < 3) ? 0 : 1) * 275, 410 + ((i < 3) ? 0 : 1) * 170, ItemImgSize, ItemImgSize };
        if (0 > SDL_RenderCopy(gGame.render, iconImg[i], &src, &dst)) {
            PrintError(SDL_GetError());
        }
        SDL_Rect src2 = { 0, 0, size_numTexture.w, size_numTexture.h };
        SDL_Rect dst2 = { WD_Width + 50 * (2.2 * i + 1.5) - ((i < 3) ? 0 : 1) * 275, 490 + ((i < 3) ? 0 : 1) * 170, size_numTexture.w, size_numTexture.h };
        if (0 > SDL_RenderCopy(gGame.render, numTexture[itemNum[i]], &src2, &dst2)) {
            PrintError(SDL_GetError());
        }
    }

    /* ゲージ */
    //クリスタルエネルギー
    if (gGame.energy > ENE_MAX)
        gGame.energy = ENE_MAX;
    SDL_Rect src_3 = { 0, 0, (GAUGE_WIDTH / ENE_MAX) * gGame.energy, 58 };
    SDL_Rect dst_3 = { WD_Width + 188, 143, (GAUGE_WIDTH / ENE_MAX) * gGame.energy / 1.185, 46.7 };
    if (0 > SDL_RenderCopy(gGame.render, uiImg[UI_BlueGauge], &src_3, &dst_3)) {
        PrintError(SDL_GetError());
    }
    //クリスタルHP
    SDL_Rect src_4 = { 0, 0, (GAUGE_WIDTH / CMAX_HP) * gGame.hp, 58 };
    SDL_Rect dst_4 = { WD_Width + 188, 183, (GAUGE_WIDTH / CMAX_HP) * gGame.hp / 1.185, 46.7 };
    if (0 > SDL_RenderCopy(gGame.render, uiImg[UI_BlueGauge], &src_4, &dst_4)) {
        PrintError(SDL_GetError());
    }
    //プレイヤーHP
    SDL_Rect src_5 = { 0, 0, (GAUGE_WIDTH / PMAX_HP) * gChara[gGame.player].hp, 58 };
    SDL_Rect dst_5 = { WD_Width + 185, 279, (GAUGE_WIDTH / PMAX_HP) * gChara[gGame.player].hp / 1.145, 48 };
    if (0 > SDL_RenderCopy(gGame.render, uiImg[UI_GreenGauge], &src_5, &dst_5)) {
        PrintError(SDL_GetError());
    }

    /* 時間の表示 */
    int min = (GameTime - gGame.secondTime) / 60;
    int sec = (GameTime - gGame.secondTime) % 60;
    if (sec <= 0)
        sec = 0;
    int figure[5] = { min / 10, min % 10, 10, sec / 10, sec % 10 };
    for (int i = 0; i < 5; i++) {
        SDL_Rect src = { 0, 0, size_numTexture.w, size_numTexture.h };
        SDL_Rect dst = { WD_Width + 210 + i * size_numTexture.w, 40, size_numTexture.w, size_numTexture.h };
        if (0 > SDL_RenderCopy(gGame.render, numTexture[figure[i]], &src, &dst)) {
            PrintError(SDL_GetError());
        }
    }

    /* アイテムアイコンの選択状態 */
    boxColor(gGame.render, WD_Width + 50 * (2.2 * gGame.selectItemNum + 1) - ((gGame.selectItemNum < 3) ? 0 : 1) * 275, 410 + ((gGame.selectItemNum < 3) ? 0 : 1) * 170, WD_Width + 50 * (2.2 * gGame.selectItemNum + 1) - ((gGame.selectItemNum < 3) ? 0 : 1) * 275 + 76, 410 + ((gGame.selectItemNum < 3) ? 0 : 1) * 170 + 76, 0x77000000);

    /* アイテム床を出す */
    for (int i = 0; i < gItemNum; i++) {
        if (gItem[i].area == gChara[gGame.player].area) {
            if (gItem[i].isSet) {
                SDL_Rect src = { 0, 0, ItemImgSize, ItemImgSize };
                SDL_Rect dst = { gItem[i].pos.x, gItem[i].pos.y, ItemImgSize, ItemImgSize };
                if (gItem[i].type == IT_Star) {
                    src.w = src.h = dst.w = dst.h = ItemImgSize * 4;
                }

                if (0 > SDL_RenderCopy(gGame.render, gItem[i].image->img, &src, &dst)) {
                    PrintError(SDL_GetError());
                }
            }
        }
    }

    /*オブジェクト*/
    for (int i = 0; i < ObjNum; i++) {
        if (gObj[i].area == gChara[gGame.player].area) {
            SDL_Rect src = { 0, 0, gObj[i].image->w, gObj[i].image->h };
            SDL_Rect dst = { gObj[i].pos.x, gObj[i].pos.y, gObj[i].image->w / 2, gObj[i].image->h / 2 };
            if (0 > SDL_RenderCopy(gGame.render, gObj[i].image->img, &src, &dst)) {
                PrintError(SDL_GetError());
            }
        }
    }

    /* キャラ */
    for (int i = 0; i < gCharaNum; i++) {
        /* 転送元設定 */
        SDL_Rect src = { gChara[i].anipat * gChara[i].image->w, gChara[i].image->h, gChara[i].image->w, gChara[i].image->h };
        src.y *= GetPatFromDir(&(gChara[i]));
        /* 転送先設定 */
        SDL_Rect dst = { gChara[i].pos.x, gChara[i].pos.y, gChara[i].image->w, gChara[i].image->h };
        if (gChara[i].flag == 1) {

            src.x = 0;
            src.y = gChara[i].image->h * 8;
            if (gChara[i].rest < 10)
                src.x = 3 * gChara[i].image->w;
            else if (gChara[i].rest < 20)
                src.x = 2 * gChara[i].image->w;
            else if (gChara[i].rest < 30)
                src.x = gChara[i].image->w;
        }
        /* 転送 */
        if (0 > SDL_RenderCopy(gGame.render, gChara[i].image->img, &src, &dst)) {
            PrintError(SDL_GetError());
        }
    }

    /* メッセージ（ウインドウの中心に） */
    if (gGame.msg != MS_NONE) {
        SDL_Rect src = { 0 };
        if (0 > SDL_QueryTexture(gGame.msgTexts[gGame.msg], NULL, NULL, &src.w, &src.h)) {
            PrintError(SDL_GetError());
        }
        SDL_Rect dst;
        dst.x = (WD_Width - src.w) / 2;
        dst.w = src.w;
        dst.y = (WD_Height - src.h) / 2;
        dst.h = src.h;

        if (0 > SDL_RenderCopy(gGame.render, gGame.msgTexts[gGame.msg], &src, &dst)) {
            PrintError(SDL_GetError());
        }
    }

    SDL_RenderPresent(gGame.render);
}

/*
    タイトルシーンの描画
*/
void RenderTitleWindow(void)
{
    /* 背景 */
    SDL_Rect src = { 0, 0, 1280, 880 };
    SDL_Rect dst = { 0, 0, WD_Width + UIWD_Width, WD_Height };
    if (0 > SDL_RenderCopy(gGame.render, uiImg[UI_Title], &src, &dst)) {
        PrintError(SDL_GetError());
    }
    /* ボタンの選択状態 */
    if (manualFlag == 0)
        boxColor(gGame.render, 388, 355 + titleButtonNum * 170, 970, 478 + titleButtonNum * 170, 0x99000000);
    else if (manualFlag == 1) {
        boxColor(gGame.render, 0, 0, WD_Width + UIWD_Width, WD_Height, 0x99000000);
        SDL_Rect src = { 0, manualPos.y, 528, 450 };
        SDL_Rect dst = { 320, 70, 700, 810 };
        if (0 > SDL_RenderCopy(gGame.render, uiImg[UI_Manual], &src, &dst)) {
            PrintError(SDL_GetError());
        }
    }

    SDL_RenderPresent(gGame.render);
}

/*
    難易度選択シーンの描画
*/
void RenderSelectWindow(void)
{
    /* 背景 */
    SDL_Rect src = { 0, 0, 1280, 880 };
    SDL_Rect dst = { 0, 0, WD_Width + UIWD_Width, WD_Height };
    if (0 > SDL_RenderCopy(gGame.render, uiImg[UI_Dufficulty], &src, &dst)) {
        PrintError(SDL_GetError());
    }

    /* ボタンの選択状態 */
    boxColor(gGame.render, 225, 107 + modeButtonNum * 283, 1130, 300 + modeButtonNum * 283, 0x99000000);

    SDL_RenderPresent(gGame.render);
}

/*
    リザルトシーンの描画
*/
void RenderResultWindow(void)
{
    if (gGame.msg == MS_CLEAR) {
        /* 背景 */
        SDL_Rect src = { 0, 0, 1280, 880 };
        SDL_Rect dst = { 0, 0, WD_Width + UIWD_Width, WD_Height };
        if (0 > SDL_RenderCopy(gGame.render, uiImg[UI_Clear], &src, &dst)) {
            PrintError(SDL_GetError());
        }

        /* クリア時間の表示 */
        int min = (GameTime - gGame.clearTime) / 60;
        int sec = (GameTime - gGame.clearTime) % 60;
        if (sec <= 0)
            sec = 0;
        if (min <= 0)
            min = 0;
        int figure[5] = { min / 10, min % 10, 10, sec / 10, sec % 10 };
        for (int i = 0; i < 5; i++) {
            SDL_Rect src = { 0, 0, size_numTexture.w, size_numTexture.h };
            SDL_Rect dst = { 700 + i * size_numTexture.w * 2, 240, size_numTexture.w * 2, size_numTexture.h * 2 };
            if (0 > SDL_RenderCopy(gGame.render, numTexture[figure[i]], &src, &dst)) {
                PrintError(SDL_GetError());
            }
        }

        /* ダメージ数 */
        int damage_figure[3] = { gGame.damageNum / 100, (gGame.damageNum % 100) / 10, (gGame.damageNum % 100) % 10 };
        for (int i = 0; i < 3; i++) {
            SDL_Rect src = { 0, 0, size_numTexture.w, size_numTexture.h };
            SDL_Rect dst = { 700 + i * size_numTexture.w * 2, 395, size_numTexture.w * 2, size_numTexture.h * 2 };
            if (0 > SDL_RenderCopy(gGame.render, numTexture[damage_figure[i]], &src, &dst)) {
                PrintError(SDL_GetError());
            }
        }

        /* アイテムの使った数 */
        int item_figure[3] = { gGame.itemUseNum / 100, (gGame.itemUseNum % 100) / 10, (gGame.itemUseNum % 100) % 10 };
        for (int i = 0; i < 3; i++) {
            SDL_Rect src = { 0, 0, size_numTexture.w, size_numTexture.h };
            SDL_Rect dst = { 700 + i * size_numTexture.w * 2, 550, size_numTexture.w * 2, size_numTexture.h * 2 };
            if (0 > SDL_RenderCopy(gGame.render, numTexture[item_figure[i]], &src, &dst)) {
                PrintError(SDL_GetError());
            }
        }
    } else if (gGame.msg == MS_GAMEOVER) {
        /* 背景 */
        SDL_Rect src = { 0, 0, 1280, 880 };
        SDL_Rect dst = { 0, 0, WD_Width + UIWD_Width, WD_Height };
        if (0 > SDL_RenderCopy(gGame.render, uiImg[UI_GameOver], &src, &dst)) {
            PrintError(SDL_GetError());
        }
    }

    /* ボタンの選択状態 */
    boxColor(gGame.render, 217 + (resultButtonNum * 518), 752, 629 + (resultButtonNum * 518), 897, 0x99000000);

    SDL_RenderPresent(gGame.render);
}


//Debug用
void TestRender(void)
{
    for (int i = 0; i < gCharaNum; i++)
        boxColor(gGame.render, gChara[i].image->mask.x + gChara[i].pos.x, gChara[i].image->mask.y + gChara[i].pos.y, gChara[i].image->mask.x + gChara[i].pos.x + gChara[i].image->mask.w, gChara[i].image->mask.y + gChara[i].pos.y + gChara[i].image->mask.h, 0xffffffff);
    for (int i = 0; i < ObjNum; i++)
        boxColor(gGame.render, gObj[i].image->trigger.x + gObj[i].pos.x, gObj[i].image->trigger.y + gObj[i].pos.y, gObj[i].image->trigger.x + gObj[i].pos.x + gObj[i].image->trigger.w, gObj[i].image->trigger.y + gObj[i].pos.y + gObj[i].image->trigger.h, 0xffff00ff);

    for (int i = 0; i < ObjNum; i++)
        boxColor(gGame.render, gObj[i].image->mask.x + gObj[i].pos.x, gObj[i].image->mask.y + gObj[i].pos.y, gObj[i].image->mask.x + gObj[i].pos.x + gObj[i].image->mask.w, gObj[i].image->mask.y + gObj[i].pos.y + gObj[i].image->mask.h, 0xffffffff);

    SDL_Rect itemMask = gItem[0].image->trigger;
    itemMask.x += gItem[0].pos.x;
    itemMask.y += gItem[0].pos.y;
    boxColor(gGame.render, itemMask.x, itemMask.y, itemMask.x + itemMask.w, itemMask.y + itemMask.h, 0xffff00ff);

    SDL_Rect isResp = { gObj[5].pos.x - 10, gObj[5].pos.y - WD_Height * 0.12, gObj[5].image->mask.w + 20, WD_Height * 0.13 }; //スポーン範囲
    boxColor(gGame.render, isResp.x, isResp.y, isResp.x + isResp.w, isResp.y + isResp.h, 0xff00ffff);
    SDL_Rect isResp2 = { gObj[6].pos.x - 10, gObj[6].pos.y - WD_Height * 0.12, gObj[6].image->mask.w + 20, WD_Height * 0.13 }; //スポーン範囲
    boxColor(gGame.render, isResp2.x, isResp2.y, isResp2.x + isResp2.w, isResp2.y + isResp2.h, 0xff00ffff);

    SDL_RenderPresent(gGame.render);
}