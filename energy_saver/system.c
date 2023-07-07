#include "header.h"
#include <SDL2/SDL2_gfxPrimitives.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define MAX_LINEBUF 256

SDL_Texture* uiImg[uiImgNum]; // UI画像のテクスチャ
MapInfo gMap;
GameInfo gGame;
CharaImgInfo gCharaImg[CHARAIMG_NUM];
CharaInfo* gBaseChara;
CharaInfo* gChara;
ObjImgInfo gObjImg[ObjImgNum];
ObjectInfo* gObj;
ItemImgInfo gItemImg[ItemImgNum];
ItemInfo* gItem;

Mix_Chunk* se[SeNum];
Mix_Music* bgm[2]; // Music 型でデータを格納する変数を宣言
char* gSeFile[] = { "sounds/bomb.wav", "sounds/click.wav", "sounds/energy.wav", "sounds/switchportal.wav" };

int titleButtonNum  = 0; // Titleシーンのボタン番号
int modeButtonNum   = 0; //難易度選択のボタン番号
int resultButtonNum = 0; // Resultシーンのボタン番号
int manualFlag      = 0; // manualを表示しているかのフラグ
Vector2 manualPos   = { 0, 0 };

int gItemNum  = 0;
int gCharaNum = 0;
int gBaseCharaNum;
int ObjNum; //マップ上のオブジェクトの数(クリスタル*1,ポータル*3)

int itemMaxNum[ItemImgNum] = { 3, 3, 3, 1, 1 };
int itemNum[ItemImgNum]    = { 3, 3, 3, 1, 1 };

const int itemEffectTime[ItemImgNum] = { 5, 7, 5, 8, 6 };  //アイテムのセットしてからの効果時間
const int itemReloadTime[ItemImgNum] = { 5, 7, 5, 12, 8 }; //アイテムをセットしてから次使えるまでの時間
SDL_Rect overlap;

void UpdateCharaInfoByPattern(CharaInfo* ch);
void SetDirToPoint(CharaInfo* src, const Vector2 dst);
float GetCharaCenterDistance(CharaInfo* ci, CharaInfo* cj);
float GetCharaObjCenterDistance(CharaInfo* ch, ObjectInfo* obj);
/* ゲームシステム初期化
 *
 * 引数
 *   chara_data_file: キャラクタデータファイルのパス
 *   map_data_file  : マップデータファイルのパス
 *
 * 返値
 *   正常終了: 0
 *   エラー  : 負数
 */
