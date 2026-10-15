[English](RELEASE-NOTES.md) | **日本語**

# DEsk v0.1.2 (BETA)

## v0.1.1 からの変更

- **Fedora**：Fedora 43・44（と、Nobara などその派生）のインストーラ、`sudo sh scripts/install_fedora.sh`。wlroots 0.18（`wlroots0.18`）も含め、すべて Fedora の公式リポジトリから入れます。wlroots 0.18 の無い Fedora のときだけ、公式のリリースからビルドします。「おすすめ」だけのパッケージは、Debian・Ubuntu と同じく入れません。Fedora に無いもの（メモ帳の Klee One は Noto Serif CJK で代わりに、Mugshot、電池と更新の知らせ）は入れずに済ませます。`install_session.sh` も Fedora を見分け、Shell Settings は開くものの無い行に Fedora のパッケージ名を示します。
- **直したこと：Arch のインストーラが wlroots のビルドで止まっていた**（AUR の `wlroots0.18` を入れていない Arch で、「mkdir: cannot create directory」）。原因と直し方を見つけてくれた Discord の Axenity さんに感謝します。
- **直したこと：DEsk からログアウトしたあと、次のデスクトップ（Plasma など）が真っ暗になっていた**。DEsk のセッションが終わるときに、利用者の systemd に渡したものを取り消し、起動したポータルを止めるようにしました。
- **直したこと：ポストから取り出した手紙を捨てられなかった**。机の上の手紙をクリックすると捨てられます。
- **新しく：`desk-report`**。不具合の報告に要るものを、個人のものを取り除いて一つのファイル（`~/desk-report.txt`）にまとめます。端末で実行して、そのファイルを添えてください。
- **直したこと：端末の中で動くプログラム（btop）や、窓を別の名前で名乗るプログラム（KDE のシステム設定）を開くと、空の紙が約 45 秒残っていた**。窓は起動したプログラムをたどって自分の紙を見つけ、窓を出さずに終わったプログラムの紙は数秒で片付きます。

## v0.1.1 での変更

- **直したこと：カレンダーやメモ帳を開くと机が止まっていた**（GPU で描く画面で）。GPU の画面では、すでに渡した絵の大きさを読み直していて、画面全体の大きさで置いていました。
- **入れ方を簡単に**：README の一番上に、システムごとに一行（`sudo sh scripts/install_debian.sh`、`install_ubuntu.sh`、`install_arch.sh`）。各インストーラが自分で `SHA256SUMS` を照合するようにしました。

更新は、新しいリリースを展開して同じ行をもう一度実行するだけです。設定は残ります。

## DEsk について

DEsk は、iOS 6 のころのような**リアルなスキューモーフィズム**で作った、デスクトップが机になっている Linux 向けの Wayland デスクトップです。窓は紙、ドックは引き出し、プログラムは本棚のバインダーに、通知はポストに届き、一つの電気スタンドがレイトレースの部屋を照らします。

## 入っているもの

- **デスクトップ一式**：ウィンドウマネージャーだけでなく、Wayland コンポジタ（wlroots 0.18）に、通知デーモン、システムトレイ、メディアの操作、画面ロック、離席と画面の消灯、画面の配置の記憶、そして polkit エージェント・キーリング・入力メソッド・ネットワークのアプレットを起動するセッションまで。
- **紙の窓**：たたむ、机いっぱいに広げる、閉じると丸めて投げる。たたんだ窓はほかの窓の下で机に置かれるか、引き出しに入ります。
- **引き出し**（ドック）、8 冊のバインダーと検索できる ALL APPS の本が並ぶ**本棚**、**Control Panel**（メーター、Wi-Fi と Bluetooth のレバー、音量、明るさ、トレイ）、**ポスト**、付箋の貼れる**卓上カレンダー**、**メモ帳**、メディアの画面のある**時計**。
- 画面ロックは**玄関の扉**。実際の時刻の光と、選べば実際の天気。
- **Shell Settings**。機械のほかの設定に借りている自由なプログラムもここから開け、各行に使うプログラムの名前が書いてあります。
- **7 か国語**：英語・日本語・中国語（簡体字）・韓国語・スペイン語・フランス語・ドイツ語。カレンダーはその国の書き方で、中国語と韓国語はそれぞれの字形の CJK 書体で。
- **二つ以上の画面**：一つの画面に机、ほかの画面には窓を置ける机の続き。
- **5 段階の画質**：Ultra から、画面を CPU で描いているとき（GPU が無い、3D の無い仮想マシン）に自動で選ばれる 2D Fallback まで。
- **インストーラ**：Debian 13、Ubuntu 24.04・26.04、Arch Linux、Fedora 43・44（と派生）向け。DEsk、必要なもの、昔ながらの GTK のプログラム一式を入れます。最初に `SHA256SUMS` でコピーを確かめます。

