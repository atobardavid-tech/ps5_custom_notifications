/* Copyright (C) 2023 John Törnblom
   Modificado: Precarga automática de parámetros y textos desde message.txt al abrir la interfaz - 2026

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
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdint.h>

#define SERVER_PORT 8080
#define TARGET_DIR "/data/ps5_autoloader"
#define TARGET_FILE TARGET_DIR "/message.txt"

int sceUserServiceInitialize(void*);
int sceUserServiceTerminate(void);
int sceSystemServiceLaunchWebBrowser(const char *uri, void*);

/*
 * ============================================================
 * PS5 Notification & System Language Definitions
 * ============================================================
 */

typedef struct notify_request {
    char useless1[45];
    char message[3075];
} notify_request_t;

int sceKernelSendNotificationRequest(int, notify_request_t *, size_t, int);
int sceSystemServiceParamGetInt(int param_id, int32_t *value);

#define SCE_SYSTEM_SERVICE_PARAM_ID_LANG 1

typedef enum {
    LANG_JA_JP = 0, LANG_EN_US, LANG_FR_FR, LANG_ES_ES, LANG_DE_DE,
    LANG_IT_IT, LANG_NL_NL, LANG_PT_PT, LANG_RU_RU, LANG_KO_KR,
    LANG_ZH_TW, LANG_ZH_CN, LANG_FI_FI, LANG_SV_SE, LANG_DA_DK,
    LANG_NO_NO, LANG_PL_PL, LANG_PT_BR, LANG_EN_GB, LANG_TR_TR,
    LANG_ES_MX, LANG_AR_SA, LANG_FR_CA, LANG_CS_CZ, LANG_HU_HU,
    LANG_EL_GR, LANG_RO_RO, LANG_TH_TH, LANG_VI_VN, LANG_ID_ID,
    LANG_UK_UA
} language_id_t;

typedef struct {
    const char *err_internal_only;
    const char *err_usb_and_internal;
    const char *err_bad_structure;
} notification_strings_t;

static const notification_strings_t lang_en = {
    "Error: message.txt not found in /data/ps5_autoloader/",
    "Error: message.txt not found in /data/ps5_autoloader/ nor in USB paths",
    "Error: Invalid structure expected in message.txt"
};

static const notification_strings_t lang_es_es = {
    "Error: no se ha encontrado el archivo message.txt en /data/ps5_autoloader/",
    "Error: no se encontro message.txt en /data/ps5_autoloader/ ni en las rutas USB",
    "Error: estructura esperada no encontrada en message.txt"
};

static const notification_strings_t lang_es_mx = {
    "Error: no se ha encontrado el archivo message.txt en /data/ps5_autoloader/",
    "Error: no se encontro message.txt en /data/ps5_autoloader/ ni en las rutas USB",
    "Error: estructura esperada no encontrada en message.txt"
};

static const notification_strings_t lang_ja = {
    "エラー: /data/ps5_autoloader/ に message.txt が見つかりません",
    "エラー: /data/ps5_autoloader/ および USB パスに message.txt が見つかりません",
    "エラー: message.txt の構造が無効です"
};

static const notification_strings_t lang_fr_fr = {
    "Erreur : message.txt introuvable dans /data/ps5_autoloader/",
    "Erreur : message.txt introuvable dans /data/ps5_autoloader/ ni sur les chemins USB",
    "Erreur : structure attendue invalide dans message.txt"
};

static const notification_strings_t lang_de = {
    "Fehler: message.txt wurde in /data/ps5_autoloader/ nicht gefunden",
    "Fehler: message.txt weder in /data/ps5_autoloader/ noch in USB-Pfaden gefunden",
    "Fehler: Ungültige Struktur in message.txt erwartet"
};

static const notification_strings_t lang_it = {
    "Errore: message.txt non trovato in /data/ps5_autoloader/",
    "Errore: message.txt non trovato né in /data/ps5_autoloader/ né nei percorsi USB",
    "Errore: struttura attesa non valida in message.txt"
};

static const notification_strings_t lang_pt_br = {
    "Erro: message.txt nao encontrado em /data/ps5_autoloader/",
    "Erro: message.txt nao encontrado em /data/ps5_autoloader/ nem nos caminhos USB",
    "Erro: Estrutura esperada invalida em message.txt"
};

