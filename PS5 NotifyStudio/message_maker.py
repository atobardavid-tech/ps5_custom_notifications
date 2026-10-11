import os
import textwrap
import tkinter as tk
from tkinter import messagebox, ttk
import sys

class PS5MessageMakerApp:
    def __init__(self, root):
        self.root = root
        self.root.title("PS5 NotifyStudio - Message Maker")
        self.root.geometry("980x820")
        self.root.configure(bg="#090d16")
        self.root.resizable(False, False)

        self.setup_styles()

        main_container = tk.Frame(root, bg="#090d16")
        main_container.pack(fill=tk.BOTH, expand=True, padx=20, pady=20)

        # Columna Izquierda (Controles y Entradas)
        left_column = tk.Frame(main_container, bg="#090d16")
        left_column.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(0, 10))

        header_frame = tk.Frame(left_column, bg="#090d16")
        header_frame.pack(fill=tk.X, pady=(0, 15))

        title_lbl = tk.Label(header_frame, text="Notification Payload & Message Generator", font=("Segoe UI", 20, "normal"), bg="#090d16", fg="#ffffff")
        title_lbl.pack(anchor="w")
        subtitle_lbl = tk.Label(header_frame, text="Settings", font=("Segoe UI", 10, "bold"), bg="#090d16", fg="#94a3b8")
        subtitle_lbl.pack(anchor="w", pady=(2, 0))

        self.list_container = tk.Frame(left_column, bg="#090d16")
        self.list_container.pack(fill=tk.BOTH, expand=True)

        self.lines_var = tk.StringVar(value="3")
        self.chars_var = tk.StringVar(value="90")
        self.delay_var = tk.StringVar(value="300")

        self.custom_lines_var = tk.BooleanVar(value=False)
        self.custom_chars_var = tk.BooleanVar(value=False)
        self.custom_delay_var = tk.BooleanVar(value=False)

        # Intentar cargar valores iniciales desde message.txt si existe en la ruta
        loaded_lines, loaded_chars, loaded_delay, loaded_messages = self.load_initial_file_data()
        if loaded_lines is not None:
            self.lines_var.set(str(loaded_lines))
            if str(loaded_lines) not in ["1", "2", "3", "4", "5"]:
                self.custom_lines_var.set(True)
        if loaded_chars is not None:
            self.chars_var.set(str(loaded_chars))
            if str(loaded_chars) not in ["60", "80", "90", "100", "120"]:
                self.custom_chars_var.set(True)
        if loaded_delay is not None:
            self.delay_var.set(str(loaded_delay))
            if str(loaded_delay) not in ["0", "100", "300", "500", "1000"]:
                self.custom_delay_var.set(True)

        self.row_lines = self.create_setting_row("Max Lines", ["1", "2", "3", "4", "5"], self.lines_var, self.custom_lines_var, 'lines')
        self.row_chars = self.create_setting_row("Chars per Line", ["60", "80", "90", "100", "120"], self.chars_var, self.custom_chars_var, 'chars')
        self.row_delay = self.create_setting_row("Display Delay (ms)", ["0", "100", "300", "500", "1000"], self.delay_var, self.custom_delay_var, 'delay')

        self.add_separator()

        text_section_lbl = tk.Label(self.list_container, text="Notification Content", font=("Segoe UI", 10, "bold"), bg="#090d16", fg="#94a3b8")
        text_section_lbl.pack(anchor="w", pady=(5, 5))

        self.inputs_frame = tk.Frame(self.list_container, bg="#090d16")
        self.inputs_frame.pack(fill=tk.BOTH, expand=True)

        # --- COLUMNA DERECHA: PREVISUALIZACIÓN MULTI-TARJETA DE PS5 ---
        right_column = tk.Frame(main_container, bg="#0e1422", highlightbackground="#1e293b", highlightthickness=1)
        right_column.pack(side=tk.RIGHT, fill=tk.BOTH, ipadx=10, ipady=10)

        preview_title = tk.Label(right_column, text="PS5 Notification Preview", font=("Segoe UI", 11, "bold"), bg="#0e1422", fg="#e2e8f0")
        preview_title.pack(anchor="w", pady=(0, 15))

        self.cards_preview_container = tk.Frame(right_column, bg="#0e1422")
        self.cards_preview_container.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        self.anim_btn = tk.Button(
            right_column, text="▶ Test Cascade Animation", 
            bg="#1c2638", fg="#ffffff", activebackground="#3b82f6", activeforeground="#ffffff", 
            font=("Segoe UI", 9, "bold"), bd=0, relief=tk.FLAT, pady=8, 
            command=self.toggle_animation, cursor="hand2"
        )
        self.anim_btn.pack(fill=tk.X, pady=(15, 0))

        self.animation_running = False
        self.animation_after_id = None
        self.entry_list = []

        self.build_text_inputs(loaded_messages)

        # Botón de generación en la columna izquierda
        btn_frame = tk.Frame(left_column, bg="#090d16")
        btn_frame.pack(fill=tk.X, pady=(10, 0))

        generate_btn = tk.Button(
            btn_frame, text="Generate message.txt", 
            bg="#162035", fg="#ffffff", activebackground="#1e2d4d", activeforeground="#ffffff", 
            font=("Segoe UI", 10, "bold"), bd=0, relief=tk.FLAT, pady=10, 
            command=self.generate_file, cursor="hand2"
        )
        generate_btn.pack(fill=tk.X)

    def get_base_path(self):
        # Detecta si se ejecuta como ejecutable compilado (PyInstaller) o como script normal
        if getattr(sys, 'frozen', False):
            return os.path.dirname(sys.executable)
        else:
            return os.path.dirname(os.path.abspath(__file__))

    def load_initial_file_data(self):
        try:
            base_dir = self.get_base_path()
            file_path = os.path.join(base_dir, "message.txt")
            if not os.path.exists(file_path):
                return None, None, None, []
            
            with open(file_path, "r", encoding="utf-8") as f:
                lines = f.readlines()
            
            if len(lines) < 4:
                return None, None, None, []
            
            l_val = int(lines[0].strip().split("=")[1])
            c_val = int(lines[1].strip().split("=")[1])
            d_val = int(lines[2].strip().split("=")[1])
            
            msg_lines = [line.rstrip("\r\n") for line in lines[4:]]
            return l_val, c_val, d_val, msg_lines
        except Exception:
            return None, None, None, []

    def setup_styles(self):
        style = ttk.Style()
        style.theme_use("clam")
        style.configure(
            "TCombobox", 
            fieldbackground="#111622", background="#1c2638", foreground="#ffffff", 
            selectbackground="#3b82f6", selectforeground="#ffffff", bordercolor="#1c2638", arrowcolor="#ffffff"
        )
        style.map("TCombobox", fieldbackground=[("readonly", "#111622")], foreground=[("readonly", "#ffffff")])
        self.root.option_add("*TCombobox*Listbox*background", "#111622")
        self.root.option_add("*TCombobox*Listbox*foreground", "#ffffff")
        self.root.option_add("*TCombobox*Listbox*selectBackground", "#3b82f6")
        self.root.option_add("*TCombobox*Listbox*selectForeground", "#ffffff")

    def add_separator(self):
        sep = tk.Frame(self.list_container, bg="#1a2234", height=1)
        sep.pack(fill=tk.X, pady=8)

    def create_setting_row(self, label_text, values, var, custom_var, field_type):
        row = tk.Frame(self.list_container, bg="#111622", highlightbackground="#22304a", highlightthickness=1)
        row.pack(fill=tk.X, pady=2, ipadx=8, ipady=6)

        def on_enter(e):
            row.config(bg="#18233c", highlightbackground="#3b82f6")
            inner_frame.config(bg="#18233c")
            lbl.config(bg="#18233c")
            chk.config(bg="#18233c", activebackground="#18233c")

        def on_leave(e):
            row.config(bg="#111622", highlightbackground="#22304a")
            inner_frame.config(bg="#111622")
            lbl.config(bg="#111622")
            chk.config(bg="#111622", activebackground="#111622")

        row.bind("<Enter>", on_enter)
        row.bind("<Leave>", on_leave)

        inner_frame = tk.Frame(row, bg="#111622")
        inner_frame.pack(fill=tk.BOTH, expand=True)

        lbl = tk.Label(inner_frame, text=label_text, font=("Segoe UI", 9), bg="#111622", fg="#e2e8f0", anchor="w")
        lbl.pack(side=tk.LEFT, padx=5)

        control_subframe = tk.Frame(inner_frame, bg="#111622")
        control_subframe.pack(side=tk.RIGHT, padx=5)

        combo = ttk.Combobox(control_subframe, textvariable=var, values=values, width=5, state="readonly")
        combo.bind("<<ComboboxSelected>>", lambda e: self.build_text_inputs())

        entry = tk.Entry(control_subframe, width=7, font=("Segoe UI", 9), bg="#090d16", fg="#ffffff", insertbackground="white", bd=1, relief=tk.SOLID)
        entry.insert(0, var.get())
        entry.bind("<KeyRelease>", lambda e: self.build_text_inputs())

        chk = tk.Checkbutton(
            control_subframe, text="Custom", variable=custom_var, 
            bg="#111622", fg="#64748b", selectcolor="#090d16", activebackground="#111622", activeforeground="#ffffff", 
            font=("Segoe UI", 8), command=lambda: self.toggle_row_custom(combo, entry, custom_var)
        )
        chk.pack(side=tk.LEFT, padx=3)

        if custom_var.get():
            combo.pack_forget()
            entry.pack(side=tk.LEFT, padx=3)
        else:
            entry.pack_forget()
            combo.pack(side=tk.LEFT, padx=3)

        return {"combo": combo, "entry": entry, "var": var, "custom": custom_var}

    def toggle_row_custom(self, combo, entry, custom_var):
        if custom_var.get():
            combo.pack_forget()
            entry.pack(side=tk.LEFT, padx=3)
        else:
            entry.pack_forget()
            combo.pack(side=tk.LEFT, padx=3)
        self.build_text_inputs()

    def get_field_val(self, row_data):
        try:
            val = row_data["entry"].get().strip() if row_data["custom"].get() else row_data["var"].get()
            return int(val)
        except ValueError:
            return None

    def render_preview_cards(self, active_texts=None):
        for widget in self.cards_preview_container.winfo_children():
            widget.destroy()

        texts = [e.get() for e in self.entry_list if e.get().strip()]
        if not texts:
            texts = ["Type a message to preview..."]

        texts_to_show = active_texts if active_texts is not None else texts

        for text in reversed(texts_to_show):
            card = tk.Frame(self.cards_preview_container, bg="#141a29", highlightbackground="#2a364f", highlightthickness=1)
            card.pack(fill=tk.NONE, anchor="w", pady=3, padx=2)

            card_header = tk.Label(card, text="Debug:", font=("Segoe UI", 9, "bold"), bg="#141a29", fg="#ffffff")
            card_header.pack(anchor="w", padx=6, pady=(2, 0))

            sep_card = tk.Frame(card, bg="#ffffff", height=1)
            sep_card.pack(fill=tk.X, padx=6, pady=(2, 4))

            wrapped_text = textwrap.fill(text, width=40)

            msg_lbl = tk.Label(
                card, 
                text=wrapped_text, 
                font=("Segoe UI", 9), 
                bg="#141a29", 
                fg="#cbd5e1", 
                justify=tk.LEFT,
                anchor="w",
                width=42
            )
            msg_lbl.pack(anchor="w", padx=6, pady=(0, 4))

    def update_live_preview(self):
        if hasattr(self, 'cards_preview_container') and not self.animation_running:
            self.render_preview_cards()

    def toggle_animation(self):
        if self.animation_running:
            self.stop_animation()
        else:
            self.play_preview_animation()

    def stop_animation(self):
        if self.animation_after_id:
            self.root.after_cancel(self.animation_after_id)
            self.animation_after_id = None
        self.animation_running = False
        self.anim_btn.config(text="▶ Test Cascade Animation", bg="#1c2638")
        self.render_preview_cards()

    def play_preview_animation(self):
        texts = [e.get() for e in self.entry_list if e.get().strip()]
        if not texts:
            messagebox.showinfo("Preview", "Please enter at least one message line to test the animation.")
            return

        delay_val = self.get_field_val(self.row_delay)
        if delay_val is None:
            delay_val = 300

        self.animation_running = True
        self.anim_btn.config(text="■ Stop Animation", bg="#dc2626")

        if delay_val == 0:
            self.render_preview_cards(texts)
            self.stop_animation()
            return

        def show_step(index):
            if not self.animation_running:
                return
            if index <= len(texts):
                current_batch = texts[:index]
                self.render_preview_cards(current_batch)
                self.animation_after_id = self.root.after(delay_val, lambda: show_step(index + 1))
            else:
                self.stop_animation()

        show_step(1)

    def build_text_inputs(self, preloaded_texts=None):
        current_texts = [e.get() for e in self.entry_list]
        if preloaded_texts and not current_texts:
            current_texts = preloaded_texts

        for widget in self.inputs_frame.winfo_children():
            widget.destroy()
        self.entry_list.clear()

        num_lines = self.get_field_val(self.row_lines)
        if num_lines is None or num_lines < 1:
            num_lines = 1
        elif num_lines > 10:
            num_lines = 10

        max_c = self.get_field_val(self.row_chars) or 90

        def validate_length(new_value):
            return len(new_value) <= max_c

        vcmd = (self.root.register(validate_length), '%P')

        for i in range(num_lines):
            row_frame = tk.Frame(self.inputs_frame, bg="#111622", highlightbackground="#1e293b", highlightthickness=1)
            row_frame.pack(fill=tk.X, pady=2, ipadx=5, ipady=4)

            line_lbl = tk.Label(row_frame, text=f"L{i+1}", font=("Segoe UI", 8, "bold"), bg="#111622", fg="#64748b", width=3, anchor="w")
            line_lbl.pack(side=tk.LEFT, padx=(5, 0))

            entry = tk.Entry(
                row_frame, font=("Segoe UI", 9), bg="#090d16", fg="#ffffff", 
                insertbackground="white", bd=0, validate="key", validatecommand=vcmd
            )
            entry.pack(side=tk.LEFT, fill=tk.X, expand=True, ipady=2, padx=5)

            if i < len(current_texts):
                entry.insert(0, current_texts[i][:max_c])

            char_count = tk.Label(row_frame, text=f"{len(entry.get())}/{max_c}", font=("Segoe UI", 8), bg="#111622", fg="#475569", width=6)
            char_count.pack(side=tk.RIGHT, padx=(0, 5))

            def on_key(event, ent=entry, lbl_c=char_count, mc=max_c):
                text = ent.get()
                if len(text) > mc:
                    ent.delete(mc, tk.END)
                    text = text[:mc]
                lbl_c.config(text=f"{len(text)}/{mc}")
                self.update_live_preview()

            entry.bind("<KeyRelease>", on_key)
            self.entry_list.append(entry)
        
        self.update_live_preview()

    def generate_file(self):
        lines_val = self.get_field_val(self.row_lines)
        chars_val = self.get_field_val(self.row_chars)
        delay_val = self.get_field_val(self.row_delay)

        if lines_val is None or chars_val is None or delay_val is None:
            messagebox.showerror("Error", "Please verify that all numeric settings are valid.")
            return

        text_lines = [entry.get() for entry in self.entry_list]

        content = f"LINES={lines_val}\n"
        content += f"CHARS={chars_val}\n"
        content += f"DELAY={delay_val}\n"
        content += "---\n"
        for line in text_lines:
            content += f"{line}\n"

        try:
            base_dir = self.get_base_path()
            file_path = os.path.join(base_dir, "message.txt")
            with open(file_path, "w", encoding="utf-8") as f:
                f.write(content)
            messagebox.showinfo("Success", f"'message.txt' successfully created at:\n{base_dir}")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to save file: {e}")

if __name__ == "__main__":
    root = tk.Tk()
    app = PS5MessageMakerApp(root)
    root.mainloop()