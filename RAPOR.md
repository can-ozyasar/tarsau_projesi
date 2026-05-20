# tarsau Arsivleme Programi Proje Raporu

## 1. Proje Bilgileri

**Ders:** Sistem Programlama  
**Proje adi:** tarsau  
**Donem:** 2025-2026 Bahar Donemi  
**Gelistirme dili:** C  
**Calisma ortami:** Linux / Unix terminal ortami

## 2. Ekip Bilgileri

| Ad Soyad | Ogrenci Numarasi |
| --- | --- |
| Muhammed Can Ozyasar | G231210009 |
| Ali | NUMARA_GIRILECEK |

Bu proje Muhammed Can Ozyasar ve Ali tarafindan birlikte hazirlanmistir.

## 3. Projenin Amaci

Bu projede bizden, `tar`, `zip` veya `rar` gibi calisan fakat sikistirma yapmayan bir arsivleme programi yazmamiz istendi. Programin adi `tarsau` olarak belirlendi.

Programin temel amaci, birden fazla ASCII metin dosyasini tek bir `.sau` uzantili arsiv dosyasi icinde toplamak ve daha sonra bu arsivi acarak dosyalari tekrar elde etmektir. Arsiv acildiginda dosyalarin icerikleriyle birlikte izin bilgilerinin de korunmasi gerekmektedir.

Projede dosya islemleri, komut satiri argumanlari, dosya izinleri, dizin olusturma ve hata kontrolu gibi Sistem Programlama dersiyle ilgili konular kullanilmistir.

## 4. Projede Istenen Temel Ozellikler

Proje isterlerine gore programda iki farkli calisma modu bulunmaktadir:

```bash
./tarsau -b dosya1 dosya2 -o arsiv.sau
```

Bu komut arsiv olusturma modudur.

```bash
./tarsau -a arsiv.sau hedef_dizin
```

Bu komut arsiv acma modudur.

Projede dikkat edilen temel kurallar sunlardir:

- Giris dosyalari ASCII metin dosyasi olmalidir.
- En fazla 32 giris dosyasi arsivlenebilmelidir.
- Giris dosyalarinin toplam boyutu 200 MB'i gecmemelidir.
- Arsiv dosyasi adi verilmezse varsayilan olarak `a.sau` kullanilmalidir.
- Arsiv dosyasi `.sau` uzantili olmalidir.
- Arsiv acilirken hedef dizin yoksa olusturulmalidir.
- Acilan dosyalarin izinleri arsivlendigi zamanki izinlerle ayni olmalidir.
- Hatali girislerde program aniden sonlanmamali, anlamli hata mesaji vermelidir.

## 5. Proje Dosya Yapisi

Projenin ana dosya yapisi asagidaki gibidir:

```text
tarsau_projesi/
├── Makefile
├── README.md
├── RAPOR.md
├── tarsau.c
└── t1.txt
```

`tarsau.c` dosyasi programin kaynak kodunu icermektedir.  
`Makefile` dosyasi projeyi kolay derlemek icin kullanilmaktadir.  
`README.md` dosyasi programin genel kullanimini aciklamaktadir.  
`RAPOR.md` dosyasi bu proje raporudur.

## 6. Kullanilan Yontem

Programda once komut satiri argumanlari kontrol edilmektedir. Kullanici `-b` parametresi verirse program arsiv olusturma modunda calisir. Kullanici `-a` parametresi verirse program arsiv acma modunda calisir.

Arsiv olusturma isleminde once giris dosyalari tek tek kontrol edilir. Dosyanin var olup olmadigi, normal bir dosya olup olmadigi ve ASCII karakterlerden olusup olusmadigi incelenir. Daha sonra dosyanin izin bilgisi ve boyutu `stat()` sistem cagrisi ile alinir.

Butun dosyalar kontrol edildikten sonra `.sau` arsiv dosyasi olusturulur. Arsivin basina organizasyon bilgisi yazilir. Bu kisimda her dosyanin adi, izinleri ve boyutu tutulur. Daha sonra dosya icerikleri herhangi bir ayirici kullanilmadan arka arkaya arsive eklenir.

Arsiv acma isleminde ise once arsivin ilk 10 bayti okunur. Bu kisim organizasyon bolumunun kac bayt oldugunu belirtir. Daha sonra organizasyon bolumu okunur ve buradaki kayitlara gore dosyalar hedef dizinde tekrar olusturulur. Her dosya icin arsivden gerekli bayt kadar veri okunur ve dosyaya yazilir. Son olarak `chmod()` ile dosya izinleri eski haline getirilir.

## 7. `.sau` Arsiv Dosyasi Formati

Proje isterlerinde belirtilen arsiv formati kullanilmistir.

Arsiv dosyasi iki ana bolumden olusur:

1. Organizasyon bilgisi
2. Arsivlenmis dosyalarin icerikleri

Dosyanin ilk 10 bayti organizasyon bolumunun boyutunu tutar.

Organizasyon kayitlari su bicimdedir:

```text
|Dosya adi,izinler,boyut|
```

Ornek bir kayit:

```text
|t1.txt,644,13|
```

Bu kayitta:

