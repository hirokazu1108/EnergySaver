# EnergySaver

<p  align="center"><img width="521" alt="image" src=""></p>

- ### 作品名
   Energy Saver

- ### 概要
   学部2年生前期の講義「ソフトウェア設計及び実験」における共同開発により作成した個人開発ゲームです.C言語とSDL2を利用して開発を行いました.

   この制作では以下を使用しています.
   - C言語
   - [SDL2](https://www.libsdl.org/)

- ### プレイ動画・マニュアル
   [プレイ動画（Google Driveに移動）]()

- ### ゲームコンセプト
   アイテムを駆使してエネルギーをためろ！

- ### プレイ人数
   1人


- ### プレイ時間
   1～3分


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
  </table>

- ### 実行方法
   #### 1.zipファイルをダウンロード後,展開する
   
   #### 2.Linuxターミナルでenergy_saverディレクトリに移動し ./mainを実行（JoyConを利用する場合は ./main joycon）

   #### ※ファイル権限がない場合は chmod 777 main を入力してください
   

