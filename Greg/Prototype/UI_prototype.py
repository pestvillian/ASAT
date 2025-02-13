import qrcode
import tkinter as tk
from tkinter import messagebox, filedialog
import os

# File to store user-selected save path
SETTINGS_FILE = "settings.txt"
filename = "default"
# Default save path (loaded from file)
def load_save_path():
    if os.path.exists(SETTINGS_FILE):
        with open(SETTINGS_FILE, "r") as f:
            path = f.read().strip()
            if os.path.isdir(path):
                return path
    return None  # If no valid path found, return None

# Function to prompt user to select save folder
def select_save_folder():
    folder_selected = filedialog.askdirectory()
    if folder_selected:
        with open(SETTINGS_FILE, "w") as f:
            f.write(folder_selected)
        update_save_path_label(folder_selected)
        return folder_selected
    return None

# Load or set the save path
SAVE_DIR = load_save_path() or select_save_folder()

# Ensure directory exists
if SAVE_DIR:
    os.makedirs(SAVE_DIR, exist_ok=True)

# Update save path label in UI
def update_save_path_label(path):
    save_path_label.config(text=f"Save Folder: {path}")

# Global Variables
gcode = ["G28"]
step_entries = []  # List to store dynamically generated entry fields

# Function to generate G-code
def generate_gcode(x, z, speed):
    gcode.append(f"G0 Z{z} F{speed}")  # Move to starting point
    gcode.append(f"G0 X{x}")  # Move along X
    gcode.append("G0 Z0")
    gcode.append(f"G0 Z{z}")
    gcode.append("G0 Z0")
    gcode.append(f"G0 Z{z}")
    return "\n".join(gcode)

# Function to convert text file to QR code
def text_to_qr(text_file, output_file):
    with open(text_file, 'r') as f:
        text_data = f.read()

    qr = qrcode.QRCode(
        version=1,
        error_correction=qrcode.constants.ERROR_CORRECT_H,
        box_size=10,
        border=4,
    )
    
    qr.add_data(text_data)
    qr.make(fit=True)
    img = qr.make_image(fill_color="black", back_color="white")
    img.save(output_file)

# Function to create input fields dynamically
def create_step_inputs():
    try:
        num_steps = int(steps_entry.get())
        
        # Clear previous entries
        for widget in step_frame.winfo_children():
            widget.destroy()
        
        global step_entries
        step_entries = []  # Reset list
        
        # Create input fields for each step
        for i in range(num_steps):
            tk.Label(step_frame, text=f"Step {i+1}").grid(row=i, column=0, padx=5, pady=2)

            speed_var = tk.Entry(step_frame, width=8)
            speed_var.grid(row=i, column=1, padx=5)
            speed_var.insert(0, "100")  # Default value

            depth_var = tk.Entry(step_frame, width=8)
            depth_var.grid(row=i, column=2, padx=5)
            depth_var.insert(0, "5.0")

            increment_var = tk.Entry(step_frame, width=8)
            increment_var.grid(row=i, column=3, padx=5)
            increment_var.insert(0, "10.0")

            step_entries.append((speed_var, depth_var, increment_var))

        # Show generate button after fields are created
        generate_button.pack(pady=10)

    except ValueError:
        messagebox.showerror("Input Error", "Please enter a valid number of steps.")

# Function to generate G-code and QR Code (Silent Save)
def generate():
    if not SAVE_DIR:
        messagebox.showerror("Error", "No save directory selected. Please set a save folder first.")
        return
    try:
        filename = filename_entry.get().strip()
        if not filename:
            messagebox.showerror("Input Error", "Please enter a valid filename.")
            return
    except ValueError:
        messagebox.showerror("Input Error", "Please enter valid numerical values.")

    try:
        global gcode
        gcode = ["G28"]  # Reset G-code list

        for i, (speed_var, depth_var, increment_var) in enumerate(step_entries):
            speed = int(speed_var.get())
            depth = float(depth_var.get())
            increments = float(increment_var.get())

            gcode_text = generate_gcode(increments, depth, speed)

        gcode_text += "\nG28"  # Move home at end

         # File paths (one per step)
        gcode_file = os.path.join(SAVE_DIR, f"{filename}.gcode")
        qr_code_file = os.path.join(SAVE_DIR, f"{filename}.png")
        
        # Save G-code file automatically
        with open(gcode_file, 'w') as f:
            f.write(gcode_text)

        # Generate QR code automatically
        text_to_qr(gcode_file, qr_code_file)

    except ValueError:
        messagebox.showerror("Input Error", "Please enter valid numerical values.")

# Function to allow user to change save folder
def change_save_folder():
    global SAVE_DIR
    new_folder = select_save_folder()
    if new_folder:
        SAVE_DIR = new_folder

# UI Setup
root = tk.Tk()
root.title("ASAT Device - G-code Generator")
root.geometry("600x650")

# Filename Input
tk.Label(root, text="Enter File Name:").pack()
filename_entry = tk.Entry(root)
filename_entry.pack()

# Save Path Label
save_path_label = tk.Label(root, text=f"Save Folder: {SAVE_DIR}", wraplength=500)
save_path_label.pack(pady=5)


# Change Save Path Button
change_save_button = tk.Button(root, text="Change Save Folder", command=change_save_folder)
change_save_button.pack(pady=5)

# Step Count Input
tk.Label(root, text="Enter Number of Steps:").pack()
steps_entry = tk.Entry(root)
steps_entry.pack()

# Button to Generate Inputs
create_steps_button = tk.Button(root, text="Create Step Inputs", command=create_step_inputs)
create_steps_button.pack(pady=5)

# Frame to Hold Step Inputs
step_frame = tk.Frame(root)
step_frame.pack()

# Button to Generate G-code (Initially Hidden)
# Button to Generate Inputs


generate_button = tk.Button(root, text="Generate G-code & QR Code", command=generate)

# Output Text Box
output_text = tk.Text(root, height=10, width=60)
output_text.pack(pady=10)

# Run the UI
root.mainloop()