const notification_strings_t *get_language_strings(void) {
    int32_t language_id = -1;
    if (sceSystemServiceParamGetInt(SCE_SYSTEM_SERVICE_PARAM_ID_LANG, &language_id) != 0) {
        return &lang_en;
    }
    switch (language_id) {
        case LANG_JA_JP: return &lang_ja;
        case LANG_ES_ES: return &lang_es_es;
        case LANG_ES_MX: return &lang_es_mx;
        case LANG_FR_FR: return &lang_fr_fr;
        case LANG_DE_DE: return &lang_de;
        case LANG_IT_IT: return &lang_it;
        case LANG_PT_BR: case LANG_PT_PT: return &lang_pt_br;
        default: return &lang_en;
    }
}

void show_notification(const char *msg) {
    notify_request_t req;
    bzero(&req, sizeof(req));
    strncpy(req.message, msg, sizeof(req.message) - 1);
    sceKernelSendNotificationRequest(0, &req, sizeof(req), 0);
}

void execute_notification_logic(void) {
    char filepath[128];
    char line_buffer[256];
    FILE *file = NULL;
    int usb_connected_flag = 0;
    const notification_strings_t *msg = get_language_strings();

    for (int i = 0; i <= 7; i++) {
        char usb_base[32];
        struct stat st;
        snprintf(usb_base, sizeof(usb_base), "/mnt/usb%d", i);
        if (stat(usb_base, &st) == 0 && S_ISDIR(st.st_mode)) {
            usb_connected_flag = 1;
        }
        snprintf(filepath, sizeof(filepath), "/mnt/usb%d/ps5_autoloader/message.txt", i);
        file = fopen(filepath, "r");
        if (file) break;
    }

    if (!file) {
        snprintf(filepath, sizeof(filepath), "/data/ps5_autoloader/message.txt");
        file = fopen(filepath, "r");
    }

    if (!file) {
        if (!usb_connected_flag) show_notification(msg->err_internal_only);
        else show_notification(msg->err_usb_and_internal);
        return;
    }

    int max_lines = 3;
    int chars_per_line = 90;
    int delay_ms = 300;

    if (fgets(line_buffer, sizeof(line_buffer), file) == NULL || strncmp(line_buffer, "LINES=", 6) != 0) {
        show_notification(msg->err_bad_structure);
        fclose(file);
        return;
    }
    max_lines = atoi(line_buffer + 6);

    if (fgets(line_buffer, sizeof(line_buffer), file) == NULL || strncmp(line_buffer, "CHARS=", 6) != 0) {
        show_notification(msg->err_bad_structure);
        fclose(file);
        return;
    }
    chars_per_line = atoi(line_buffer + 6);

    if (fgets(line_buffer, sizeof(line_buffer), file) == NULL || strncmp(line_buffer, "DELAY=", 6) != 0) {
        show_notification(msg->err_bad_structure);
        fclose(file);
        return;
    }
    delay_ms = atoi(line_buffer + 6);

    if (fgets(line_buffer, sizeof(line_buffer), file) == NULL || strncmp(line_buffer, "---", 3) != 0) {
        show_notification(msg->err_bad_structure);
        fclose(file);
        return;
    }

    if (max_lines < 1) max_lines = 1;
    if (max_lines > 10) max_lines = 10;
    if (chars_per_line < 10) chars_per_line = 10;
    if (chars_per_line > 200) chars_per_line = 200;

    int lines_shown = 0;
    while (fgets(line_buffer, sizeof(line_buffer), file) != NULL && lines_shown < max_lines) {
        size_t len = strlen(line_buffer);
        if (len > 0 && (line_buffer[len - 1] == '\n' || line_buffer[len - 1] == '\r')) {
            line_buffer[--len] = '\0';
        }
        if (len == 0) continue;

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
}

/*
 * ============================================================
 * HTML Frontend (Con precarga de datos desde el backend)
 * ============================================================
 */
const char* html_template = 
"<!DOCTYPE html>\n"
"<html lang=\"es\">\n"
"<head>\n"
"    <meta charset=\"UTF-8\">\n"
"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
"    <title>Message Maker - PS5 Notification Suite</title>\n"
"    <style>\n"
"        * { box-sizing: border-box; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; }\n"
"        html, body {\n"
"            background-color: #090d16;\n"
"            color: #ffffff;\n"
"            margin: 0; padding: 0;\n"
"            width: 100vw; height: 100vh;\n"
"            overflow: hidden;\n"
"            display: flex;\n"
"            justify-content: center;\n"
"            align-items: center;\n"
"        }\n"
"        #main-container {\n"
"            display: flex;\n"
"            flex-direction: column;\n"
"            width: 700px; height: 680px;\n"
"            background-color: #0e1422;\n"
"            border: 1px solid #1e293b;\n"
"            border-radius: 8px;\n"
"            padding: 12px 20px;\n"
"        }\n"
"        #header { margin-bottom: 8px; }\n"
"        #header h1 { font-size: 22px; font-weight: normal; margin: 0; color: #ffffff; }\n"
"        #header p { font-size: 12px; color: #8292a8; margin: 2px 0 0 0; }\n"
"        \n"
"        .setting-row {\n"
"            background-color: #111622;\n"
"            border: 1px solid #22304a;\n"
"            padding: 8px 12px;\n"
"            margin-bottom: 6px;\n"
"            display: flex;\n"
"            justify-content: space-between;\n"
"            align-items: center;\n"
"            border-radius: 4px;\n"
"        }\n"
"        .setting-label { font-size: 14px; color: #e2e8f0; font-weight: 500; }\n"
"        .control-group { display: flex; align-items: center; gap: 8px; }\n"
"        select, .custom-input {\n"
"            background-color: #1a2336;\n"
"            color: #ffffff;\n"
"            border: 1px solid #334155;\n"
"            padding: 4px 8px;\n"
"            font-size: 14px;\n"
"            border-radius: 3px;\n"
"        }\n"
"        .custom-input { width: 75px; display: none; }\n"
"        .custom-checkbox-label {\n"
"            font-size: 13px;\n"
"            color: #94a3b8;\n"
"            display: flex;\n"
"            align-items: center;\n"
"            gap: 4px;\n"
"            cursor: pointer;\n"
"        }\n"
"        .custom-checkbox-label input { transform: scale(1.2); }\n"
"        \n"
"        #separator { height: 1px; background-color: #1a2234; margin: 8px 0; }\n"
"        #text-section-label { font-size: 14px; font-weight: bold; color: #94a3b8; margin-bottom: 6px; }\n"
"        \n"
"        #inputs-frame {\n"
"            flex: 1;\n"
"            overflow-y: auto;\n"
"            padding-right: 5px;\n"
"            margin-bottom: 6px;\n"
"        }\n"
"        .input-row {\n"
"            background-color: #111622;\n"
"            border: 1px solid #1e293b;\n"
"            padding: 6px 10px;\n"
"            margin-bottom: 5px;\n"
"            display: flex;\n"
"            align-items: center;\n"
"            border-radius: 4px;\n"
"        }\n"
"        .line-label { font-size: 13px; font-weight: bold; color: #64748b; width: 35px; }\n"
"        .message-input {\n"
"            flex: 1;\n"
"            background-color: transparent;\n"
"            color: #ffffff;\n"
"            border: none; outline: none;\n"
"            font-size: 14px; padding: 3px;\n"
"        }\n"
"        .char-count { font-size: 12px; color: #64748b; width: 60px; text-align: right; }\n"
"        \n"
"        .btn-container { display: flex; gap: 10px; margin-top: 10px; }\n"
"        .action-btn {\n"
"            flex: 1;\n"
"            background-color: #162035;\n"
"            color: #ffffff;\n"
"            border: 1px solid #3b82f6;\n"
"            padding: 12px;\n"
"            font-size: 14px;\n"
"            font-weight: bold;\n"
"            cursor: pointer;\n"
"            border-radius: 4px;\n"
"            transition: background-color: 0.2s;\n"
"        }\n"
"        .action-btn:hover { background-color: #1e2d4d; }\n"
"        #run-preview-btn { border-color: #10b981; background-color: #064e3b; }\n"
"        #run-preview-btn:hover { background-color: #047857; }\n"
"    </style>\n"
"</head>\n"
"<body>\n"
"    <div id=\"main-container\">\n"
"        <div id=\"header\">\n"
"            <h1>Settings</h1>\n"
"            <p>Notification Payload & Message Generator</p>\n"
"        </div>\n"
"        <div id=\"settings-list\">\n"
"            <div class=\"setting-row\">\n"
"                <span class=\"setting-label\">Max Lines</span>\n"
"                <div class=\"control-group\">\n"
"                    <select id=\"lines-select\">\n"
"                        <option value=\"1\">1</option>\n"
"                        <option value=\"2\">2</option>\n"
"                        <option value=\"3\">3</option>\n"
"                        <option value=\"4\">4</option>\n"
"                        <option value=\"5\">5</option>\n"
"                    </select>\n"
"                    <input type=\"number\" id=\"lines-custom\" class=\"custom-input\">\n"
"                    <label class=\"custom-checkbox-label\">\n"
"                        <input type=\"checkbox\" id=\"lines-custom-check\"> Custom\n"
"                    </label>\n"
"                </div>\n"
"            </div>\n"
"            <div class=\"setting-row\">\n"
"                <span class=\"setting-label\">Chars per Line</span>\n"
"                <div class=\"control-group\">\n"
"                    <select id=\"chars-select\">\n"
"                        <option value=\"60\">60</option>\n"
"                        <option value=\"80\">80</option>\n"
"                        <option value=\"90\">90</option>\n"
"                        <option value=\"100\">100</option>\n"
"                        <option value=\"120\">120</option>\n"
"                    </select>\n"
"                    <input type=\"number\" id=\"chars-custom\" class=\"custom-input\">\n"
"                    <label class=\"custom-checkbox-label\">\n"
"                        <input type=\"checkbox\" id=\"chars-custom-check\"> Custom\n"
"                    </label>\n"
"                </div>\n"
"            </div>\n"
"            <div class=\"setting-row\">\n"
"                <span class=\"setting-label\">Display Delay (ms)</span>\n"
"                <div class=\"control-group\">\n"
"                    <select id=\"delay-select\">\n"
"                        <option value=\"0\">0</option>\n"
"                        <option value=\"100\">100</option>\n"
"                        <option value=\"300\">300</option>\n"
"                        <option value=\"500\">500</option>\n"
"                        <option value=\"1000\">1000</option>\n"
"                    </select>\n"
"                    <input type=\"number\" id=\"delay-custom\" class=\"custom-input\">\n"
"                    <label class=\"custom-checkbox-label\">\n"
"                        <input type=\"checkbox\" id=\"delay-custom-check\"> Custom\n"
"                    </label>\n"
"                </div>\n"
"            </div>\n"
"        </div>\n"
"        <div id=\"separator\">-></div>\n"
"        <div id=\"text-section-label\">Notification Content</div>\n"
"        <div id=\"inputs-frame\"></div>\n"
"        <div class=\"btn-container\">\n"
"            <button id=\"generate-btn\" class=\"action-btn\">Generate message.txt</button>\n"
"            <button id=\"run-preview-btn\" class=\"action-btn\">Run Preview</button>\n"
"        </div>\n"
"    </div>\n"
"    <script>\n"
"        // Datos precargados inyectados desde el servidor C\n"
"        const initialData = %s;\n"
"        \n"
"        const standardLines = [1, 2, 3, 4, 5];\n"
"        const standardChars = [60, 80, 90, 100, 120];\n"
"        const standardDelays = [0, 100, 300, 500, 1000];\n"
"\n"
"        const state = { \n"
"            maxLines: initialData.lines,\n"
"            charsPerLine: initialData.chars,\n"
"            delay: initialData.delay,\n"
"            messages: initialData.messages \n"
"        };\n"
"\n"
"        const els = {\n"
"            linesSelect: document.getElementById('lines-select'), linesCustom: document.getElementById('lines-custom'), linesCheck: document.getElementById('lines-custom-check'),\n"
"            charsSelect: document.getElementById('chars-select'), charsCustom: document.getElementById('chars-custom'), charsCheck: document.getElementById('chars-custom-check'),\n"
"            delaySelect: document.getElementById('delay-select'), delayCustom: document.getElementById('delay-custom'), delayCheck: document.getElementById('delay-custom-check'),\n"
"            inputsFrame: document.getElementById('inputs-frame'), generateBtn: document.getElementById('generate-btn'), runPreviewBtn: document.getElementById('run-preview-btn')\n"
"        };\n"
"\n"
"        const setupInitialControls = () => {\n"
"            if (standardLines.includes(state.maxLines)) {\n"
"                els.linesSelect.value = state.maxLines;\n"
"                els.linesCustom.value = state.maxLines;\n"
"                els.linesCheck.checked = false;\n"
"                els.linesSelect.style.display = 'block';\n"
"                els.linesCustom.style.display = 'none';\n"
"            } else {\n"
"                els.linesCustom.value = state.maxLines;\n"
"                els.linesCheck.checked = true;\n"
"                els.linesSelect.style.display = 'none';\n"
"                els.linesCustom.style.display = 'block';\n"
"            }\n"
"\n"
"            if (standardChars.includes(state.charsPerLine)) {\n"
"                els.charsSelect.value = state.charsPerLine;\n"
"                els.charsCustom.value = state.charsPerLine;\n"
"                els.charsCheck.checked = false;\n"
"                els.charsSelect.style.display = 'block';\n"
"                els.charsCustom.style.display = 'none';\n"
"            } else {\n"
"                els.charsCustom.value = state.charsPerLine;\n"
"                els.charsCheck.checked = true;\n"
"                els.charsSelect.style.display = 'none';\n"
"                els.charsCustom.style.display = 'block';\n"
"            }\n"
"\n"
"            if (standardDelays.includes(state.delay)) {\n"
"                els.delaySelect.value = state.delay;\n"
"                els.delayCustom.value = state.delay;\n"
"                els.delayCheck.checked = false;\n"
"                els.delaySelect.style.display = 'block';\n"
"                els.delayCustom.style.display = 'none';\n"
"            } else {\n"
"                els.delayCustom.value = state.delay;\n"
"                els.delayCheck.checked = true;\n"
"                els.delaySelect.style.display = 'none';\n"
"                els.delayCustom.style.display = 'block';\n"
"            }\n"
"        };\n"
"\n"
"        const getVal = (s, c, ch) => ch.checked ? parseInt(c.value) : parseInt(s.value);\n"
"        \n"
"        const buildTextInputs = () => {\n"
"            const currentInputs = els.inputsFrame.querySelectorAll('.message-input');\n"
"            const oldMessages = Array.from(currentInputs).map(i => i.value);\n"
"            els.inputsFrame.innerHTML = ''; \n"
"            let newMessages = [];\n"
"            for (let i = 0; i < state.maxLines; i++) {\n"
"                const row = document.createElement('div'); row.className = 'input-row';\n"
"                const label = document.createElement('span'); label.className = 'line-label'; label.textContent = `L${i + 1}`;\n"
"                const input = document.createElement('input'); \n"
"                input.type = 'text'; input.className = 'message-input'; input.maxLength = state.charsPerLine; input.dataset.index = i;\n"
"                \n"
"                if (i < oldMessages.length && oldMessages[i]) {\n"
"                    input.value = oldMessages[i].substring(0, state.charsPerLine);\n"
"                } else if (i < state.messages.length && state.messages[i]) {\n"
"                    input.value = state.messages[i].substring(0, state.charsPerLine);\n"
"                }\n"
"                \n"
"                newMessages.push(input.value);\n"
"                input.addEventListener('input', updateInputState);\n"
"                const count = document.createElement('span'); \n"
"                count.className = 'char-count'; count.id = `count-${i}`; count.textContent = `${input.value.length}/${state.charsPerLine}`;\n"
"                row.appendChild(label); row.appendChild(input); row.appendChild(count);\n"
"                els.inputsFrame.appendChild(row);\n"
"            }\n"
"            state.messages = newMessages;\n"
"        };\n"
"        \n"
"        const updateInputState = (e) => {\n"
"            const input = e.target; const index = input.dataset.index;\n"
"            if (input.value.length > state.charsPerLine) {\n"
"                input.value = input.value.substring(0, state.charsPerLine);\n"
"            }\n"
"            state.messages[index] = input.value;\n"
"            document.getElementById(`count-${index}`).textContent = `${input.value.length}/${state.charsPerLine}`;\n"
"        };\n"
"        \n"
"        const toggleCustom = (s, c, ch) => {\n"
"            s.style.display = ch.checked ? 'none' : 'block';\n"
"            c.style.display = ch.checked ? 'block' : 'none';\n"
"            updateSettings();\n"
"        };\n"
"        \n"
"        const updateSettings = () => {\n"
"            state.maxLines = getVal(els.linesSelect, els.linesCustom, els.linesCheck);\n"
"            state.charsPerLine = getVal(els.charsSelect, els.charsCustom, els.charsCheck);\n"
"            state.delay = getVal(els.delaySelect, els.delayCustom, els.delayCheck);\n"
"            buildTextInputs();\n"
"        };\n"
"        \n"
"        const validateAndGetContent = () => {\n"
"            const inputs = els.inputsFrame.querySelectorAll('.message-input');\n"
"            let textLines = Array.from(inputs).map(i => i.value);\n"
"            let hasText = textLines.some(line => line.trim() !== \"\");\n"
"            \n"
"            if (!hasText) {\n"
"                alert('Error: All message lines are empty. Please enter at least one message.');\n"
"                return null;\n"
"            }\n"
"            \n"
"            let content = `LINES=${state.maxLines}\\nCHARS=${state.charsPerLine}\\nDELAY=${state.delay}\\n---\\n`;\n"
"            for (let line of textLines) content += `${line}\\n`;\n"
"            return content;\n"
"        };\n"
"        \n"
"        const generateFile = () => {\n"
"            const content = validateAndGetContent();\n"
"            if (!content) return;\n"
"            \n"
"            fetch('/save_message', { method: 'POST', body: content })\n"
"            .then(res => { if (res.ok) alert('message.txt successfully created in /data/ps5_autoloader'); else alert('Error saving file on server'); })\n"
"            .catch(() => alert('Connection error with PS5 payload'));\n"
"        };\n"
"        \n"
"        const runPreview = () => {\n"
"            const content = validateAndGetContent();\n"
"            if (!content) return;\n"
"            \n"
"            fetch('/run_preview', { method: 'POST', body: content })\n"
"            .then(res => { if (res.ok) alert('Preview executed successfully on console'); else alert('Error executing preview'); })\n"
"            .catch(() => alert('Connection error with PS5 payload'));\n"
"        };\n"
"        \n"
"        els.linesCheck.addEventListener('change', () => toggleCustom(els.linesSelect, els.linesCustom, els.linesCheck));\n"
"        els.charsCheck.addEventListener('change', () => toggleCustom(els.charsSelect, els.charsCustom, els.charsCheck));\n"
"        els.delayCheck.addEventListener('change', () => toggleCustom(els.delaySelect, els.delayCustom, els.delayCheck));\n"
"        els.linesSelect.addEventListener('change', updateSettings); els.charsSelect.addEventListener('change', updateSettings); els.delaySelect.addEventListener('change', updateSettings);\n"
"        els.linesCustom.addEventListener('input', updateSettings); els.charsCustom.addEventListener('input', updateSettings); els.delayCustom.addEventListener('input', updateSettings);\n"
"        \n"
"        els.generateBtn.addEventListener('click', generateFile);\n"
"        els.runPreviewBtn.addEventListener('click', runPreview);\n"
"        \n"
"        setupInitialControls();\n"
"        buildTextInputs();\n"
"    </script>\n"
"</body>\n"
"</html>\n";

__attribute__((constructor)) static void constructor(void) {
    sceUserServiceInitialize(0);
}

int ensure_directory_exists(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0777) == -1) return -1;
    }
    return 0;
}