- `t1.txt` dosya adidir.
- `644` dosya izinidir.
- `13` dosyanin bayt cinsinden boyutudur.

Organizasyon bolumunden hemen sonra dosyalarin icerikleri art arda yazilir. Dosya icerikleri arasinda ekstra bir ayirici kullanilmamistir. Hangi dosyanin kac bayt okunacagi organizasyon bolumundeki boyut bilgisinden anlasilmaktadir.

## 8. Kodun Onemli Bolumleri

Programda dosya bilgilerini tutmak icin `FileInfo` isimli bir yapi kullanilmistir.

```c
typedef struct {
    char archive_name[PATH_MAX];
    const char *source_path;
    mode_t permissions;
    long long size;
} FileInfo;
```

Bu yapida dosyanin arsiv icindeki adi, kaynak yolu, izinleri ve boyutu tutulmaktadir.

ASCII kontrolu icin dosya ikili modda okunur ve her baytin 127'den buyuk olup olmadigi kontrol edilir. 127'den buyuk bir karakter varsa dosya uyumsuz kabul edilir.

```c
if (buffer[i] > 127) {
    fclose(file);
    return 0;
}
```

Arsiv olustururken organizasyon bolumunun boyutu 10 baytlik alan olarak yazilir.

```c
fprintf(archive, "%010zu", index_size);
```

Bu sayede arsiv acilirken program ilk 10 bayti okuyup organizasyon bilgisinin uzunlugunu ogrenebilir.

Dosya izinleri `stat()` ile alinip arsive yazilir. Arsiv acilirken de `chmod()` ile geri yuklenir.

```c
file_infos[i].permissions = file_stat.st_mode & 0777;
chmod(output_path, (mode_t)permissions);
```

## 9. Hata Kontrolleri

Programda hatali girislerin programi cokertmemesi icin cesitli kontroller yapilmistir.

Kontrol edilen durumlar:

- Eksik veya hatali komut parametresi
- `.sau` uzantisi olmayan arsiv dosyasi
- Var olmayan giris dosyasi
- Normal dosya olmayan giris
- ASCII olmayan dosya icerigi
- 32'den fazla dosya girilmesi
- Toplam dosya boyutunun 200 MB'i gecmesi
- Bozuk arsiv basligi
- Hatali organizasyon kaydi
- Dosya yolu guvenligi
- Hedef dizin olusturma hatasi
- Dosya yazma ve izin verme hatalari

ASCII olmayan dosya icin verilen hata mesaji:

```text
dosya_adi giris dosyasinin formati uyumsuzdur!
```

Bozuk veya uygun olmayan arsiv icin verilen hata mesaji:

```text
Arsiv dosyasi uygunsuz veya bozuk!
```

## 10. Derleme

Program `Makefile` ile derlenmektedir.

```bash
make
```

Bu komut sonucunda `tarsau` isimli calistirilabilir dosya olusur.

Derleme ciktilarini temizlemek icin:

```bash
make clean
```

## 11. Ornek Calisma

Asagidaki komutla `t1.txt` dosyasi arsivlenir:

```bash
./tarsau -b t1.txt -o ornek.sau
```

Program cikti olarak sunu verir:

```text
ornek.sau arsivi olusturuldu.
```

Daha sonra arsiv su komutla acilir:

```bash
./tarsau -a ornek.sau cikti
```

Program cikti olarak sunu verir:

```text
cikti dizinine 1 dosya acildi.
```

Bu islemden sonra `cikti` dizini icinde `t1.txt` dosyasi olusur.

## 12. Testler

Projede farkli durumlar denenmistir.

Ilk olarak normal bir ASCII metin dosyasi arsivlenmis ve tekrar acilmistir. Dosyanin icerigi acildiktan sonra ayni kalmistir.

Ikinci olarak birden fazla dosya ve alt dizin iceren dosya yolu denenmistir. Program bu dosyalari arsive ekleyip hedef dizinde tekrar olusturmustur.

Ucuncu olarak dosya izinleri test edilmistir. Ornek olarak `640` ve `755` izinlerine sahip dosyalar arsivlenmis, arsiv acildiginda izinlerin korundugu gorulmustur.

Dorduncu olarak ASCII olmayan karakter iceren bir dosya denenmistir. Program bu dosyayi kabul etmemis ve uygun hata mesajini vermistir.

Son olarak bozuk bir `.sau` dosyasi denenmistir. Program arsivi acmaya calisirken hata vermis ve cokmeden sonlanmistir.

## 13. Sonuc

Bu projede C dili kullanilarak komut satirindan calisan bir arsivleme programi gelistirilmistir. Program sikistirma yapmadan metin dosyalarini `.sau` uzantili tek bir arsiv dosyasinda birlestirmektedir. Arsiv acma sirasinda dosyalarin icerikleri ve izinleri geri yuklenmektedir.

Proje sayesinde dosya okuma-yazma islemleri, dosya izinleri, dizin olusturma, komut satiri argumanlari ve hata kontrolu konulari pratik olarak uygulanmistir. Program proje isterlerinde belirtilen temel ozellikleri karsilayacak sekilde hazirlanmistir.

