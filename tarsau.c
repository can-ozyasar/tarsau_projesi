/**
 * BSM301 - Sistem Programlama
 * Proje: tarsau - Dosya Arsivleme Araci
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_TOTAL_SIZE (200 * 1024 * 1024) // 200 MB siniri (Bayt cinsinden)

typedef struct {
    char ad[256];
    mode_t izinler;
    long boyut;
} DosyaBilgisi;

// Fonksiyon prototipleri (Burada sadece isimleri olmali)
void arsivle(int dosya_sayisi, char *dosyalar[], const char *cikis_dosyasi);
void arsiv_ac(const char *arsiv_dosyasi, const char *hedef_dizin);
int is_ascii_file(const char *dosya_adi);

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Kullanim: %s -b dosya1 dosya2 ... -o arsiv.sau\n", argv[0]);
        fprintf(stderr, "Veya    : %s -a arsiv.sau dizin_adi\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "-b") == 0) {
        char *cikis_dosyasi = "a.sau"; 
        int dosya_sayisi = 0;
        char *dosyalar[32]; 

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-o") == 0) {
                if (i + 1 < argc) {
                    cikis_dosyasi = argv[i + 1];
                }
                break; 
            } else {
                if (dosya_sayisi < 32) {
                    dosyalar[dosya_sayisi] = argv[i];
                    dosya_sayisi++;
                }
            }
        }
        arsivle(dosya_sayisi, dosyalar, cikis_dosyasi);
    } 
    else if (strcmp(argv[1], "-a") == 0) {
        const char *arsiv_dosyasi = argv[2];
        const char *hedef_dizin = (argc > 3) ? argv[3] : "."; 
        arsiv_ac(arsiv_dosyasi, hedef_dizin);
    } 
    else {
        fprintf(stderr, "Hata: Gecersiz parametre. Lutfen '-b' veya '-a' kullanin.\n");
        return 1;
    }

    return 0;
}

int is_ascii_file(const char *dosya_adi) {
    FILE *file = fopen(dosya_adi, "rb");
    if (file == NULL) return 0; 
    
    int ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ch > 127 || ch < 0) { 
            fclose(file);
            return 0; 
        }
    }
    fclose(file);
    return 1; 
}

void arsivle(int dosya_sayisi, char *dosyalar[], const char *cikis_dosyasi) {
    DosyaBilgisi bilgiler[32]; 
    long toplam_boyut = 0;
    struct stat st;

    printf("Giris dosyalari kontrol ediliyor...\n");

    for (int i = 0; i < dosya_sayisi; i++) {
        if (!is_ascii_file(dosyalar[i])) {
            fprintf(stderr, "\"%s\" giris dosyasinin formati uyumsuzdur!\n", dosyalar[i]);
            exit(0);
        }
        if (stat(dosyalar[i], &st) == -1) {
            fprintf(stderr, "Hata: \"%s\" dosyasi bulunamadi veya okunamadi.\n", dosyalar[i]);
            exit(1);
        }
        strcpy(bilgiler[i].ad, dosyalar[i]);
        bilgiler[i].izinler = st.st_mode & 0777; 
        bilgiler[i].boyut = st.st_size;
        toplam_boyut += st.st_size;
    }

    if (toplam_boyut > MAX_TOTAL_SIZE) {
        fprintf(stderr, "Hata: Giris dosyalarinin toplam boyutu 200 MB'i gecemez!\n");
        exit(1);
    }

    FILE *arsiv = fopen(cikis_dosyasi, "wb"); 
    if (arsiv == NULL) {
        fprintf(stderr, "Hata: Cikis dosyasi (%s) olusturulamadi.\n", cikis_dosyasi);
        exit(1);
    }

    char organizasyon_bilgisi[8192] = ""; 
    char gecici_bilgi[512];

    for (int i = 0; i < dosya_sayisi; i++) {
        sprintf(gecici_bilgi, "|%s,%o,%ld|", bilgiler[i].ad, bilgiler[i].izinler, bilgiler[i].boyut);
        strcat(organizasyon_bilgisi, gecici_bilgi);
    }

    int organizasyon_boyutu = strlen(organizasyon_bilgisi);
    fprintf(arsiv, "%-10d", organizasyon_boyutu); 
    fprintf(arsiv, "%s", organizasyon_bilgisi);

    for (int i = 0; i < dosya_sayisi; i++) {
        FILE *kaynak = fopen(dosyalar[i], "rb");
        if (kaynak != NULL) {
            int ch;
            while ((ch = fgetc(kaynak)) != EOF) {
                fputc(ch, arsiv);
            }
            fclose(kaynak);
        }
    }

    fclose(arsiv);
    printf("Dosyalar birlestirildi.\n"); 
}

void arsiv_ac(const char *arsiv_dosyasi, const char *hedef_dizin) {
    printf("'%s' arsivi aciliyor...\n", arsiv_dosyasi);

    struct stat st = {0};
    if (stat(hedef_dizin, &st) == -1) {
        #ifdef _WIN32
            mkdir(hedef_dizin); 
        #else
            mkdir(hedef_dizin, 0777); 
        #endif
    }

    FILE *arsiv = fopen(arsiv_dosyasi, "rb");
    if (arsiv == NULL) {
        fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n"); 
        exit(1);
    }

    char boyut_str[11] = {0};
    fread(boyut_str, 1, 10, arsiv);
    int org_boyut = atoi(boyut_str);

    if (org_boyut <= 0) {
        fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
        fclose(arsiv);
        exit(1);
    }

    char *org_bilgisi = (char *)malloc(org_boyut + 1);
    if (org_bilgisi == NULL) {
        fprintf(stderr, "Bellek hatasi!\n");
        fclose(arsiv);
        exit(1);
    }
    
    fread(org_bilgisi, 1, org_boyut, arsiv);
    org_bilgisi[org_boyut] = '\0';

    char *token = strtok(org_bilgisi, "|");
    while (token != NULL) {
        char ad[256];
        unsigned int izinler;
        long boyut;

        sscanf(token, "%[^,],%o,%ld", ad, &izinler, &boyut);

        char hedef_dosya[512];
        sprintf(hedef_dosya, "%s/%s", hedef_dizin, ad);

        FILE *cikis = fopen(hedef_dosya, "wb");
        if (cikis != NULL) {
            for (long i = 0; i < boyut; i++) {
                fputc(fgetc(arsiv), cikis);
            }
            fclose(cikis);
            chmod(hedef_dosya, izinler);
        }
        
        token = strtok(NULL, "|");
    }

    free(org_bilgisi);
    fclose(arsiv);

    printf("%s dizininde dosyalar acildi.\n", hedef_dizin); 
}