/* Copyright (C) 2023 John Törnblom

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING. If not, see
<http://www.gnu.org/licenses/>.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>

/*
 * ============================================================
 * PS5 Notification
 * ============================================================
 */

typedef struct notify_request {
    char useless1[45];
    char message[3075];
} notify_request_t;

int sceKernelSendNotificationRequest(
    int,
    notify_request_t *,
    size_t,
    int
);

/*
 * ============================================================
 * PS5 System Language
 * ============================================================
 */

int sceSystemServiceParamGetInt(int param_id, int32_t *value);

#define SCE_SYSTEM_SERVICE_PARAM_ID_LANG 1

typedef enum {
    LANG_JA_JP = 0,
    LANG_EN_US,
    LANG_FR_FR,
    LANG_ES_ES,
    LANG_DE_DE,
    LANG_IT_IT,
    LANG_NL_NL,
    LANG_PT_PT,
    LANG_RU_RU,
    LANG_KO_KR,
    LANG_ZH_TW,
    LANG_ZH_CN,
    LANG_FI_FI,
    LANG_SV_SE,
    LANG_DA_DK,
    LANG_NO_NO,
    LANG_PL_PL,
    LANG_PT_BR,
    LANG_EN_GB,
    LANG_TR_TR,
    LANG_ES_MX,
    LANG_AR_SA,
    LANG_FR_CA,
    LANG_CS_CZ,
    LANG_HU_HU,
    LANG_EL_GR,
    LANG_RO_RO,
    LANG_TH_TH,
    LANG_VI_VN,
    LANG_ID_ID,
    LANG_UK_UA
} language_id_t;


/*
 * ============================================================
 * Notification messages struct for errors and structure validation
 * ============================================================
 */

typedef struct {
    const char *err_internal_only;
    const char *err_usb_and_internal;
    const char *err_bad_structure;
    const char *err_empty_lines;
} notification_strings_t;


/*
 * English (Fallback)
 */
static const notification_strings_t lang_en = {
    "Error: message.txt not found in /data/ps5_autoloader/",
    "Error: message.txt not found in /data/ps5_autoloader/ nor in USB paths",
    "Error: Invalid structure expected in message.txt",
    "Error: text lines are empty in message.txt"
};

/*
 * Spanish (Spain)
 */
static const notification_strings_t lang_es_es = {
    "Error: no se ha encontrado el archivo message.txt en /data/ps5_autoloader/",
    "Error: no se encontro message.txt en /data/ps5_autoloader/ ni en las rutas USB",
    "Error: estructura esperada no encontrada en message.txt",
    "Error: en message.txt las lineas de texto estan vacias"
};

/*
 * Spanish (Mexico)
 */
static const notification_strings_t lang_es_mx = {
    "Error: no se ha encontrado el archivo message.txt en /data/ps5_autoloader/",
    "Error: no se encontro message.txt en /data/ps5_autoloader/ ni en las rutas USB",
    "Error: estructura esperada no encontrada en message.txt",
    "Error: en message.txt las lineas de texto estan vacias"
};

/*
 * Japanese
 */
static const notification_strings_t lang_ja = {
    "エラー: /data/ps5_autoloader/ に message.txt が見つかりません",
    "エラー: /data/ps5_autoloader/ および USB パスに message.txt が見つかりません",
    "エラー: message.txt の構造が無効です",
    "エラー: message.txt のテキスト行が空です"
};

/*
 * French (France)
 */
static const notification_strings_t lang_fr_fr = {
    "Erreur : message.txt introuvable dans /data/ps5_autoloader/",
    "Erreur : message.txt introuvable dans /data/ps5_autoloader/ ni sur les chemins USB",
    "Erreur : structure attendue invalide dans message.txt",
    "Erreur : les lignes de texte sont vides dans message.txt"
};

/*
 * French (Canada)
 */
static const notification_strings_t lang_fr_ca = {
    "Erreur : message.txt introuvable dans /data/ps5_autoloader/",
    "Erreur : message.txt introuvable dans /data/ps5_autoloader/ ni sur les chemins USB",
    "Erreur : structure attendue invalide dans message.txt",
    "Erreur : les lignes de texte sont vides dans message.txt"
};

/*
 * German
 */
