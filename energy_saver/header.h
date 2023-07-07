#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <joyconlib.h>

#define ExplodeTime 0.3 //爆発の時間
#define GameTime 180 //制限時間

/*JoyCon*/
#define DZONE 0.3 // JoyConのスティック感度


typedef struct {
    float x;
    float y;
} Vector2;

/* ゲーム終了時の表示メッセージ */
typedef enum {
    MS_NONE,
    MS_CLEAR,
    MS_GAMEOVER
} GameMsg;
#define MSG_NUM 3

/* SEのタイプ */
typedef enum {
    SE_Bomb,
    SE_Click,
    SE_Energy,
    SE_SwitchPortal,
} SeType;
#define SeNum 4

/* UI画像タイプ */
typedef enum {
    UI_Back       = 0,
    UI_GreenGauge = 1,
    UI_BlueGauge  = 2,
    UI_Flame      = 3,
    UI_Title      = 4,
    UI_Clear      = 5,
    UI_GameOver   = 6,
    UI_Dufficulty = 7,
    UI_Manual     = 8,
} UiImgType;
#define uiImgNum 9
#define GAUGE_WIDTH 201 //ゲージ画像の幅

/* オブジェクト画像タイプ */
typedef enum {
    Crystal   = 0,
    OffPortal = 1,
    OnPortal  = 2,
    Spawner   = 3
} ObjImgType;

typedef enum {
    Player = 0,
    Bom1   = 1,

} CharaImgType;

/* キャラクター行動パターン */
typedef enum {
    CP_Input = 0, /* ユーザー入力による */
    CP_Bom1  = 1, /*プレイヤーの近くで爆発*/
    CP_Bom2  = 2, /* プレイヤーを追う */
    CP_Boss  = 3  /* プレイヤーに近づく */
} CharaPat;

/* キャラの状態 */
typedef enum {
    Normal     = 0,
    Stop       = 1,
    Disable    = 3,
    Decelerate = 4,
    Reverce    = 5,
    Star       = 6
} CharaStts;

/* アイテム画像タイプ */
typedef enum {
    IT_Explode    = 0,
    IT_Decelerate = 1, //減速床
    IT_Reverce    = 2,
    IT_Inhale     = 3,
    IT_Star       = 4
} ItemImgType;

typedef struct {
    int w;            /* キャラ(画像1つ)の幅 */
    int h;            /* 　　　　　　　高さ */
    SDL_Rect mask;    //当たり判定
    SDL_Rect trigger; //トリガー範囲
    SDL_Texture* img;
} ObjImgInfo;

#define WD_Width 960
#define WD_Height 960
#define UIWD_Width 400

#define ObjImgNum 4 //オブジェクト画像の種類(クリスタル,offポータル,onポータル)
/* マップ情報 */
typedef struct {
    ObjImgType type; //オブジェクトの種類
    int area;
    int id;            //オブジェクトのid
    Vector2 pos;       //座標
    int flag;          //フラグ(ポータルのon/offに使用)
    ObjImgInfo* image; //オブジェクトの画像情報
} ObjectInfo;

typedef struct {
    SDL_Rect wall[4];
    SDL_Texture* mapImg;
} MapInfo;

typedef enum {
    GS_Title,
    GS_End,     /* 終了 */
    GS_Playing, /* 通常 */
    GS_Result,
    GS_Select,
} GameStts;

//ゲーム難易度
typedef enum {
    G_Easy,
    G_Normal,
    G_Hard,
} G_Mode;

/* 入力の状態 */
typedef struct {
    SDL_bool up;
    SDL_bool right;
    SDL_bool left;
    SDL_bool down;
} InputStts;

/* ゲームの情報 */
typedef struct {
    GameStts stts;
    GameMsg msg;
    int hp;            //クリスタルの耐久
    int isOn;          //ポータルのOn数
    int energy;        //クリスタルのエネルギー量
    int selectItemNum; //選んでいるアイテムの番号
    InputStts input;
    int player;     /* プレイヤーの番号 */
    float timeStep; /* 時間の増分(1フレームの時間,s) */
    int rest;       /* 残り時間(特殊な動作，アニメーションなどをするのに必要な，フレーム数) */
    SDL_Texture* msgTexts[3];
    SDL_Window* window;
    SDL_Renderer* render;
    int secondTime; //時間(秒)
    int inhale;     //収集フラグ

    int itemUseNum; //総アイテム使用回数
    int damageNum;  //総ダメージ数
    int clearTime;  //クリアした時間を格納

    G_Mode mode; //難易度
    int spawnTime[3];
} GameInfo;
#define CMAX_HP 10 //クリスタルの最大HP
#define ENE_MAX 10 //エネルギーの最大,ポータルの数

