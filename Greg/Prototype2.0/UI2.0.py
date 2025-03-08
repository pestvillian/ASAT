import qrcode
import tkinter as tk
from tkinter import ttk, messagebox, filedialog, Canvas, Frame
import os
import time

SETTINGS_FILE = "settings.txt"


clipboard = {'well': None, 'step': None}
undo_stack = []

def copy_well(well_name):
    clipboard['well'] = copy.deepcopy(wells_data[well_name])

def paste_well():
    if clipboard['well']:
        if len(wells_data) >= 12:
            messagebox.showwarning("Limit Reached", "You can only add up to 12 wells.")
            return
        well_name = f"Well {len(wells_data) + 1}"
        wells_data[well_name] = copy.deepcopy(clipboard['well'])
        # Code to refresh UI for the new well


def copy_step(well_name, step_index):
    clipboard['step'] = copy.deepcopy(wells_data[well_name]['steps'][step_index])

def paste_step(well_name, steps_frame):
    if clipboard['step']:
        wells_data[well_name]['steps'].append(copy.deepcopy(clipboard['step']))
        # Code to refresh UI for the new step
        
def undo():
    if not undo_stack:
        messagebox.showinfo("Undo", "Nothing to undo.")
        return
    action, name, data = undo_stack.pop()
    if action == 'step':
        wells_data[name]['steps'].append(data)
        # Code to refresh UI
    elif action == 'well':
        wells_data[name] = data
        # Code to refresh UI
        
