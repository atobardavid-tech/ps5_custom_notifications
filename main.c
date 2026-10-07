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
 * Notification messages struct for errors
 * ============================================================
 */

typedef struct {
    const char *err_internal_only;
    const char *err_usb_and_internal;
} notification_strings_t;


/*
 * English (Fallback)
 */
static const notification_strings_t lang_en = {
    "Error: message.txt not found in /data/ps5_autoloader/",
    "Error: message.txt not found in /data/ps5_autoloader/ nor in USB"
};

/*
 * Spanish (Spain)
 */
static const notification_strings_t lang_es_es = {
    "Error: no se ha encontrado el archivo message.txt en /data/ps5_autoloader/",
    "Error: no se encontro el archivo message.txt en /data/ps5_autoloader/ ni en el USB"
};

/*
 * Spanish (Mexico)
 */
static const notification_strings_t lang_es_mx = {
    "Error: no se ha encontrado el archivo message.txt en /data/ps5_autoloader/",
    "Error: no se encontro el archivo message.txt en /data/ps5_autoloader/ ni en el USB"
};

/*
 * Japanese
 */
static const notification_strings_t lang_ja = {
    "エラー: /data/ps5_autoloader/ に message.txt が見つかりません",
    "エラー: /data/ps5_autoloader/ および USB に message.txt が見つかりません"
};

/*
 * French (France)
 */
static const notification_strings_t lang_fr_fr = {
    "Erreur : message.txt introuvable dans /data/ps5_autoloader/",
    "Erreur : message.txt introuvable dans /data/ps5_autoloader/ ni sur l'USB"
};

/*
 * French (Canada)
 */
static const notification_strings_t lang_fr_ca = {
    "Erreur : message.txt introuvable dans /data/ps5_autoloader/",
    "Erreur : message.txt introuvable dans /data/ps5_autoloader/ ni sur l'USB"
};

/*
 * German
 */
static const notification_strings_t lang_de = {
    "Fehler: message.txt wurde in /data/ps5_autoloader/ nicht gefunden",
    "Fehler: message.txt weder in /data/ps5_autoloader/ noch auf USB gefunden"
};

/*
 * Italian
 */
static const notification_strings_t lang_it = {
    "Errore: message.txt non trovato in /data/ps5_autoloader/",
    "Errore: message.txt non trovato né in /data/ps5_autoloader/ né su USB"
};

/*
 * Dutch
 */
static const notification_strings_t lang_nl = {
    "Fout: message.txt niet gevonden in /data/ps5_autoloader/",
    "Fout: message.txt niet gevonden in /data/ps5_autoloader/ noch op USB"
};

/*
 * Portuguese (Portugal)
 */
static const notification_strings_t lang_pt_pt = {
    "Erro: message.txt nao encontrado em /data/ps5_autoloader/",
    "Erro: message.txt nao encontrado em /data/ps5_autoloader/ nem no USB"
};

/*
 * Portuguese (Brazil)
 */
static const notification_strings_t lang_pt_br = {
    "Erro: message.txt nao encontrado em /data/ps5_autoloader/",
    "Erro: message.txt nao encontrado em /data/ps5_autoloader/ nem no USB"
};

/*
 * Russian
 */
static const notification_strings_t lang_ru = {
    "Ошибка: message.txt не найден в /data/ps5_autoloader/",
    "Ошибка: message.txt не найден ни в /data/ps5_autoloader/, ни на USB"
};

/*
 * Korean
 */
static const notification_strings_t lang_ko = {
    "오류: /data/ps5_autoloader/에서 message.txt를 찾을 수 없습니다",
    "오류: /data/ps5_autoloader/ 또는 USB에서 message.txt를 찾을 수 없습니다"
};

/*
 * Chinese (Traditional)
 */
static const notification_strings_t lang_zh_tw = {
    "錯誤：在 /data/ps5_autoloader/ 中找不到 message.txt",
    "錯誤：在 /data/ps5_autoloader/ 或 USB 中找不到 message.txt"
};

/*
 * Chinese (Simplified)
 */
static const notification_strings_t lang_zh_cn = {
    "错误：在 /data/ps5_autoloader/ 中找不到 message.txt",
    "错误：在 /data/ps5_autoloader/ 或 USB 中找不到 message.txt"
};

/*
 * Finnish
 */
static const notification_strings_t lang_fi = {
    "Virhe: tiedostoa message.txt ei loytynyt kansiosta /data/ps5_autoloader/",
    "Virhe: tiedostoa message.txt ei loytynyt kansiosta /data/ps5_autoloader/ eikä USB:lta"
};

/*
 * Swedish
 */
static const notification_strings_t lang_sv = {
    "Fel: message.txt hittades inte i /data/ps5_autoloader/",
    "Fel: message.txt hittades inte i /data/ps5_autoloader/ eller på USB"
};

/*
 * Danish
 */
static const notification_strings_t lang_da = {
    "Fejl: message.txt blev ikke fundet i /data/ps5_autoloader/",
    "Fejl: message.txt blev ikke fundet i /data/ps5_autoloader/ eller på USB"
};

/*
 * Norwegian
 */
static const notification_strings_t lang_no = {
    "Feil: message.txt ble ikke funnet i /data/ps5_autoloader/",
    "Feil: message.txt ble ikke funnet i /data/ps5_autoloader/ eller på USB"
};