int InitSystem(const char* chara_data_file, const char* map_data_file, const char* item_data_file)
{
    int ret = 0;
    srand(time(NULL)); //乱数の種設定

    FILE* fp = fopen(map_data_file, "r");
    if (fp == NULL) {
        return PrintError("failed to open map data file.");
    }
    /* 1行読込 */
    int wallno = 0, objimgno = 0, objno = -1;
    char linebuf[MAX_LINEBUF];
    while (fgets(linebuf, MAX_LINEBUF, fp)) {
        /* 先頭が#の行はコメントとして飛ばす */
        if (linebuf[0] == '#')
            continue;
        /* マップ情報読込 */
        else if (wallno < 4) {
            if (4 != sscanf(linebuf, "%d%d%d%d", &(gMap.wall[wallno].x), &(gMap.wall[wallno].y), &(gMap.wall[wallno].w), &(gMap.wall[wallno].h))) {
                ret = PrintError("failed to read the area data.");
                goto CLOSEFILE;
            }
            wallno++;
        }
        /* オブジェクト画像情報読込 */
        else if (objimgno < ObjImgNum) {
            if (10 != sscanf(linebuf, "%d%d%d%d%d%d%d%d%d%d", &(gObjImg[objimgno].w), &(gObjImg[objimgno].h), &(gObjImg[objimgno].mask.x), &(gObjImg[objimgno].mask.y), &(gObjImg[objimgno].mask.w), &(gObjImg[objimgno].mask.h), &(gObjImg[objimgno].trigger.x), &(gObjImg[objimgno].trigger.y), &(gObjImg[objimgno].trigger.w), &(gObjImg[objimgno].trigger.h))) {
                ret = PrintError("failed to read the object image data.");
                goto CLOSEFILE;
            }
            objimgno++;
        }
        /* オブジェクト総数読込 */
        else if (objno < 0) {
            if (1 != sscanf(linebuf, "%d", &ObjNum)) {
                ret = PrintError("failed to read the number of object data.");
                goto CLOSEFILE;
            }
            /* オブジェクト情報確保 */
            gObj = (ObjectInfo*)malloc(sizeof(ObjectInfo) * ObjNum);
            if (!gObj) {
                ret = PrintError("failed to allocate the chara data.");
                goto CLOSEFILE;
            }
            objno = 0;
        }
        /* オブジェクト情報読込 */
        else if (objno < ObjNum) {
            if (6 != sscanf(linebuf, "%d%d%d%f%f%d", &(gObj[objno].area), &(gObj[objno].id), (int*)&(gObj[objno].type), &(gObj[objno].pos.x), &(gObj[objno].pos.y), &(gObj[objno].flag))) {
                ret = PrintError("failed to read the object data.");
                goto CLOSEFILE;
            }
            objno++;
        }
    }

    /** キャラクター情報読込 **/
    /* ファイルオープン */
    fp = fopen(chara_data_file, "r");
    if (fp == NULL) {
        return PrintError("failed to open chara data file.");
    }
    /* 1行読込 */
    int imgno = 0, charano = -1;
    while (fgets(linebuf, MAX_LINEBUF, fp)) {
        /* 先頭が#の行はコメントとして飛ばす */
        if (linebuf[0] == '#')
            continue;
        /* キャラ画像情報読込 */
        else if (imgno < CHARAIMG_NUM) {
            if (6 != sscanf(linebuf, "%d%d%d%d%d%d", &(gCharaImg[imgno].w), &(gCharaImg[imgno].h), &(gCharaImg[imgno].mask.x), &(gCharaImg[imgno].mask.y), &(gCharaImg[imgno].mask.w), &(gCharaImg[imgno].mask.h))) {
                ret = PrintError("failed to read the chara image data.");
                goto CLOSEFILE;
            }
            imgno++;
        }
        /* キャラ総数読込 */
        else if (charano < 0) {
            if (1 != sscanf(linebuf, "%d", &gBaseCharaNum)) {
                ret = PrintError("failed to read the number of chara data.");
                goto CLOSEFILE;
            }
            /* キャラ情報確保 */
            gBaseChara = (CharaInfo*)malloc(sizeof(CharaInfo) * gBaseCharaNum);
            if (!gBaseChara) {
                ret = PrintError("failed to allocate the chara data.");
                goto CLOSEFILE;
            }
            charano = 0;
        }
        /* キャラ情報読込 */
        else if (charano < gBaseCharaNum) {
            if (6 != sscanf(linebuf, "%d%u%f%d%f%d", &(gBaseChara[charano].kind), &(gBaseChara[charano].type), &(gBaseChara[charano].mass), &(gBaseChara[charano].power), &(gBaseChara[charano].basevel), (int*)&(gBaseChara[charano].pattern))) {
                ret = PrintError("failed to read the chara data.");
                break;
            }
            if (gBaseChara[charano].type == Player) {
                /* プレイヤーのセット */
                gGame.player = charano;
            }
            charano++;
        }
    }
    //プレイヤー用のデータ領域の作成,キャラ体数の初期化
    gChara    = (CharaInfo*)malloc(sizeof(CharaInfo) * CharaNumMax);
    gCharaNum = 0;

    /** アイテム情報読込 **/
    /* ファイルオープン */
    fp = fopen(item_data_file, "r");
    if (fp == NULL) {
        return PrintError("failed to open item data file.");
    }
    /* 1行読込 */
    int itemimgno = 0, itemno = -1;
    while (fgets(linebuf, MAX_LINEBUF, fp)) {
        /* 先頭が#の行はコメントとして飛ばす */
        if (linebuf[0] == '#')
            continue;
        /* アイテム画像情報の読み込み */
        else if (itemimgno < ItemImgNum) {
            if (4 != sscanf(linebuf, "%d%d%d%d", &(gItemImg[itemimgno].trigger.x), &(gItemImg[itemimgno].trigger.y), &(gItemImg[itemimgno].trigger.w), &(gItemImg[itemimgno].trigger.h))) {
                ret = PrintError("failed to read the item image data.");
                goto CLOSEFILE;
            }
            itemimgno++;
            /* アイテムの個数の読み込み */
        } else if (itemno < 0) {
            if (1 != sscanf(linebuf, "%d", &gItemNum)) {
                ret = PrintError("failed to read the item num.");
                break;
            }
            itemno = 0;
            gItem  = (ItemInfo*)malloc(sizeof(ItemInfo) * gItemNum);
            if (!gItem) {
                ret = PrintError("failed to allocate the item data.");
                goto CLOSEFILE;
            }
        }
        /* アイテム情報読込 */
        else if (itemno < gItemNum) {
            if (1 != sscanf(linebuf, "%d", (int*)&(gItem[itemno].type))) {
                ret = PrintError("failed to read the item data.");
                break;
            }
            itemno++;
        }
    }

CLOSEFILE:
    fclose(fp);
    return ret;
}

