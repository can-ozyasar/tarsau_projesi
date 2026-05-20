# tarsau - Metin Dosyasi Arsivleme Programi

## Proje Bilgileri

Bu proje, **Bilgisayar Muhendisligi Sistem Programlama 2025-2026 Bahar Donemi Projesi** kapsaminda gelistirilmistir.

**Proje adi:** tarsau  
**Programlama dili:** C  
**Calisma ortami:** Linux / Unix  
**Amac:** Metin dosyalarini sikistirma yapmadan tek bir `.sau` arsiv dosyasinda birlestirmek ve daha sonra ayni icerik ve izinlerle geri acmak.

## Ekip

| Ad Soyad | Ogrenci Numarasi |
| --- | --- |
| Muhammed Can Ozyasar | G231210009 |
| Ali Elhüseyin  | B201210555 |

## Projenin Amaci

`tarsau`, `tar`, `zip` veya `rar` gibi arsivleme araclarina benzer sekilde calisan, ancak herhangi bir sikistirma islemi yapmayan bir komut satiri programidir. Program yalnizca ASCII metin dosyalarini kabul eder ve bu dosyalari tek bir `.sau` arsiv dosyasi icinde saklar.

Arsiv acma islemi sirasinda dosyalar hedef dizine geri yazilir ve dosyalarin okuma, yazma, calistirma izinleri korunur.

## Klasor Yapisi

```text
tarsau_projesi/
├── Makefile
├── README.md
├── tarsau.c
└── t1.txt
```

Ana kaynak kod `tarsau.c` dosyasindadir. Proje `Makefile` ile derlenir.

## Derleme

Projeyi derlemek icin `tarsau_projesi` dizininde asagidaki komut calistirilir:

```bash
make
```

Bu komut sonucunda `tarsau` adinda calistirilabilir dosya olusur.

Derleme ciktilarini temizlemek icin:

```bash
make clean
```

Ornek bir arsivleme ve acma islemi calistirmak icin:

```bash
make run-example
```

## Kullanim

### Arsiv Olusturma

```bash
./tarsau -b dosya1.txt dosya2.txt -o arsiv.sau
```

`-b` parametresi arsiv olusturma modudur. `-o` parametresinden sonra cikti arsiv dosyasinin adi verilir.

Arsiv dosyasi belirtilmezse varsayilan cikti dosyasi `a.sau` olur:

```bash
./tarsau -b dosya1.txt dosya2.txt
```

### Arsiv Acma

```bash
./tarsau -a arsiv.sau hedef_dizin
```

`-a` parametresi arsiv acma modudur. Ikinci parametre hedef dizindir.

Hedef dizin verilmezse dosyalar gecerli dizine acilir:

```bash
./tarsau -a arsiv.sau
```

## Desteklenen Kurallar ve Sinirlar

- Giris dosyalari ASCII metin dosyasi olmalidir.
- Giris dosyasi sayisi en fazla 32 olabilir.
- Giris dosyalarinin toplam boyutu 200 MB'i gecemez.
- Arsiv dosyasinin uzantisi `.sau` olmalidir.
- Arsiv acilirken hedef dizin yoksa program tarafindan olusturulur.
- Dosyalar acildiktan sonra orijinal izinleri korunur.
- Program hatali girislerde veya bozuk arsivlerde aniden cokmez; uygun hata mesaji vererek sonlanir.

## `.sau` Arsiv Dosyasi Formati

Arsiv dosyasi iki ana bolumden olusur:

1. Organizasyon bilgisi
2. Arsivlenmis dosya icerikleri

Dosyanin ilk 10 bayti organizasyon bolumunun boyutunu ASCII sayisal formatta tutar.

Organizasyon bolumundeki her kayit su formattadir:

```text
|Dosya adi,izinler,boyut|
```

Ornek:

```text
0000000021|t1.txt,644,13|Merhaba dunya
```

Bu ornekte:

- `0000000021`: Organizasyon bolumunun bayt cinsinden uzunlugu
- `t1.txt`: Arsivlenen dosya adi
- `644`: Dosya izinleri
- `13`: Dosya boyutu
- Devam eden kisim: Dosyanin ASCII icerigi

## Hata Kontrolleri

Program asagidaki durumlari kontrol eder:

- Dosyanin var olup olmadigi
- Dosyanin normal dosya olup olmadigi
- Dosyanin ASCII karakterlerden olusup olusmadigi
- Dosya sayisi siniri
- Toplam dosya boyutu siniri
- Arsiv dosyasi uzantisinin `.sau` olup olmadigi
- Arsiv baslik bilgisinin gecerli olup olmadigi
- Organizasyon kayitlarinin beklenen formatta olup olmadigi
- Hedef dizin ve alt dizinlerin olusturulabilir olup olmadigi
- Dosya izinlerinin geri yuklenebilir olup olmadigi

ASCII olmayan bir giris dosyasi verildiginde program su mesaji uretir:

```text
dosya_adi giris dosyasinin formati uyumsuzdur!
```

Uygunsuz veya bozuk bir arsiv dosyasi verildiginde:

```text
Arsiv dosyasi uygunsuz veya bozuk!
```

## Ornek Calisma

```bash
make
./tarsau -b t1.txt -o ornek.sau
./tarsau -a ornek.sau cikti
```

Beklenen sonuc:

```text
ornek.sau arsivi olusturuldu.
cikti dizinine 1 dosya acildi.
```

## Temizleme

```bash
make clean
```

Bu komut derleme ciktilarini, ornek `.sau` dosyalarini ve ornek cikti dizinini temizler.

