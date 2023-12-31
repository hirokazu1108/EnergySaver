# EnergySaver
<p  align="center"><img width="521" alt="image" src="https://github.com/hirokazu1108/EnergySaver/assets/87222170/5cf50cc3-80ee-4956-9c7a-930c3019a224"></p>

- ### 作品名
   Energy Saver

- ### 概要
   学部2年生前期の講義「ソフトウェア設計及び実験」で2022年6月末から2023年7月中旬にかけて約3週間で作成した個人開発ゲームです.C言語とSDL2を利用して開発を行いました.

   この制作では以下を使用しています.
   - C言語
   - [SDL2](https://www.libsdl.org/)

- ### プレイ動画・マニュアル
   - [プレイ動画（Youtube）](https://www.youtube.com/watch?v=O7UEjkAv2NY&list=PLq43FGDaDfJTbCs2VUPctQjEVikHAY_2U&index=1)
   - [プレイ動画データ（Google Driveに移動）](https://drive.google.com/drive/u/0/folders/1SwU_pk9CbYyyfkR2DmZt1eWglQIDIqkJ)

- ### ゲームコンセプト
   アイテムを駆使してエネルギーをためろ！

- ### プレイ人数
   1人


- ### プレイ時間
   3分


- ### 使用機器・環境
   Linux環境
  
   PC(キーボード)またはJoyCon(右)


- ### ゲームルール
  - #### クリア条件
    ポータルをONにした状態でクリスタルに触れることで,ONになっているポータルの数だけエネルギーがたまる.
  
    このエネルギーをクリスタルに貯めきるとゲームクリア！
    
  - #### ゲームオーバー条件
    - クリスタルの耐久値が0になる
    - プレイヤーのHPが0になる
    - 残り時間が00:00になる
  - #### 敵の攻撃
    敵はクリスタルとプレイヤーの近くで爆発する

    プレイヤーが爆発範囲にいるとHPが減る

    クリスタルが爆発範囲にあると耐久値が減る.また,ランダムでONのポータルがOFFになる
    
  - #### アイテム
    5種類のアイテムを使用して,ゲームを有利に進めよう
    
    - 各アイテムの効果
      - 誘発床　触れた敵を爆発させる
      - 減速床　触れた敵を一定時間減速させる
      - 反転床　触れた敵を一定時間逆方向に移動させる
      - 吸引床　一定時間すべての敵をこの床に向かわせる
      - 無敵床　床の範囲内での爆発を無効にする

- ### 操作方法
  <table border="1">
    <caption> キーボード </caption>
    <tr>
      <th>キー</th>
      <th>説明</th>
    </tr>
    <tr>
      <td>Wキー</td>
      <td>上方向へ移動</td>
    </tr>
    <tr>
      <td>Sキー</td>
      <td>下方向へ移動</td>
    </tr>
    <tr>
      <td>Aキー</td>
      <td>左方向へ移動</td>
    </tr>
    <tr>
      <td>Dキー</td>
      <td>右方向へ移動</td>
    </tr>
    <tr>
      <td>カーソルキー</td>
      <td>アイテムの変更</td>
    </tr>
    <tr>
      <td>スペースキー</td>
      <td>アイテムの使用</td>
    </tr>
    <tr>
      <td rowspan="2">エンターキー</td>
      <td>（クリスタルの近くで）エネルギーチャージ</td>
    </tr>
    <tr>
       <td>（ポータルの近くで）ON/OFF切り替え</td>
    </tr>
    <tr>
      <td>テンキー4,5,7,8,9</td>
      <td>各アイテムに変更＆アイテムを使用</td>
    </tr>
  　<tr>
       <td>EscキーまたはQキー</td>
       <td>ゲーム終了</td>
    </tr>
  </table>

  <table border="1">
    <caption> JoyCon（右/横持ち） </caption>
    <tr>
      <th>ボタン</th>
      <th>説明</th>
    </tr>
    <tr>
      <td>スティック</td>
      <td>移動</td>
    </tr>
    <tr>
      <td>BボタンまたはXボタン</td>
      <td>アイテムの変更</td>
    </tr>
    <tr>
      <td>Yボタン</td>
      <td>アイテムの使用</td>
    </tr>
    <tr>
      <td rowspan="2">スティック押し込み</td>
      <td>（クリスタルの近くで）エネルギーチャージ</td>
    </tr>
    <tr>
       <td>（ポータルの近くで）ON/OFF切り替え</td>
    </tr>
    <tr>
       <td>ホームボタン</td>
       <td>ゲーム終了</td>
    </tr>
  </table>
- ### 実行方法
   #### 1.zipファイルをダウンロード後,展開する
   
   #### 2.Linuxターミナルでenergy_saverディレクトリに移動し ./mainを実行（JoyConを利用する場合は ./main joycon）

   #### ※ファイル権限がない場合は chmod 777 main を入力してください
   
- ### 使用音源
   - [Springin’ Sound Stock](https://www.springin.org/sound-stock/)
   - [魔王魂](https://maou.audio/)
   - [Music-Note.jp](https://www.music-note.jp/)