/* システム終了処理 */
void DestroySystem(void)
{
    free(gChara);
    free(gObj);
    free(gItem);
    return;
}

/* AUDIO関連の読み込み */
int LoadSound(void)
{
    int ret = 0;
    /* AUDIOデバイスの初期化 */
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        ret = PrintError("failed to init audio.");
    }
    Mix_Init(MIX_INIT_MP3); // MP3 ファイルを読み込むための初期化
    /* Audioを開く */
    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024) == -1) {
        printf("Failed: %s¥n", Mix_GetError());
        ret = PrintError("failed to openAudio.");
    }

    for (int i = 0; i < SeNum; i++) {
        se[i] = Mix_LoadWAV(gSeFile[i]);
        if (se[i] == NULL) {
            ret = PrintError("not found sound of se file.");
        }
    }
    Mix_Volume(1, MIX_MAX_VOLUME *5);  //click
    Mix_Volume(2, MIX_MAX_VOLUME); // bomb,charge

    bgm[0] = Mix_LoadMUS("sounds/bgm1.mp3"); // Music型で読み込み
    bgm[1] = Mix_LoadMUS("sounds/bgm2.mp3"); // Music型で読み込み

    if (bgm == NULL) {
        ret = PrintError("not found sound of bgm file.");
    }

    return ret;
}

/* キャラ基本状態の設定
 *  キャラクタの状態を設定する
 *  同時に状態に合わせた画像を設定する
 *
 * 引数
 *   chara: キャラ
 */
void InitBaseCharaInfo(CharaInfo* baseChara)
{
    /* キャラ画像情報の設定 */
    baseChara->image = &(gCharaImg[baseChara->type]);

    /* 初期値設定 */
    baseChara->hp       = 1;
    baseChara->anipat   = 0;
    baseChara->flag     = 0;
    baseChara->velocity = 0.0;
    baseChara->stts     = Disable;
    baseChara->dir.x    = 0;
    baseChara->dir.y    = 0;

    baseChara->pos.x = 0;
    baseChara->pos.y = 0;

    if (baseChara->type == Player) {
        baseChara->area = 0;
        baseChara->stts = Normal;
        baseChara->hp   = PMAX_HP;
    }
}