static const notification_strings_t lang_de = {
    "Fehler: message.txt wurde in /data/ps5_autoloader/ nicht gefunden",
    "Fehler: message.txt weder in /data/ps5_autoloader/ noch in USB-Pfaden gefunden",
    "Fehler: Ungültige Struktur in message.txt erwartet",
    "Fehler: Textzeilen sind in message.txt leer"
};

/*
 * Italian
 */
static const notification_strings_t lang_it = {
    "Errore: message.txt non trovato in /data/ps5_autoloader/",
    "Errore: message.txt non trovato né in /data/ps5_autoloader/ né nei percorsi USB",
    "Errore: struttura attesa non valida in message.txt",
    "Errore: le righe di testo sono vuote in message.txt"
};

/*
 * Dutch
 */
static const notification_strings_t lang_nl = {
    "Fout: message.txt niet gevonden in /data/ps5_autoloader/",
    "Fout: message.txt niet gevonden in /data/ps5_autoloader/ noch in USB-paden",
    "Fout: Ongeldige structuur verwacht in message.txt",
    "Fout: tekstregels zijn leeg in message.txt"
};

/*
 * Portuguese (Portugal)
 */
static const notification_strings_t lang_pt_pt = {
    "Erro: message.txt nao encontrado em /data/ps5_autoloader/",
    "Erro: message.txt nao encontrado em /data/ps5_autoloader/ nem nos caminhos USB",
    "Erro: Estrutura esperada invalida em message.txt",
    "Erro: as linhas de texto estao vazias em message.txt"
};

/*
 * Portuguese (Brazil)
 */
static const notification_strings_t lang_pt_br = {
    "Erro: message.txt nao encontrado em /data/ps5_autoloader/",
    "Erro: message.txt nao encontrado em /data/ps5_autoloader/ nem nos caminhos USB",
    "Erro: Estrutura esperada invalida em message.txt",
    "Erro: as linhas de texto estao vazias em message.txt"
};

/*
 * Russian
 */
static const notification_strings_t lang_ru = {
    "Ошибка: message.txt не найден в /data/ps5_autoloader/",
    "Ошибка: message.txt не найден ни в /data/ps5_autoloader/, ни на путях USB",
    "Ошибка: неверная структура в message.txt",
    "Ошибка: строки текста пусты в message.txt"
};

/*
 * Korean
 */
static const notification_strings_t lang_ko = {
    "오류: /data/ps5_autoloader/에서 message.txt를 찾을 수 없습니다",
    "오류: /data/ps5_autoloader/ 또는 USB 경로에서 message.txt를 찾을 수 없습니다",
    "오류: message.txt의 구조가 잘못되었습니다",
    "오류: message.txt의 텍스트 줄이 비어 있습니다"
};

/*
 * Chinese (Traditional)
 */
static const notification_strings_t lang_zh_tw = {
    "錯誤：在 /data/ps5_autoloader/ 中找不到 message.txt",
    "錯誤：在 /data/ps5_autoloader/ 或 USB 路徑中找不到 message.txt",
    "錯誤：message.txt 結構无效",
    "錯誤：message.txt 中的文字行是空的"
};

/*
 * Chinese (Simplified)
 */
static const notification_strings_t lang_zh_cn = {
    "错误：在 /data/ps5_autoloader/ 中找不到 message.txt",
    "错误：在 /data/ps5_autoloader/ 或 USB 路径中找不到 message.txt",
    "错误：message.txt 结构无效",
    "错误：message.txt 中的文本行为空"
};

/*
 * Finnish
 */
static const notification_strings_t lang_fi = {
    "Virhe: tiedostoa message.txt ei loytynyt kansiosta /data/ps5_autoloader/",
    "Virhe: tiedostoa message.txt ei loytynyt kansiosta /data/ps5_autoloader/ eikä USB-poluista",
    "Virhe: virheellinen rakenne tiedostossa message.txt",
    "Virhe: tekstirivit ovat tyhjiä tiedostossa message.txt"
};

/*
 * Swedish
 */
static const notification_strings_t lang_sv = {
    "Fel: message.txt hittades inte i /data/ps5_autoloader/",
    "Fel: message.txt hittades inte i /data/ps5_autoloader/ eller på USB-sökvägar",
    "Fel: Ogiltig struktur förväntad i message.txt",
    "Fel: textrader är tomma i message.txt"
};

/*
 * Danish
 */
