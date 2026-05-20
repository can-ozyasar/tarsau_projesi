/*
 * BSM301 Sistem Programlama Projesi
 * tarsau - sikistirma yapmadan metin dosyasi arsivleme araci
 */

#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define DEFAULT_ARCHIVE "a.sau"
#define HEADER_SIZE 10
#define MAX_FILES 32
#define MAX_TOTAL_SIZE (200LL * 1024LL * 1024LL)
#define COPY_BUFFER_SIZE 8192

typedef struct {
    char archive_name[PATH_MAX];
    const char *source_path;
    mode_t permissions;
    long long size;
} FileInfo;

static void print_usage(const char *program_name);
static int build_archive(int file_count, char *files[], const char *archive_name);
static int extract_archive(const char *archive_name, const char *target_dir);
static int validate_input_file(const char *path, struct stat *file_stat);
static bool has_sau_extension(const char *path);
static bool is_safe_archive_name(const char *name);
static bool normalize_archive_name(const char *input, char *output, size_t output_size);
static bool ensure_directory(const char *path, mode_t mode);
static bool ensure_parent_directories(const char *path);
static int copy_exact_bytes(FILE *input, FILE *output, long long byte_count);
static int copy_file(FILE *input, FILE *output);
static int parse_nonnegative_ll(const char *text, long long *value);
static int parse_permissions(const char *text, unsigned int *permissions);

int main(int argc, char *argv[])
{
    if (argc < 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "-b") == 0) {
        char *files[MAX_FILES];
        int file_count = 0;
        const char *archive_name = DEFAULT_ARCHIVE;
        bool output_seen = false;

        for (int i = 2; i < argc; ++i) {
            if (strcmp(argv[i], "-o") == 0) {
                if (output_seen || i + 1 >= argc || i + 2 != argc) {
                    print_usage(argv[0]);
                    return EXIT_FAILURE;
                }
                output_seen = true;
                archive_name = argv[++i];
            } else {
                if (output_seen || file_count >= MAX_FILES) {
                    print_usage(argv[0]);
                    return EXIT_FAILURE;
                }
                files[file_count++] = argv[i];
            }
        }

        if (file_count == 0) {
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }

        return build_archive(file_count, files, archive_name);
    }

    if (strcmp(argv[1], "-a") == 0) {
        if (argc < 3 || argc > 4) {
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }

        return extract_archive(argv[2], argc == 4 ? argv[3] : ".");
    }

    print_usage(argv[0]);
    return EXIT_FAILURE;
}

static void print_usage(const char *program_name)
{
    fprintf(stderr, "Kullanim: %s -b dosya1 [dosya2 ...] [-o arsiv.sau]\n", program_name);
    fprintf(stderr, "         %s -a arsiv.sau [hedef_dizin]\n", program_name);
}