/* 情報の初期化 */
void InitInfo(GameInfo* game)
{
    //ゲーム情報の初期化
    game->msg           = MS_NONE;
    game->hp            = CMAX_HP;
    game->isOn          = 0;
    game->energy        = 0;
    game->selectItemNum = 0;
    game->secondTime    = 0;
    game->inhale        = 0;

    game->itemUseNum = 0;
    game->damageNum  = 0;
}
/*
キャラの情報を上書きコピーする
indexにはファイルから読み込んだ情報の何番目かを入れる
*/
int CreateCharaInfo(CharaInfo* chara, int index)
{
    int ret            = 0;
    SDL_Rect isResp[4] = { { gObj[5].pos.x - 10, gObj[5].pos.y - WD_Height * 0.12, gObj[5].image->mask.w + 20, WD_Height * 0.13 }, { gObj[6].pos.x - 10, gObj[6].pos.y - WD_Height * 0.12, gObj[6].image->mask.w + 20, WD_Height * 0.13 }, { gObj[7].pos.x - 10, gObj[7].pos.y - WD_Height * 0.12, gObj[7].image->mask.w + 20, WD_Height * 0.13 }, { gObj[8].pos.x - 10, gObj[8].pos.y - WD_Height * 0.12, gObj[8].image->mask.w + 20, WD_Height * 0.13 } }; //スポーン範囲

    Vector2 resPos[4] = { { gObj[5].pos.x, gObj[5].pos.y - WD_Height * 0.10 }, { gObj[6].pos.x, gObj[6].pos.y - WD_Height * 0.10 }, { gObj[7].pos.x, gObj[7].pos.y - WD_Height * 0.10 }, { gObj[8].pos.x, gObj[8].pos.y - WD_Height * 0.10 } }; //スポーン地点の座標

    if (index != 0) {
        int randNum = rand() % 4;
        int prenum; //探索しているキャラのindex番号

        for (prenum = 0; prenum < gCharaNum; prenum++) {
            SDL_Rect s = gChara[prenum].image->mask;
            s.x += gChara[prenum].pos.x;
            s.y += gChara[prenum].pos.y;

            // isResp内にキャラがいるならループを出る
            if (CheckOverLap(s, isResp[randNum], &overlap, 0.0))
                break;
        }
        //範囲内に重なりがないなら
        if (prenum == gCharaNum) {
            chara->pos.x = resPos[randNum].x;
            chara->pos.y = resPos[randNum].y;
            chara->area  = 0;
            ret          = 1;
        } else {
            return ret;
        }
    } else {
        chara->pos.x = 450;
        chara->pos.y = 600;
        chara->area  = 0;
    }

    CharaInfo baseChara = gBaseChara[index];
    chara->kind         = baseChara.kind;
    chara->id           = gCharaNum;
    chara->type         = index;
    chara->hp           = baseChara.hp;
    chara->power        = baseChara.power;
    chara->pattern      = baseChara.pattern;
    chara->flag         = baseChara.flag;
    chara->mass         = baseChara.mass;
    chara->basevel      = baseChara.basevel;
    chara->velocity     = baseChara.velocity;
    chara->dir          = baseChara.dir;
    chara->anipat       = baseChara.anipat;
    chara->image        = baseChara.image;

    chara->stts = Normal;
    ret         = 1;

    return ret;
}

/*
任意のキャラ情報を削除
index: 消したい要素番号
*/
void RemoveChara(int index)
{
    gCharaNum--;
    for (int i = index; i < gCharaNum; i++) {
        gChara[i]    = gChara[i + 1];
        gChara[i].id = i;
    }
}

/* ２つのベクトルの差のベクトルを返す */
Vector2 DiffVec(Vector2 a, Vector2 b)
{
    Vector2 diff;
    diff.x = b.x - a.x;
    diff.y = b.y - a.y;
    return diff;
}

/* 2つのベクトルのなす角を求める */
float CulcAngle(Vector2 a, Vector2 b)
{
    float a_angle = (float)atan2(a.y, a.x);
    if (a_angle < 0) {
        a_angle += 2 * M_PI;
    }
    float b_angle = (float)atan2(b.y, b.x);
    if (b_angle < 0) {
        b_angle += 2 * M_PI;
    }
    float absAngle = fabsf(b_angle - a_angle);
    if (absAngle >= M_PI)
        absAngle = 2 * M_PI - absAngle;
    return absAngle;
}