/*
 * Polish
 */
static const notification_strings_t lang_pl = {
    "Blad: nie znaleziono pliku message.txt w /data/ps5_autoloader/",
    "Blad: nie znaleziono pliku message.txt w /data/ps5_autoloader/ ani na USB"
};

/*
 * English (United Kingdom)
 */
static const notification_strings_t lang_en_gb = {
    "Error: message.txt not found in /data/ps5_autoloader/",
    "Error: message.txt not found in /data/ps5_autoloader/ nor in USB"
};

/*
 * Turkish
 */
static const notification_strings_t lang_tr = {
    "Hata: message.txt dosyasi /data/ps5_autoloader/ icinde bulunamadi",
    "Hata: message.txt dosyasi ne /data/ps5_autoloader/ icinde ne de USB'de bulunamadi"
};

/*
 * Arabic
 */
static const notification_strings_t lang_ar = {
    "خطأ: لم يتم العثور على message.txt في /data/ps5_autoloader/",
    "خطأ: لم يتم العثور على message.txt في /data/ps5_autoloader/ ولا على USB"
};

/*
 * Czech
 */
static const notification_strings_t lang_cs = {
    "Chyba: soubor message.txt nebyl nalezen v /data/ps5_autoloader/",
    "Chyba: soubor message.txt nebyl nalezen v /data/ps5_autoloader/ ani na USB"
};

/*
 * Hungarian
  */
static const notification_strings_t lang_hu = {
    "Hiba: a message.txt nem talalhato a /data/ps5_autoloader/ mappaban",
    "Hiba: a message.txt nem talalhato sem a /data/ps5_autoloader/ mappaban, sem az USB-n"
};

/*
 * Greek
 */
static const notification_strings_t lang_el = {
    "Σφάλμα: το message.txt δεν βρέθηκε στο /data/ps5_autoloader/",
    "Σφάλμα: το message.txt δεν βρέθηκε ούτε στο /data/ps5_autoloader/ ούτε στο USB"
};

/*
 * Romanian
 */
static const notification_strings_t lang_ro = {
    "Eroare: message.txt nu a fost gasit în /data/ps5_autoloader/",
    "Eroare: message.txt nu a fost gasit în /data/ps5_autoloader/ și nici pe USB"
};

/*
 * Thai
 */
static const notification_strings_t lang_th = {
    "ข้อผิดพลาด: ไม่พบ message.txt ใน /data/ps5_autoloader/",
    "ข้อผิดพลาด: ไม่พบ message.txt ใน /data/ps5_autoloader/ และบน USB"
};

/*
 * Vietnamese
 */
static const notification_strings_t lang_vi = {
    "Lỗi: không tìm thấy message.txt trong /data/ps5_autoloader/",
    "Lỗi: không tìm thấy message.txt trong /data/ps5_autoloader/ cũng như trên USB"
};

/*
 * Indonesian
 */
static const notification_strings_t lang_id = {
    "Kesalahan: message.txt tidak ditemukan di /data/ps5_autoloader/",
    "Kesalahan: message.txt tidak ditemukan di /data/ps5_autoloader/ maupun di USB"
};

/*
 * Ukrainian
 */
static const notification_strings_t lang_uk = {
    "Помилка: message.txt не знайдено в /data/ps5_autoloader/",
    "Помилка: message.txt не знайдено ні в /data/ps5_autoloader/, ні на USB"
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
        snprintf(
            filepath,
            sizeof(filepath),
            "/mnt/usb%d/ps5_autoloader/message.txt",
            i
        );

        struct stat st;
        if (stat("/mnt/usb0", &st) == 0 || stat(filepath, &st) == 0) {
            // Nota de presencia de USB comprobada o directorio base de puerto disponible
        }

        // Validar si la partición USB o puerto al menos responde físicamente
        char usb_base[32];
        snprintf(usb_base, sizeof(usb_base), "/mnt/usb%d", i);
        if (stat(usb_base, &st) == 0 && S_ISDIR(st.st_mode)) {
            usb_connected_flag = 1;
        }

        file = fopen(filepath, "r");
        if (file) {
            // Se encontró el archivo en la USB, detenemos búsqueda aquí
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
     * 4. Read file line by line (Max 5 lines, max 80 chars per line with '...')
     */
    int lines_shown = 0;

    while (fgets(line_buffer, sizeof(line_buffer), file) != NULL && lines_shown < 5) {
        
        // Remover salto de línea al final si existe
        size_t len = strlen(line_buffer);
        if (len > 0 && (line_buffer[len - 1] == '\n' || line_buffer[len - 1] == '\r')) {
            line_buffer[--len] = '\0';
        }

        // Omitir líneas en blanco
        if (len == 0) {
            continue;
        }

        // Truncar a un máximo de 80 caracteres y colocar puntos suspensivos si excede
        char final_line[96];
        if (len > 80) {
            snprintf(final_line, sizeof(final_line), "%.77s...", line_buffer);
        } else {
            snprintf(final_line, sizeof(final_line), "%s", line_buffer);
        }

        // Mostrar como notificación en pantalla
        show_notification(final_line);
        lines_shown++;
    }

    fclose(file);

    return 0;
}