class ScrollableFrame(ttk.Frame):
    def __init__(self, container):
        super().__init__(container)
        canvas = Canvas(self)
        scrollbar = ttk.Scrollbar(self, orient="vertical", command=canvas.yview)
        self.scrollable_frame = ttk.Frame(canvas)

        self.scrollable_frame.bind(
            "<Configure>", lambda e: canvas.configure(scrollregion=canvas.bbox("all"))
        )

        window = canvas.create_window((0, 0), window=self.scrollable_frame, anchor="nw")
        canvas.configure(yscrollcommand=scrollbar.set)

        canvas.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")

        self.scrollable_frame.bind("<Enter>", lambda _: self.bind_scroll(canvas))
        self.scrollable_frame.bind("<Leave>", lambda _: self.unbind_scroll(canvas))

    def bind_scroll(self, canvas):
        canvas.bind_all("<MouseWheel>", lambda e: canvas.yview_scroll(-1*(e.delta//120), "units"))

    def unbind_scroll(self, canvas):
        canvas.unbind_all("<MouseWheel>")

def load_save_path():
    if os.path.exists(SETTINGS_FILE):
        with open(SETTINGS_FILE, "r") as f:
            path = f.read().strip()
            if os.path.isdir(path):
                return path
    return None

def select_save_folder():
    folder_selected = filedialog.askdirectory()
    if folder_selected:
        with open(SETTINGS_FILE, "w") as f:
            f.write(folder_selected)
        save_path_label.config(text=f"Save Folder: {folder_selected}")
        return folder_selected
    return None

SAVE_DIR = load_save_path() or select_save_folder()
if SAVE_DIR:
    os.makedirs(SAVE_DIR, exist_ok=True)

protocols = []

root = tk.Tk()
root.title("ASAT Device - Protocol Generator")
root.geometry("800x600")

notebook = ttk.Notebook(root)
notebook.pack(fill="both", expand=True)

wells_data = {}

file_name_var = tk.StringVar(value="protocols.txt")

def create_parameter_fields(step_type, entries_frame, entries):
    for widget in entries_frame.winfo_children():
        widget.destroy()
    entries.clear()

    if step_type == "Agitation":
        labels = ["Volume (µL)", "% of Volume", "Speed", "Duration (s)"]
        keys = ["volume", "percent_volume", "speed", "duration"]
    elif step_type == "Pausing":
        labels = ["Pause Time (s)"]
        keys = ["pause_time"]
    elif step_type == "Moving":
        labels = ["Init Surface Time (s)", "Speed", "Sequences", "Sequence Pause Time (s)"]
        keys = ["init_surface_time", "speed", "sequences", "sequence_pause_time"]
    else:
        labels, keys = [], []

    for label, key in zip(labels, keys):
        ttk.Label(entries_frame, text=label).pack(side="left", padx=2)
        entry = ttk.Entry(entries_frame, width=8)
        entry.pack(side="left", padx=2)
        entries[key] = entry

def delete_step(well_name, step_frame, step_index):
    undo_stack.append(('step', well_name, wells_data[well_name]['steps'][step_index]))
    step_frame.destroy()
    wells_data[well_name]['steps'].pop(step_index)

def add_step(well_name, steps_frame):
    step_type = tk.StringVar(value="Agitation")

    step_frame = ttk.Frame(steps_frame.scrollable_frame)
    step_frame.pack(fill="x", pady=5)

    step_index = len(wells_data[well_name]["steps"])

    ttk.Label(step_frame, text=f"Step {step_index+1}:").pack(side="left", padx=5)

    step_menu = ttk.Combobox(step_frame, textvariable=step_type, values=["Agitation", "Pausing", "Moving"], width=10)
    step_menu.pack(side="left", padx=5)

    entries_frame = ttk.Frame(step_frame)
    entries_frame.pack(side="left", padx=5)

    entries = {}
    create_parameter_fields(step_type.get(), entries_frame, entries)

    step_menu.bind("<<ComboboxSelected>>", lambda e: create_parameter_fields(step_type.get(), entries_frame, entries))

    delete_step_button = ttk.Button(step_frame, text="🗑️ Delete", command=lambda: delete_step(well_name, step_frame, step_index))
    delete_step_button.pack(side="right", padx=5)

    wells_data[well_name]["steps"].append((step_type, entries))

def delete_well(well_name, well_frame):
    undo_stack.append(('well', well_name, copy.deepcopy(wells_data[well_name])))
    del wells_data[well_name]
    well_frame.destroy()


def add_well():
    if len(wells_data) >= 12:
        messagebox.showwarning("Limit Reached", "You can only add up to 12 wells.")
        return

    well_name = f"Well {len(wells_data) + 1}"
    wells_data[well_name] = {"steps": []}

    well_frame = ttk.Frame(notebook)
    notebook.add(well_frame, text=well_name)

    steps_frame = ScrollableFrame(well_frame)
    steps_frame.pack(fill="both", expand=True, pady=10)

    ttk.Button(well_frame, text="Add Step", command=lambda: add_step(well_name, steps_frame)).pack(pady=5)
    ttk.Button(well_frame, text="🛑 Delete Well", command=lambda: delete_well(well_name, well_frame)).pack(pady=5)

def generate_protocol_file():
    global protocols
    protocols = []

    for well_name, data in wells_data.items():
        for step_type, entries in data["steps"]:
            step_type_val = step_type.get()[0]  # First character

            if step_type.get() == "Agitation":
                values = [
                    entries["volume"].get(),
                    entries["percent_volume"].get(),
                    entries["speed"].get(),
                    entries["duration"].get()
                ]
                protocol_string = f"B{''.join(values)}"
            elif step_type.get() == "Pausing":
                protocol_string = f"P{entries['pause_time'].get()}"
            elif step_type.get() == "Moving":
                values = [
                    entries["init_surface_time"].get(),
                    entries["speed"].get(),
                    entries["sequences"].get(),
                    entries["sequence_pause_time"].get()
                ]
                protocol_string = f"M{''.join(values)}"
            else:
                continue  # Skip unknown types

            protocols.append(protocol_string)

    file_name = file_name_var.get().strip()
    if not file_name.endswith(".txt"):
        file_name += ".txt"

    protocol_file = os.path.join(SAVE_DIR, file_name)
    with open(protocol_file, 'w') as f:
        f.write("\n".join(protocols))

    qr = qrcode.make("\r\n".join(protocols))
    qr_file = os.path.join(SAVE_DIR, "protocol_qr.png")
    qr.save(qr_file)

    messagebox.showinfo("Success", f"Protocols saved as: {protocol_file}\nQR Code saved as: {qr_file}")

top_frame = ttk.Frame(root)
top_frame.pack(fill="x", pady=5)

file_frame = ttk.Frame(root)
file_frame.pack(fill="x", pady=5)

save_path_label = ttk.Label(root, text=f"Save Folder: {SAVE_DIR}", wraplength=500)
save_path_label.pack(pady=5)

ttk.Label(file_frame, text="File Name:").pack(side="left", padx=5)
file_name_entry = ttk.Entry(file_frame, textvariable=file_name_var)
file_name_entry.pack(side="left", padx=5)

def change_save_folder():
    global SAVE_DIR
    new_folder = select_save_folder()
    if new_folder:
        SAVE_DIR = new_folder

ttk.Button(top_frame, text="Add Well", command=add_well).pack(side="left", padx=5)
ttk.Button(top_frame, text="Generate Protocol File", command=generate_protocol_file).pack(side="left", padx=5)
ttk.Button(file_frame, text="Select Folder", command=change_save_folder).pack(side="left", padx=5)

# Buttons to add in the UI:
ttk.Button(root, text="Copy Well", command=lambda: copy_well(selected_well_name)).pack()
ttk.Button(root, text="Paste Well", command=paste_well).pack()
ttk.Button(root, text="Copy Step", command=lambda: copy_step(well_name, step_index)).pack()
ttk.Button(root, text="Paste Step", command=lambda: paste_step(well_name, steps_frame)).pack()
ttk.Button(root, text="Undo", command=undo).pack()

root.mainloop()