/* 入力状態から方向，速度の設定 */
void SetInput(void)
{
    Vector2 dir = { 0.0, 0.0 };
    if (gGame.input.up && !gGame.input.down) {
        dir.y = -1.0;
    }
    if (gGame.input.down && !gGame.input.up) {
        dir.y = 1.0;
    }
    if (gGame.input.left && !gGame.input.right) {
        dir.x = -1.0;
    }
    if (gGame.input.right && !gGame.input.left) {
        dir.x = 1.0;
    }
    /* 斜め方向の時は補正 */
    if (dir.x != 0.0 && dir.y != 0.0) {
        dir.x *= cosf(M_PI / 4.0);
        dir.x *= sinf(M_PI / 4.0);
    }
    if (!(dir.x == 0.0 && dir.y == 0.0)) {
        gChara[gGame.player].velocity = gChara[gGame.player].basevel;
        gChara[gGame.player].dir      = dir;
    } else {
        gChara[gGame.player].velocity = 0.0f;
    }
}

/* 指定した座標の方へ向く
 *  srcキャラから見た特定の座標へ方向を設定する
 *
 * 引数
 *   src: 方向を設定するキャラ
 *   dst: 特定の座標
 */
void SetDirToPoint(CharaInfo* src, const Vector2 dst)
{
    float rad  = atan2(dst.y - src->pos.y, dst.x - src->pos.x);
    src->dir.x = cosf(rad);
    src->dir.y = sinf(rad);
}

/* キャラクター間の距離を得る */
float GetCharaCenterDistance(CharaInfo* ci, CharaInfo* cj)
{
    Vector2 f = { (cj->pos.x + cj->image->mask.w / 2) - (ci->pos.x + ci->image->mask.w / 2), (cj->pos.y + cj->image->mask.h / 2) - (ci->pos.y + ci->image->mask.h / 2) };

    return sqrtf((f.x * f.x) + (f.y * f.y));
}
float GetCharaObjCenterDistance(CharaInfo* ch, ObjectInfo* obj)
{
    Vector2 f = { (obj->pos.x + obj->image->mask.w / 2) - (ch->pos.x + ch->image->mask.w / 2), (obj->pos.y + obj->image->mask.h / 2) - (ch->pos.y + ch->image->mask.h / 2) };

    return sqrtf((f.x * f.x) + (f.y * f.y));
}

/* キャラの情報更新
 *  キャラの行動パターン別に情報（状態，戦略など）を更新する
 *
 * 引数
 *   ch: 対象キャラ
 */
void UpdateCharaInfoByPattern(CharaInfo* ch)
{
    if (ch->stts == Normal)
        ch->velocity = ch->basevel;

    switch (ch->pattern) {
    case CP_Input:
        SetInput();
        break;
    case CP_Bom1:
        if (ch->stts == Normal || ch->stts == Decelerate)
            SetDirToPoint(ch, gObj[0].pos);
        //プレイヤーとの距離が近いと爆発
        if (80 > GetCharaCenterDistance(ch, &(gChara[gGame.player])) && ch->flag != 1) {
            ch->flag = 1;
            ch->rest = ExplodeTime / gGame.timeStep;
            Mix_PlayChannel(2, se[SE_Bomb], 0); //爆発音を1回再生
        }
        //クリスタルとの距離が近いと爆発
        if (110 > GetCharaObjCenterDistance(ch, &(gObj[0])) && ch->flag != 1) {
            ch->flag = 1;
            ch->rest = ExplodeTime / gGame.timeStep;
            Mix_PlayChannel(2, se[SE_Bomb], 0); //爆発音を1回再生
        }
        break;
    case CP_Bom2:
        if (ch->stts == Normal || ch->stts == Decelerate)
            SetDirToPoint(ch, gChara[0].pos);
        //プレイヤーとの距離が近いと爆発
        if (70 > GetCharaCenterDistance(ch, &(gChara[gGame.player])) && ch->flag != 1) {
            ch->flag = 1;
            ch->rest = ExplodeTime / gGame.timeStep;
            Mix_PlayChannel(2, se[SE_Bomb], 0); //爆発音を1回再生
        }
        //クリスタルとの距離が近いと爆発
        if (110 > GetCharaObjCenterDistance(ch, &(gObj[0])) && ch->flag != 1) {
            ch->flag = 1;
            ch->rest = ExplodeTime / gGame.timeStep;
            Mix_PlayChannel(2, se[SE_Bomb], 0); //爆発音を1回再生
        }
        break;
    default:
        break;
    }
}