/*キャラ画像の情報*/
typedef struct {
    int anipatnum; /* アニメーションパターン数 */
    int dirpatnum; /* 向きパターン数 */
    int w;         /* キャラ(画像1つ)の幅 */
    int h;         /* 　　　　　　　高さ */
    SDL_Rect mask; /* 当たり判定の範囲 */
    SDL_Texture* img;
} CharaImgInfo;
#define CHARAIMG_NUM 2

typedef struct {
    int kind; // baseキャラの識別
    int id;
    int hp;
    int power; //攻撃力
    int area;
    CharaImgType type; /* 画像タイプ */
    CharaStts stts;    /* キャラの状態 */
    CharaPat pattern;
    Vector2 pos;
    int flag; //フラグ(爆発するかなどに使用,1ならする)
    float mass;
    float basevel;       /* 速度の大きさの基準値 */
    float velocity;      /* 現在の速度の大きさ */
    Vector2 dir;         /* 現在の方向（大きさ1となる成分） */
    int rest;            /* 残り時間(特殊な動作，アニメーションなどをするのに必要な，フレーム数) */
    int limit;           /* アイテム作用した時間を入れる */
    int anipat;          /* 現在のアニメーションパターン */
    CharaImgInfo* image; //キャラの画像情報
} CharaInfo;
#define CharaNumMax 20
#define PMAX_HP 5 //プレイヤーの最大HP

typedef struct {
    SDL_Rect trigger; //トリガー範囲
    SDL_Texture* img;
} ItemImgInfo;
#define ItemImgSize 76

typedef struct {
    ItemImgType type;
    int area;
    int isSet;   //設置されているかどうか
    int isUse;   //使えるかどうか
    int setTime; //設置された時間
    Vector2 pos; //座標
    ItemImgInfo* image;
} ItemInfo;
#define ItemImgNum 5 //アイテム画像の種類

extern int PrintError(const char* str);

/* window.c の関数プロトタイプ宣言*/
extern int InitWindow(void);
extern void DestroyWindow(void);
extern int MakeText(void);
extern void RenderWindow(void);
extern void RenderTitleWindow(void);
extern void RenderSelectWindow(void);
extern void RenderResultWindow(void);

/* system.c の関数プロトタイプ宣言*/
extern int InitSystem(const char* chara_data_file, const char* map_data_file, const char* item_data_file);
extern void DestroySystem(void);
extern int LoadSound(void);
extern void InitBaseCharaInfo(CharaInfo* chara);
extern void InitInfo(GameInfo* game);
extern int CreateCharaInfo(CharaInfo* chara, int index);
extern void RemoveChara(int index);
extern Vector2 DiffVec(Vector2 a, Vector2 b);
extern float CulcAngle(Vector2 a, Vector2 b);
extern void SetInput(void);
extern void UpdateCharaInfo(CharaInfo* ch);
extern void MoveChara(CharaInfo* ch, float frame);
extern SDL_bool CheckOverLap(SDL_Rect a, SDL_Rect b, SDL_Rect* overlap, float percent);
extern void AdjustPosition(CharaInfo* ch, SDL_Rect* overlap);
extern int IsTriggerByObject(CharaInfo* ch, SDL_Rect* overlap, float percent);
extern void Collision(CharaInfo* ci, CharaInfo* cj);
extern void UseItem(void);
extern int CulcEmpNum(int selected);
extern void EffectItem(ItemImgType type, CharaInfo* ch);
extern void ChangeItem(int mode);
extern void PortalSwitch(int index, int mode, int isOn);
extern int Judge(void);
extern void SelectButton(int* button, int mode, int numMax);

/* system.c も外部変数宣言*/
extern SDL_Texture* uiImg[uiImgNum];
extern MapInfo gMap;
extern GameInfo gGame;
extern CharaImgInfo gCharaImg[CHARAIMG_NUM];
extern CharaInfo* gBaseChara;
extern CharaInfo* gChara;
extern ObjImgInfo gObjImg[ObjImgNum];
extern ObjectInfo* gObj;
extern ItemImgInfo gItemImg[ItemImgNum];
extern ItemInfo* gItem;
extern Mix_Chunk* se[SeNum];
extern Mix_Music* bgm[2];
extern int titleButtonNum;
extern int modeButtonNum;
extern int resultButtonNum;
extern int manualFlag;
extern Vector2 manualPos;
extern int gItemNum;
extern int gCharaNum;
extern int gBaseCharaNum;
extern int ObjNum;
extern int itemNum[ItemImgNum];
extern int itemMaxNum[ItemImgNum];
extern SDL_Rect overlap;
extern const int itemEffectTime[ItemImgNum];
extern const int itemReloadTime[ItemImgNum];

/* window.c も外部変数宣言*/
extern SDL_Texture* numTexture[11];
extern SDL_Rect size_numTexture;

/* main.c も外部変数宣言*/
extern joyconlib_t jc_r;

extern void TestRender(void);