static const notification_strings_t lang_da = {
    "Fejl: message.txt blev ikke fundet i /data/ps5_autoloader/",
    "Fejl: message.txt blev ikke fundet i /data/ps5_autoloader/ eller på USB-stier",
    "Fejl: Ugyldig struktur i message.txt",
    "Fejl: tekstlinjer er tomme i message.txt"
};

/*
 * Norwegian
 */
static const notification_strings_t lang_no = {
    "Feil: message.txt ble ikke funnet i /data/ps5_autoloader/",
    "Feil: message.txt ble ikke funnet i /data/ps5_autoloader/ eller på USB-baner",
    "Feil: Ugyldig struktur i message.txt",
    "Feil: tekstlinjer er tomme i message.txt"
};

/*
 * Polish
 */
static const notification_strings_t lang_pl = {
    "Blad: nie znaleziono pliku message.txt w /data/ps5_autoloader/",
    "Blad: nie znaleziono message.txt w /data/ps5_autoloader/ ani w ścieżkach USB",
    "Blad: Nieprawidłowa struktura pliku message.txt",
    "Blad: linie tekstu są puste w message.txt"
};

/*
 * English (United Kingdom)
 */
static const notification_strings_t lang_en_gb = {
    "Error: message.txt not found in /data/ps5_autoloader/",
    "Error: message.txt not found in /data/ps5_autoloader/ nor in USB paths",
    "Error: Invalid structure expected in message.txt",
    "Error: text lines are empty in message.txt"
};

/*
 * Turkish
 */
static const notification_strings_t lang_tr = {
    "Hata: message.txt dosyasi /data/ps5_autoloader/ icinde bulunamadi",
    "Hata: message.txt /data/ps5_autoloader/ icinde veya USB yollarinda bulunamadi",
    "Hata: message.txt içinde geçersiz yapı",
    "Hata: message.txt içinde metin satırları boş"
};

/*
 * Arabic
 */
static const notification_strings_t lang_ar = {
    "خطأ: لم يتم العثور على message.txt في /data/ps5_autoloader/",
    "خطأ: لم يتم العثور على message.txt في /data/ps5_autoloader/ ولا في مسارات USB",
    "خطأ: الهيكل المطلوب غير صالح في message.txt",
    "خطأ: أسطر النص فارغة في message.txt"
};

/*
 * Czech
 */
static const notification_strings_t lang_cs = {
    "Chyba: soubor message.txt nebyl nalezen v /data/ps5_autoloader/",
    "Chyba: soubor message.txt nebyl nalezen v /data/ps5_autoloader/ ani v USB cestách",
    "Chyba: Očekávaná struktura v message.txt je neplatná",
    "Chyba: řádky textu jsou prázdné v message.txt"
};

/*
 * Hungarian
 */
static const notification_strings_t lang_hu = {
    "Hiba: a message.txt nem talalhato a /data/ps5_autoloader/ mappaban",
    "Hiba: a message.txt nem talalhato sem a /data/ps5_autoloader/ mappaban, sem az USB útvonalakon",
    "Hiba: Érvénytelen szerkezet a message.txt fájlban",
    "Hiba: a szöveges sorok üresek a message.txt fájlban"
};

/*
 * Greek
 */
static const notification_strings_t lang_el = {
    "Σφάλμα: το message.txt δεν βρέθηκε στο /data/ps5_autoloader/",
    "Σφάλμα: το message.txt δεν βρέθηκε ούτε στο /data/ps5_autoloader/ ούτε στις διαδρομές USB",
    "Σφάλμα: Μη έγκυρη δομή στο message.txt",
    "Σφάλμα: οι γραμμές κειμένου είναι κενές στο message.txt"
};

/*
 * Romanian
 */
static const notification_strings_t lang_ro = {
    "Eroare: message.txt nu a fost gasit în /data/ps5_autoloader/",
    "Eroare: message.txt nu a fost gasit în /data/ps5_autoloader/ și nici în căile USB",
    "Eroare: Structură invalidă în message.txt",
    "Eroare: liniile de text sunt goale în message.txt"
};

/*
 * Thai
 */
static const notification_strings_t lang_th = {
    "ข้อผิดพลาด: ไม่พบ message.txt ใน /data/ps5_autoloader/",
    "ข้อผิดพลาด: ไม่พบ message.txt ใน /data/ps5_autoloader/ และในเส้นทาง USB",
    "ข้อผิดพลาด: โครงสร้างใน message.txt ไม่ถูกต้อง",
    "ข้อผิดพลาด: บรรทัดข้อความว่างเปล่าใน message.txt"
};