/* キャラの情報更新
 *  対象キャラの情報（状態，行動戦略など）を更新する
 *
 * 引数
 *   ch: 対象キャラ
 */
void UpdateCharaInfo(CharaInfo* ch)
{
    UpdateCharaInfoByPattern(ch);

    switch (ch->stts) {
    /*case Disable:
        switch (ch->type) {

        default:
            break;
        }
        break;
    */
    case Normal:
        break;
    case Stop:
        ch->velocity = 0.0f;
        break;
    case Decelerate:
        // 5秒以上たったら効果終了
        if (gGame.secondTime - ch->limit >= 10)
            ch->stts = Normal;
        break;
    case Reverce:
        // 3秒以上たったら効果終了
        if (gGame.secondTime - ch->limit >= 3)
            ch->stts = Normal;
        break;
    default:
        break;
    }

    if (gGame.inhale) {
        if (ch->type != Player)
            SetDirToPoint(ch, gItem[9].pos);
        if (!gItem[9].isSet)
            gGame.inhale = 0;
    }

    //爆発用のフラグがtrueなら
    if (ch->flag) {
        ch->velocity = 0.0f;
        ch->rest--;
        if (ch->rest < 0) {

            ch->hp   = 0;
            ch->flag = 0;
            if (70 > GetCharaCenterDistance(ch, &(gChara[gGame.player]))) {

                gChara[gGame.player].hp -= ch->power;
                gGame.damageNum += ch->power;
            }
            if (110 > GetCharaObjCenterDistance(ch, &(gObj[0]))) {

                gGame.hp -= ch->power;
                //ポータルをランダムでoffにする
                int rands = rand() % 5;
                for (int i = 0; i < rands; i++)
                    PortalSwitch(rand() % 4, 0, 1);
            }
            RemoveChara(ch->id);
        }
    }
}

/* キャラの移動
 *  対象キャラの座標を（指定フレーム分）更新する
 *
 * 引数
 *   ch: 対象キャラ
 *   frame: フレーム数，巻き戻すときは負数を指定する
 */
void MoveChara(CharaInfo* ch, float frame)
{
    Vector2 newpoint = ch->pos;
    /* 向きに応じた移動(等速運動 p=vt) */
    newpoint.x += ch->dir.x * ch->velocity * gGame.timeStep * frame;
    newpoint.y += ch->dir.y * ch->velocity * gGame.timeStep * frame;

    /* マップ外となったときはマップ内まで戻す*/
    if (newpoint.x < 0)
        newpoint.x = 0.0;
    else if (newpoint.x + ch->image->mask.w >= WD_Width)
        newpoint.x = WD_Width - ch->image->mask.w;
    if (newpoint.y < 0)
        newpoint.y = 0.0;
    else if (newpoint.y + ch->image->mask.h >= WD_Height)
        newpoint.y = WD_Height - ch->image->mask.h;

    /* 座標更新 */
    ch->pos = newpoint;

    SDL_Rect chCol = ch->image->mask;
    chCol.x += ch->pos.x;
    chCol.y += ch->pos.y;
    //壁との判定
    for (int i = 0; i < 4; i++) {
        if (CheckOverLap(chCol, gMap.wall[i], &overlap, 0.0)) {
            AdjustPosition(ch, &overlap);
        }
    }
    //オブジェクトとの判定
    for (int i = 0; i < ObjNum; i++) {
        SDL_Rect objMask = gObj[i].image->mask;
        objMask.x += gObj[i].pos.x;
        objMask.y += gObj[i].pos.y;
        if (CheckOverLap(chCol, objMask, &overlap, 0.0)) {
            AdjustPosition(ch, &overlap);
        }
    }

    //アイテムとの判定

    if (ch->type != Player) {
        for (int i = 0; i < gItemNum; i++) {
            if (gItem[i].isSet) {
                SDL_Rect itemMask = gItem[i].image->trigger;
                itemMask.x += gItem[i].pos.x;
                itemMask.y += gItem[i].pos.y;
                if (CheckOverLap(chCol, itemMask, &overlap, 0.2)) {
                    EffectItem(gItem[i].type, ch);
                } else {
                    // Starとの重なりがないとき
                    if (i == 10) {
                        ch->power = gBaseChara[ch->kind].power;
                    }
                }
            }
        }
    }
}

