CC = gcc
CFLAGS = -Wall -Wextra -g

all: tarsau

tarsau: tarsau.c
	$(CC) $(CFLAGS) -o tarsau tarsau.c

clean:
	rm -f tarsau *.sau

# Projeyi hocaya teslim formatında sıkıştırmak için özel komut
# Kullanımı terminale: make zip (Daha sonra ogrenci_no kısımlarını güncelleriz)
zip: clean
	mkdir -p ogrenci_no1_ogrenci_no2
	cp tarsau.c Makefile rapor.pdf ogrenci_no1_ogrenci_no2/ 2>/dev/null || :
	zip -r ogrenci_no1_ogrenci_no2.zip ogrenci_no1_ogrenci_no2
	rm -rf ogrenci_no1_ogrenci_no2