/*
 * Vietnamese
 */
static const notification_strings_t lang_vi = {
    "Lỗi: không tìm thấy message.txt trong /data/ps5_autoloader/",
    "Lỗi: không tìm thấy message.txt trong /data/ps5_autoloader/ cũng như trong các đường dẫn USB",
    "Lỗi: Cấu trúc tệp message.txt không hợp lệ",
    "Lỗi: các dòng văn bản trống trong message.txt"
};

/*
 * Indonesian
 */
static const notification_strings_t lang_id = {
    "Kesalahan: message.txt tidak ditemukan di /data/ps5_autoloader/",
    "Kesalahan: message.txt tidak ditemukan di /data/ps5_autoloader/ maupun di jalur USB",
    "Kesalahan: Struktur yang diharapkan tidak valid di message.txt",
    "Kesalahan: baris teks kosong di message.txt"
};

/*
 * Ukrainian
 */
static const notification_strings_t lang_uk = {
    "Помилка: message.txt не знайдено в /data/ps5_autoloader/",
    "Помилка: message.txt не знайдено ні в /data/ps5_autoloader/, ні на шляхах USB",
    "Помилка: Неочікувана структура в message.txt",
    "Помилка: рядки тексту порожні в message.txt"
};


/*
 * ============================================================
 * Get notification language
 * ============================================================
 */

const notification_strings_t *get_language_strings(void)
{
    int32_t language_id = -1;

    int ret = sceSystemServiceParamGetInt(
        SCE_SYSTEM_SERVICE_PARAM_ID_LANG,
        &language_id
    );

    if (ret != 0) {
        return &lang_en;
    }

    switch (language_id) {
        case LANG_JA_JP: return &lang_ja;
        case LANG_EN_US: return &lang_en;
        case LANG_FR_FR: return &lang_fr_fr;
        case LANG_ES_ES: return &lang_es_es;
        case LANG_DE_DE: return &lang_de;
        case LANG_IT_IT: return &lang_it;
        case LANG_NL_NL: return &lang_nl;
        case LANG_PT_PT: return &lang_pt_pt;
        case LANG_RU_RU: return &lang_ru;
        case LANG_KO_KR: return &lang_ko;
        case LANG_ZH_TW: return &lang_zh_tw;
        case LANG_ZH_CN: return &lang_zh_cn;
        case LANG_FI_FI: return &lang_fi;
        case LANG_SV_SE: return &lang_sv;
        case LANG_DA_DK: return &lang_da;
        case LANG_NO_NO: return &lang_no;
        case LANG_PL_PL: return &lang_pl;
        case LANG_PT_BR: return &lang_pt_br;
        case LANG_EN_GB: return &lang_en_gb;
        case LANG_TR_TR: return &lang_tr;
        case LANG_ES_MX: return &lang_es_mx;
        case LANG_AR_SA: return &lang_ar;
        case LANG_FR_CA: return &lang_fr_ca;
        case LANG_CS_CZ: return &lang_cs;
        case LANG_HU_HU: return &lang_hu;
        case LANG_EL_GR: return &lang_el;
        case LANG_RO_RO: return &lang_ro;
        case LANG_TH_TH: return &lang_th;
        case LANG_VI_VN: return &lang_vi;
        case LANG_ID_ID: return &lang_id;
        case LANG_UK_UA: return &lang_uk;
        default: return &lang_en;
    }
}


/*
 * ============================================================
 * Show notification
 * ============================================================
 */

void show_notification(const char *msg)
{
    notify_request_t req;

    bzero(&req, sizeof(req));

    strncpy(
        req.message,
        msg,
        sizeof(req.message) - 1
    );

    sceKernelSendNotificationRequest(
        0,
        &req,
        sizeof(req),
        0
    );
}


/*
 * ============================================================
 * Main
 * ============================================================
 */