//２つのオブジェクトの重なりをチェック
SDL_bool CheckOverLap(SDL_Rect a, SDL_Rect b, SDL_Rect* overlap, float percent)
{
    float space = 0.0;

    if (SDL_IntersectRect(&a, &b, overlap)) {
        space += (*overlap).w * (*overlap).h;
    }
    /* キャラ当たり矩形の0%以上で重なっているとする */
    return (space / (a.w * a.h) > percent) ? SDL_TRUE : SDL_FALSE;
}

//重なったキャラと重なった矩形を渡してキャラをずらす
void AdjustPosition(CharaInfo* ch, SDL_Rect* overlap)
{
    if (overlap->w && overlap->h) {
        if (overlap->w < overlap->h) {
            /* x方向の重なりの補正
                overlapはmask範囲の4辺のいずれかに接するので
                接する辺の逆方向に座標を補正する
            */
            if (overlap->x > ch->pos.x) {
                ch->pos.x -= overlap->w;
            } else {
                ch->pos.x += overlap->w;
            }
        } else {
            /* y方向の重なりの補正 */
            if (overlap->y > ch->pos.y) {
                ch->pos.y -= overlap->h;
            } else {
                ch->pos.y += overlap->h;
            }
        }
    }
}

/* キャラchとトリガー範囲が重なっているオブジェクトのidを返す ない場合-1*/
int IsTriggerByObject(CharaInfo* ch, SDL_Rect* overlap, float percent)
{
    int id;

    SDL_Rect chCol = ch->image->mask;
    chCol.x += ch->pos.x;
    chCol.y += ch->pos.y;

    for (id = 0; id < ObjNum; id++) {
        float space         = 0.0;
        SDL_Rect objTrigger = gObj[id].image->trigger;
        objTrigger.x += gObj[id].pos.x;
        objTrigger.y += gObj[id].pos.y;

        if (SDL_IntersectRect(&chCol, &objTrigger, overlap)) {
            space += (*overlap).w * (*overlap).h;
        }
        /* キャラ当たり矩形の0%以上で重なっているとする */
        if (space / (ch->image->mask.w * ch->image->mask.h) > percent)
            return id;
    }
    return -1;
}

/* 当たり判定
 *  対象キャラ同士が重なったか調べ，状態更新などをする
 *
 * 引数
 *   ci: 対象キャラ
 *   cj: 対象キャラ
 */
void Collision(CharaInfo* ci, CharaInfo* cj)
{
    /* 判定が不要な組み合わせを除外 */
    if (ci->stts == Disable || cj->stts == Disable || ci->flag == 1 || cj->flag == 1)
        return;
    /* 当たり矩形をマップ座標に合わせる */
    SDL_Rect mi = ci->image->mask;
    mi.x += ci->pos.x;
    mi.y += ci->pos.y;
    SDL_Rect mj = cj->image->mask;
    mj.x += cj->pos.x;
    mj.y += cj->pos.y;
    /* 当たり（重なり）判定 */
    if (CheckOverLap(mi, mj, &overlap, 0.0)) {
        AdjustPosition(ci, &overlap);
    }
}

void UseItem(void)
{
    int index = CulcEmpNum(gGame.selectItemNum);
    if (index == -1) {
        return;
    }
    if (gItem[index].isUse && !gItem[index].isSet) {
        gItem[index].isSet = 1;
        gItem[index].isUse = 0;
        gItem[index].pos.x = gChara[gGame.player].pos.x - ItemImgSize / 3;
        gItem[index].pos.y = gChara[gGame.player].pos.y - ItemImgSize / 3;
        if (index == 10) {
            gItem[index].pos.x = gChara[gGame.player].pos.x - ItemImgSize * 4 / 3;
            gItem[index].pos.y = gChara[gGame.player].pos.y - ItemImgSize * 4 / 3;
        }

        gItem[index].setTime = gGame.secondTime; //設置された時間を格納

        gGame.itemUseNum += 1;
    }
    if (index == 9) {
        gGame.inhale = 1;
    }
}