static int build_archive(int file_count, char *files[], const char *archive_name)
{
    FileInfo file_infos[MAX_FILES];
    long long total_size = 0;
    size_t index_size = 0;

    if (!has_sau_extension(archive_name)) {
        fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < file_count; ++i) {
        struct stat file_stat;

        if (!validate_input_file(files[i], &file_stat)) {
            fprintf(stderr, "%s giris dosyasinin formati uyumsuzdur!\n", files[i]);
            return EXIT_SUCCESS;
        }

        if (!normalize_archive_name(files[i], file_infos[i].archive_name,
                                    sizeof(file_infos[i].archive_name))) {
            fprintf(stderr, "%s giris dosyasinin formati uyumsuzdur!\n", files[i]);
            return EXIT_SUCCESS;
        }

        for (int j = 0; j < i; ++j) {
            if (strcmp(file_infos[j].archive_name, file_infos[i].archive_name) == 0) {
                fprintf(stderr, "Hata: Arsiv icinde tekrar eden dosya adi: %s\n",
                        file_infos[i].archive_name);
                return EXIT_FAILURE;
            }
        }

        file_infos[i].source_path = files[i];
        file_infos[i].permissions = file_stat.st_mode & 0777;
        file_infos[i].size = (long long)file_stat.st_size;
        total_size += file_infos[i].size;

        if (total_size > MAX_TOTAL_SIZE) {
            fprintf(stderr, "Hata: Giris dosyalarinin toplam boyutu 200 MB'i gecemez!\n");
            return EXIT_FAILURE;
        }

        index_size += (size_t)snprintf(NULL, 0, "|%s,%03o,%lld|",
                                       file_infos[i].archive_name,
                                       (unsigned int)file_infos[i].permissions,
                                       file_infos[i].size);
    }

    if (index_size > 9999999999ULL) {
        fprintf(stderr, "Hata: Arsiv organizasyon bolumu cok buyuk.\n");
        return EXIT_FAILURE;
    }

    FILE *archive = fopen(archive_name, "wb");
    if (archive == NULL) {
        fprintf(stderr, "Hata: Arsiv dosyasi olusturulamadi: %s\n", archive_name);
        return EXIT_FAILURE;
    }

    fprintf(archive, "%010zu", index_size);

    for (int i = 0; i < file_count; ++i) {
        if (fprintf(archive, "|%s,%03o,%lld|",
                    file_infos[i].archive_name,
                    (unsigned int)file_infos[i].permissions,
                    file_infos[i].size) < 0) {
            fprintf(stderr, "Hata: Arsiv organizasyon bolumu yazilamadi.\n");
            fclose(archive);
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < file_count; ++i) {
        FILE *input = fopen(file_infos[i].source_path, "rb");
        if (input == NULL) {
            fprintf(stderr, "Hata: Dosya okunamadi: %s\n", file_infos[i].source_path);
            fclose(archive);
            return EXIT_FAILURE;
        }

        if (copy_file(input, archive) != 0) {
            fprintf(stderr, "Hata: Dosya arsive yazilamadi: %s\n", file_infos[i].source_path);
            fclose(input);
            fclose(archive);
            return EXIT_FAILURE;
        }

        fclose(input);
    }

    if (fclose(archive) != 0) {
        fprintf(stderr, "Hata: Arsiv dosyasi kapatilamadi: %s\n", archive_name);
        return EXIT_FAILURE;
    }

    printf("%s arsivi olusturuldu.\n", archive_name);
    return EXIT_SUCCESS;
}

static int extract_archive(const char *archive_name, const char *target_dir)
{
    char header[HEADER_SIZE + 1];
    char *index_data = NULL;
    int status = EXIT_FAILURE;

    if (!has_sau_extension(archive_name)) {
        fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
        return EXIT_FAILURE;
    }

    FILE *archive = fopen(archive_name, "rb");
    if (archive == NULL) {
        fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
        return EXIT_FAILURE;
    }

    if (fread(header, 1, HEADER_SIZE, archive) != HEADER_SIZE) {
        fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
        goto cleanup;
    }
    header[HEADER_SIZE] = '\0';

    for (int i = 0; i < HEADER_SIZE; ++i) {
        if (!isdigit((unsigned char)header[i]) && header[i] != ' ') {
            fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
            goto cleanup;
        }
    }

    long long index_size_ll;
    if (parse_nonnegative_ll(header, &index_size_ll) != 0 || index_size_ll <= 0 ||
        index_size_ll > MAX_TOTAL_SIZE) {
        fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
        goto cleanup;
    }

    index_data = malloc((size_t)index_size_ll + 1);
    if (index_data == NULL) {
        fprintf(stderr, "Hata: Bellek ayrilamadi.\n");
        goto cleanup;
    }

    if (fread(index_data, 1, (size_t)index_size_ll, archive) != (size_t)index_size_ll) {
        fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
        goto cleanup;
    }
    index_data[index_size_ll] = '\0';

    if (!ensure_directory(target_dir, 0777)) {
        fprintf(stderr, "Hata: Hedef dizin olusturulamadi: %s\n", target_dir);
        goto cleanup;
    }

    char *cursor = index_data;
    int extracted_count = 0;

    while (*cursor != '\0') {
        if (*cursor != '|') {
            fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
            goto cleanup;
        }

        char *record_start = ++cursor;
        char *record_end = strchr(record_start, '|');
        if (record_end == NULL || record_end == record_start) {
            fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
            goto cleanup;
        }
        *record_end = '\0';

        char *name = strtok(record_start, ",");
        char *permissions_text = strtok(NULL, ",");
        char *size_text = strtok(NULL, ",");
        char *extra = strtok(NULL, ",");
        unsigned int permissions;
        long long size;

        if (name == NULL || permissions_text == NULL || size_text == NULL || extra != NULL ||
            !is_safe_archive_name(name) ||
            parse_permissions(permissions_text, &permissions) != 0 ||
            parse_nonnegative_ll(size_text, &size) != 0) {
            fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
            goto cleanup;
        }

        char output_path[PATH_MAX];
        int written = snprintf(output_path, sizeof(output_path), "%s/%s", target_dir, name);
        if (written < 0 || (size_t)written >= sizeof(output_path)) {
            fprintf(stderr, "Hata: Dosya yolu cok uzun: %s\n", name);
            goto cleanup;
        }

        if (!ensure_parent_directories(output_path)) {
            fprintf(stderr, "Hata: Hedef alt dizin olusturulamadi: %s\n", output_path);
            goto cleanup;
        }

        FILE *output = fopen(output_path, "wb");
        if (output == NULL) {
            fprintf(stderr, "Hata: Dosya olusturulamadi: %s\n", output_path);
            goto cleanup;
        }

        if (copy_exact_bytes(archive, output, size) != 0) {
            fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
            fclose(output);
            goto cleanup;
        }

        if (fclose(output) != 0 || chmod(output_path, (mode_t)permissions) != 0) {
            fprintf(stderr, "Hata: Dosya izinleri ayarlanamadi: %s\n", output_path);
            goto cleanup;
        }

        ++extracted_count;
        cursor = record_end + 1;
    }

    if (extracted_count == 0) {
        fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
        goto cleanup;
    }

    printf("%s dizinine %d dosya acildi.\n", target_dir, extracted_count);
    status = EXIT_SUCCESS;

cleanup:
    free(index_data);
    fclose(archive);
    return status;
}

static int validate_input_file(const char *path, struct stat *file_stat)
{
    if (stat(path, file_stat) != 0 || !S_ISREG(file_stat->st_mode)) {
        return 0;
    }

    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return 0;
    }

    unsigned char buffer[COPY_BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        for (size_t i = 0; i < bytes_read; ++i) {
            if (buffer[i] > 127) {
                fclose(file);
                return 0;
            }
        }
    }

    if (ferror(file)) {
        fclose(file);
        return 0;
    }

    fclose(file);
    return 1;
}