int main(int argc, char *argv[])
{
    char filepath[128];
    char line_buffer[256];
    FILE *file = NULL;
    int usb_connected_flag = 0;

    /*
     * Detect the PS5 system language at startup.
     */
    const notification_strings_t *msg = get_language_strings();

    /*
     * 1. Search USB mount points from usb0 to usb7 for message.txt
     */
    for (int i = 0; i <= 7; i++) {
        char usb_base[32];
        struct stat st;

        snprintf(usb_base, sizeof(usb_base), "/mnt/usb%d", i);
        if (stat(usb_base, &st) == 0 && S_ISDIR(st.st_mode)) {
            usb_connected_flag = 1;
        }

        snprintf(
            filepath,
            sizeof(filepath),
            "/mnt/usb%d/ps5_autoloader/message.txt",
            i
        );

        file = fopen(filepath, "r");
        if (file) {
            break;
        }
    }

    /*
     * 2. Fallback to internal storage /data/ps5_autoloader/message.txt if not found on USB
     */
    if (!file) {
        snprintf(
            filepath,
            sizeof(filepath),
            "/data/ps5_autoloader/message.txt"
        );
        
        file = fopen(filepath, "r");
    }

    /*
     * 3. Error Handling if file was not found anywhere
     */
    if (!file) {
        if (!usb_connected_flag) {
            show_notification(msg->err_internal_only);
        } else {
            show_notification(msg->err_usb_and_internal);
        }
        return -1;
    }

    /*
     * 4. Read and validate configuration parameters from the first lines
     */
    int max_lines = 3;
    int chars_per_line = 90;
    int delay_ms = 300;

    // Leer línea 1: LINES=X
    if (fgets(line_buffer, sizeof(line_buffer), file) == NULL || strncmp(line_buffer, "LINES=", 6) != 0) {
        show_notification(msg->err_bad_structure);
        fclose(file);
        return -1;
    }
    max_lines = atoi(line_buffer + 6);

    // Leer línea 2: CHARS=X
    if (fgets(line_buffer, sizeof(line_buffer), file) == NULL || strncmp(line_buffer, "CHARS=", 6) != 0) {
        show_notification(msg->err_bad_structure);
        fclose(file);
        return -1;
    }
    chars_per_line = atoi(line_buffer + 6);

    // Leer línea 3: DELAY=X
    if (fgets(line_buffer, sizeof(line_buffer), file) == NULL || strncmp(line_buffer, "DELAY=", 6) != 0) {
        show_notification(msg->err_bad_structure);
        fclose(file);
        return -1;
    }
    delay_ms = atoi(line_buffer + 6);

    // Leer línea 4: --- (separador)
    if (fgets(line_buffer, sizeof(line_buffer), file) == NULL || strncmp(line_buffer, "---", 3) != 0) {
        show_notification(msg->err_bad_structure);
        fclose(file);
        return -1;
    }

    // Límites de seguridad para evitar desbordamientos
    if (max_lines < 1) max_lines = 1;
    if (max_lines > 10) max_lines = 10;
    if (chars_per_line < 10) chars_per_line = 10;
    if (chars_per_line > 200) chars_per_line = 200;

    /*
     * 5. Read message lines starting from line 5 onwards and validate they are not empty
     */
    int lines_shown = 0;
    int valid_lines_count = 0;

    // Guardar posición actual para hacer una verificación previa de contenido no vacío
    long pos = ftell(file);
    while (fgets(line_buffer, sizeof(line_buffer), file) != NULL) {
        size_t len = strlen(line_buffer);
        if (len > 0 && (line_buffer[len - 1] == '\n' || line_buffer[len - 1] == '\r')) {
            line_buffer[--len] = '\0';
        }
        // Verificar si la línea tiene al menos un caracter imprimible/válido
        for (size_t i = 0; i < len; i++) {
            if (line_buffer[i] != ' ' && line_buffer[i] != '\t') {
                valid_lines_count++;
                break;
            }
        }
    }

    if (valid_lines_count == 0) {
        show_notification(msg->err_empty_lines);
        fclose(file);
        return -1;
    }

    // Regresar al inicio del bloque de texto
    fseek(file, pos, SEEK_SET);

    while (fgets(line_buffer, sizeof(line_buffer), file) != NULL && lines_shown < max_lines) {
        
        size_t len = strlen(line_buffer);
        if (len > 0 && (line_buffer[len - 1] == '\n' || line_buffer[len - 1] == '\r')) {
            line_buffer[--len] = '\0';
        }

        if (len == 0) {
            continue;
        }

        char final_line[256];
        if (len > chars_per_line) {
            snprintf(final_line, sizeof(final_line), "%.*s...", chars_per_line - 3, line_buffer);
        } else {
            snprintf(final_line, sizeof(final_line), "%s", line_buffer);
        }

        show_notification(final_line);
        
        usleep(delay_ms * 1000);

        lines_shown++;
    }

    fclose(file);

    return 0;
}