int CulcEmpNum(int selected)
{
    int index = 0;
    for (int i = 0; i < selected; i++) {
        index += itemMaxNum[i];
    }
    for (int i = index; i < index + itemMaxNum[selected]; i++) {
        if (!gItem[i].isSet && gItem[i].isUse) {
            itemNum[selected]--;
            return i;
        }
    }
    return -1; //ない場合
}

/*
    アイテムの効果発動を行う関数

    引数：
        type:アイテムの画像タイプ
        ch: キャラ情報へのポインタ
*/
void EffectItem(ItemImgType type, CharaInfo* ch)
{
    switch (type) {
    case IT_Explode:
        if (ch->flag == 0) {
            ch->rest = ExplodeTime / gGame.timeStep;
            Mix_PlayChannel(2, se[SE_Bomb], 0); //爆発音を1回再生
        }
        ch->flag = 1;
        break;
    case IT_Decelerate:
        if (ch->stts != Decelerate)
            ch->limit = gGame.secondTime;
        ch->velocity = ch->basevel / 2.0f;
        ch->stts     = Decelerate;

        break;
    case IT_Reverce:
        if (ch->stts != Reverce)
            ch->limit = gGame.secondTime;
        ch->dir.x = -ch->dir.x;
        ch->dir.y = -ch->dir.y;
        ch->stts  = Reverce;
        break;
    case IT_Star:
        ch->power = 0;
        break;
    default:
        break;
    }
}

/*
    アイテムの切り替えを行う関数

    引数：
        mode:ボタンのずらし方
*/
void ChangeItem(int mode)
{
    gGame.selectItemNum += mode;
    if (gGame.selectItemNum < 0) {
        gGame.selectItemNum = ItemImgNum - 1;
    } else if (gGame.selectItemNum > ItemImgNum - 1) {
        gGame.selectItemNum = 0;
    }
}

/* ポータルのON/OFFを切り替える

    引数：
    index: ON/OFFを切り替えるポータルのid
    mode: 0ならOFFに,1ならON,-1なら状態を逆にする.
    isOn: 0ならisOnは変化せず,1なら変化する
*/
void PortalSwitch(int index, int mode, int isOn)
{
    if (mode == 1 || mode == -1) {
        if (gObj[index].type == OffPortal) {
            gObj[index].flag  = 1;
            gObj[index].type  = OnPortal;
            gObj[index].image = &(gObjImg[OnPortal]);
            if (isOn == 1)
                gGame.isOn++;
            return;
        }
    }

    if (mode == 0 || mode == -1) {
        if (gObj[index].type == OnPortal) {
            gObj[index].flag  = 0;
            gObj[index].type  = OffPortal;
            gObj[index].image = &(gObjImg[OffPortal]);
            if (isOn == 1)
                gGame.isOn--;
            return;
        }
    }
}

/* クリア判定を行う */
int Judge(void)
{
    int ret = 0;

    //クリスタルのHPが0以下なら//プレイヤーのHPが0以下なら
    if (gGame.hp <= 0 || gChara[gGame.player].hp <= 0 || (GameTime - gGame.secondTime) <= 0) {
        ret        = 1;
        gGame.msg  = MS_GAMEOVER;
        gGame.stts = GS_Result;
    }

    //エネルギーが溜まったなら
    if (gGame.energy >= ENE_MAX) {
        ret        = 1;
        gGame.msg  = MS_CLEAR;
        gGame.stts = GS_Result;
    }

    return ret;
}

/* ボタンの選択番号の変更を行う.
    引数:
        button: ボタンの種類
        mode:追加する数字
        numMax: そのボタンの項目の最大数
*/
void SelectButton(int* button, int mode, int numMax)
{
    *button += mode;
    if (*button < 0) {
        *button = numMax - 1;
    } else if (*button > numMax - 1) {
        *button = 0;
    }
}