static bool has_sau_extension(const char *path)
{
    size_t length = strlen(path);

    return length > 4 && strcmp(path + length - 4, ".sau") == 0;
}

static bool normalize_archive_name(const char *input, char *output, size_t output_size)
{
    while (strncmp(input, "./", 2) == 0) {
        input += 2;
    }

    if (!is_safe_archive_name(input) || strlen(input) >= output_size) {
        return false;
    }

    strcpy(output, input);
    return true;
}

static bool is_safe_archive_name(const char *name)
{
    if (name == NULL || name[0] == '\0' || name[0] == '/' || strstr(name, "\\") != NULL ||
        strstr(name, "..") != NULL || strchr(name, ',') != NULL || strchr(name, '|') != NULL) {
        return false;
    }

    return true;
}

static bool ensure_directory(const char *path, mode_t mode)
{
    char buffer[PATH_MAX];
    size_t length;

    if (path == NULL || path[0] == '\0') {
        return false;
    }

    if (snprintf(buffer, sizeof(buffer), "%s", path) >= (int)sizeof(buffer)) {
        return false;
    }

    length = strlen(buffer);
    while (length > 1 && buffer[length - 1] == '/') {
        buffer[--length] = '\0';
    }

    for (char *p = buffer + 1; *p != '\0'; ++p) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(buffer, mode) != 0 && errno != EEXIST) {
                return false;
            }
            *p = '/';
        }
    }

    if (mkdir(buffer, mode) != 0 && errno != EEXIST) {
        return false;
    }

    struct stat st;
    return stat(buffer, &st) == 0 && S_ISDIR(st.st_mode);
}

static bool ensure_parent_directories(const char *path)
{
    char parent[PATH_MAX];
    char *slash;

    if (snprintf(parent, sizeof(parent), "%s", path) >= (int)sizeof(parent)) {
        return false;
    }

    slash = strrchr(parent, '/');
    if (slash == NULL) {
        return true;
    }

    if (slash == parent) {
        slash[1] = '\0';
    } else {
        *slash = '\0';
    }

    return ensure_directory(parent, 0777);
}

static int copy_exact_bytes(FILE *input, FILE *output, long long byte_count)
{
    unsigned char buffer[COPY_BUFFER_SIZE];

    while (byte_count > 0) {
        size_t chunk = byte_count > (long long)sizeof(buffer) ? sizeof(buffer) : (size_t)byte_count;
        size_t bytes_read = fread(buffer, 1, chunk, input);

        if (bytes_read != chunk) {
            return -1;
        }

        if (fwrite(buffer, 1, bytes_read, output) != bytes_read) {
            return -1;
        }

        byte_count -= (long long)bytes_read;
    }

    return 0;
}

static int copy_file(FILE *input, FILE *output)
{
    unsigned char buffer[COPY_BUFFER_SIZE];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), input)) > 0) {
        if (fwrite(buffer, 1, bytes_read, output) != bytes_read) {
            return -1;
        }
    }

    return ferror(input) ? -1 : 0;
}

static int parse_nonnegative_ll(const char *text, long long *value)
{
    char *endptr;
    long long parsed;

    errno = 0;
    parsed = strtoll(text, &endptr, 10);
    if (errno != 0 || parsed < 0) {
        return -1;
    }

    while (*endptr != '\0') {
        if (!isspace((unsigned char)*endptr)) {
            return -1;
        }
        ++endptr;
    }

    *value = parsed;
    return 0;
}

static int parse_permissions(const char *text, unsigned int *permissions)
{
    unsigned int parsed = 0;

    if (text == NULL || *text == '\0') {
        return -1;
    }

    for (const char *p = text; *p != '\0'; ++p) {
        if (*p < '0' || *p > '7') {
            return -1;
        }
        parsed = parsed * 8U + (unsigned int)(*p - '0');
        if (parsed > 0777U) {
            return -1;
        }
    }

    *permissions = parsed;
    return 0;
}