int save_message_file(const char *content, size_t content_len) {
    if (ensure_directory_exists(TARGET_DIR) != 0) return -1;
    int fd = open(TARGET_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) return -1;
    if (write(fd, content, content_len) == -1) {
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

/*
 * Función para leer message.txt y generar el JSON inicial para la página web
 */
void get_html_with_preloaded_data(char *out_html, size_t max_len) {
    int lines = 3;
    int chars = 90;
    int delay = 300;
    char messages_json[4096] = "[\"\", \"\", \"\"]";

    FILE *file = fopen(TARGET_FILE, "r");
    if (file) {
        char line_buf[256];
        int l = 3, c = 90, d = 300;
        
        if (fgets(line_buf, sizeof(line_buf), file) && strncmp(line_buf, "LINES=", 6) == 0) l = atoi(line_buf + 6);
        if (fgets(line_buf, sizeof(line_buf), file) && strncmp(line_buf, "CHARS=", 6) == 0) c = atoi(line_buf + 6);
        if (fgets(line_buf, sizeof(line_buf), file) && strncmp(line_buf, "DELAY=", 6) == 0) d = atoi(line_buf + 6);
        
        // Omitir separador ---
        fgets(line_buf, sizeof(line_buf), file);

        lines = l; chars = c; delay = d;

        char temp_msgs[2048] = "";
        int count = 0;
        while (fgets(line_buf, sizeof(line_buf), file) != NULL && count < lines) {
            size_t len = strlen(line_buf);
            if (len > 0 && (line_buf[len - 1] == '\n' || line_buf[len - 1] == '\r')) {
                line_buf[--len] = '\0';
            }
            // Escapar comillas dobles si las hubiera
            char escaped[512];
            int e = 0;
            for(int i = 0; line_buf[i] != '\0' && e < 500; i++) {
                if(line_buf[i] == '"') { escaped[e++] = '\\'; escaped[e++] = '"'; }
                else { escaped[e++] = line_buf[i]; }
            }
            escaped[e] = '\0';

            if (count > 0) strcat(temp_msgs, ", ");
            char single_msg[600];
            snprintf(single_msg, sizeof(single_msg), "\"%s\"", escaped);
            strcat(temp_msgs, single_msg);
            count++;
        }

        // Rellenar si faltaban líneas
        while (count < lines) {
            if (count > 0) strcat(temp_msgs, ", ");
            strcat(temp_msgs, "\"\"");
            count++;
        }

        snprintf(messages_json, sizeof(messages_json), "[%s]", temp_msgs);
        fclose(file);
    }

    char json_data[4600];
    snprintf(json_data, sizeof(json_data), "{\"lines\": %d, \"chars\": %d, \"delay\": %d, \"messages\": %s}", lines, chars, delay, messages_json);
    snprintf(out_html, max_len, html_template, json_data);
}

void start_web_server(void) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) exit(EXIT_FAILURE);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(SERVER_PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) exit(EXIT_FAILURE);
    if (listen(server_fd, 5) < 0) exit(EXIT_FAILURE);

    while(1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) continue;
        
        char buffer[8192];
        memset(buffer, 0, sizeof(buffer));
        int total_bytes = 0;
        int bytes_read = 0;

        while ((bytes_read = read(new_socket, buffer + total_bytes, sizeof(buffer) - total_bytes - 1)) > 0) {
            total_bytes += bytes_read;
            if (strstr(buffer, "\r\n\r\n")) {
                char *content_length_hdr = strstr(buffer, "Content-Length: ");
                if (content_length_hdr) {
                    int content_len = atoi(content_length_hdr + 16);
                    char *body_start = strstr(buffer, "\r\n\r\n") + 4;
                    if ((buffer + total_bytes - body_start) >= content_len) {
                        break;
                    }
                } else {
                    break;
                }
            }
        }

        if (strncmp(buffer, "GET / ", 6) == 0) {
            char final_html[65536];
            get_html_with_preloaded_data(final_html, sizeof(final_html));

            char response_hdr[512];
            int hdr_len = sprintf(response_hdr, "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: %lu\r\nConnection: close\r\n\r\n", strlen(final_html));
            write(new_socket, response_hdr, hdr_len);
            write(new_socket, final_html, strlen(final_html));
        } 
        else if (strncmp(buffer, "POST /save_message", 18) == 0) {
            char *body = strstr(buffer, "\r\n\r\n");
            if (body) {
                body += 4;
                char *content_len_hdr = strstr(buffer, "Content-Length: ");
                size_t payload_size = content_len_hdr ? atoi(content_len_hdr + 16) : strlen(body);
                
                if (save_message_file(body, payload_size) == 0) {
                    const char *res = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nOK";
                    write(new_socket, res, strlen(res));
                } else {
                    const char *res = "HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\n\r\nFAIL";
                    write(new_socket, res, strlen(res));
                }
            }
        }
        else if (strncmp(buffer, "POST /run_preview", 17) == 0) {
            char *body = strstr(buffer, "\r\n\r\n");
            if (body) {
                body += 4;
                char *content_len_hdr = strstr(buffer, "Content-Length: ");
                size_t payload_size = content_len_hdr ? atoi(content_len_hdr + 16) : strlen(body);
                
                if (save_message_file(body, payload_size) == 0) {
                    const char *res = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nOK";
                    write(new_socket, res, strlen(res));
                    execute_notification_logic();
                } else {
                    const char *res = "HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\n\r\nFAIL";
                    write(new_socket, res, strlen(res));
                }
            }
        }
        else {
            const char *res = "HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n";
            write(new_socket, res, strlen(res));
        }
        close(new_socket);
    }
}

int main(void) {
    char url[64];
    sprintf(url, "http://127.0.0.1:%d", SERVER_PORT);
    sceSystemServiceLaunchWebBrowser(url, 0);
    start_web_server();
    return EXIT_SUCCESS;
}

__attribute__((destructor)) static void destructor(void) {
    sceUserServiceTerminate();
}