## インストール

リリース（`DEsk-v0.1.2-BETA.zip` か `.tar.gz`）を展開してそのフォルダに入り、使っているシステムの行を実行します。

```sh
sudo sh scripts/install_debian.sh    # Debian 13
sudo sh scripts/install_ubuntu.sh    # Ubuntu 24.04 / 26.04、Linux Mint、Pop!_OS
sudo sh scripts/install_arch.sh      # Arch Linux、EndeavourOS、Manjaro
sudo sh scripts/install_fedora.sh    # Fedora 43 / 44、Nobara
```

ログアウトして、ログイン画面で **DEsk** を選びます。詳しいこと、オプション（`--yes`、`--no-apps`、`--bare`、`--prefix`）、自分でビルドする方法は README にあります。あとで更新するときは、新しいコピーから同じ行を実行します。設定と焼いた部屋は残ります。

ある画面の大きさで初めて起動するときは部屋を焼きます。GPU なら数秒、プロセッサではもっとかかります（机の上の手紙がそう言います）。そのあとはディスクに残ります。

## 分かっている制限

- **まだ対応していないプロトコル**：layer-shell（waybar などのパネルや、範囲を選ぶ slurp）、session-lock（外部の画面ロッカー）、gamma-control（夜間の色温度）、input-method-v2（画面キーボード）、pointer-constraints と relative-pointer（ゲームや仮想マシンでのポインタの固定）、fractional-scale。
- **高密度の画面**：DEsk は自分の物をポイントで大きさを決めますが、プログラムにはまだ倍率を伝えないので、HiDPI の画面ではプログラムの窓が小さく出ます。
- **二つ以上の画面**：机のある画面以外は、窓を置くための机の続きだけです。本棚・Control Panel・ポストは机の画面にだけあります。
- **X11 のプログラム**は Xwayland で動きます。`org.freedesktop.ScreenSaver` で画面を点けたままにするもの（一部の動画プレーヤーやゲーム）には、まだ応えられません。
- **サスペンドと休止**のボタンはまだありません（ログアウト・再起動・電源オフはあります）。
- 英語と日本語以外の**訳**は、まだその言語の話者に見てもらっていません。
- **プラグインの API はありません**：DEsk を変えるにはソースを変えます（[docs/MODDING.ja.md](docs/MODDING.ja.md)）。
- これまでに試した実機：VMware の仮想マシン（SVGA3D）と ASUS ZenBook Duo（GPU 二つ、画面二つ）。ほかの機械での報告を歓迎します。
- **Fedora** は、まっさらな Fedora 44 のコンテナで試しました（インストーラを最初からと更新として、wlroots の無い Fedora のためのビルド、入れた机を画面なしで動かす）。Fedora の機械のログイン画面（GDM、SDDM）から入るところは、まだ試していません。

## どう確かめたか

4 つの構成（最小、`-Werror`、`_FORTIFY_SOURCE=3`、AddressSanitizer＋UndefinedBehaviorSanitizer）で警告 0 のビルド。厳しい警告と GCC の静的解析を全ファイルに。ThreadSanitizer・AddressSanitizer・UndefinedBehaviorSanitizer で動かし、わざと壊した設定・データ・`.desktop` ファイルも与えました。すべての眺めを 7 か国語で描画し、訳の表は `meson test` で確かめています。詳しくは [docs/ARCHITECTURE.ja.md](docs/ARCHITECTURE.ja.md) に。

## コピーの確かめ方

フォルダには `SHA256SUMS` が入っていて、インストーラがビルドの前に確かめます。アーカイブには `DEsk-v0.1.2-BETA.tar.gz.sha256` が付いています。

```sh
sha256sum -c DEsk-v0.1.2-BETA.tar.gz.sha256
```

## ライセンス

MIT。[LICENSE](LICENSE) を見てください。
