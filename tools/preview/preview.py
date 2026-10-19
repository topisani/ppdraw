#!/bin/env -S python

import sys
import math
import select
import tkinter as tk
from tkinter import ttk
import threading
import queue
import time
import argparse
import subprocess
from dataclasses import dataclass
from pathlib import Path


def iter_pixels(data, bpp):
    mask = (1 << bpp) - 1
    ppbyte = 8 // bpp
    for byte in data:
        for j in reversed(range(ppbyte)):
            yield (byte >> (j * bpp)) & mask

@dataclass
class Img:
    width: int
    height: int
    pixels: list

@dataclass
class Param:
    value: int
    min: int
    max: int

class PreviewUI:
    grid_color = "#101010"

    def __init__(self):
        self.command_queue = queue.Queue()
        self.root = tk.Tk()
        self.canvas = None
        self.img = None
        self.running = False
        self.scale = 5
        self.stop_event = threading.Event()
        self.params = {}
        self.selected_widget = None

        self.canvas = tk.Canvas(
            self.root,
            bg=self.grid_color, borderwidth=0, highlightthickness=0
        )

    def _draw_img(self):
        """Draw a grid of pixels"""
        w = self.canvas.winfo_width()
        h = self.canvas.winfo_height()

        if self.img.width == 0 or self.img.height == 0:
            return

        scale = max(1, min(w / self.img.width, h / self.img.height))
        x0 = (w - self.img.width * scale) / 2
        y0 = (h - self.img.height * scale) / 2

        for y in range(self.img.height):
            for x in range(self.img.width):
                idx = x + y * self.img.width
                pixel = self.img.pixels[idx] if idx < len(self.img.pixels) else 0
                gs = pixel / (2**self.img.bpp -1)
                color = f"#{int(gs * 255):02X}{int(gs * 255):02X}{int(gs * 255):02X}"

                # Draw the pixel
                self.canvas.create_rectangle(
                    x0 + x * scale,
                    y0 + y * scale,
                    x0 + (x + 1) * scale,
                    y0 + (y + 1) * scale,
                    fill=color,
                    outline=self.grid_color if scale > 3 else "",
                )

    def main_loop(self):
        """Main UI loop running in background thread"""
        self.running = True
        self.stop_event.clear()
        self.root.configure(background=self.grid_color, borderwidth=self.scale)


        self.canvas.pack(fill='both', expand=True)
        self.root.bind("<Button-3>", self.show_popup)
        self.canvas.bind("<Button-3>", self.show_popup)

        # Process commands periodically
        self._process_commands()

        self.root.geometry(
            f"{self.scale * self.img.width}x{self.scale * self.img.height}"
        )

        self.root.mainloop()
        self.running = False
        self.stop_event.set()
    
    def show_popup(self, event):
        popup_menu = tk.Menu(self.root, tearoff=0)


        for name in self.widgets:
            def command(name=name):
                self.selected_widget = name
                self.on_widget_selected(name)
            popup_menu.add_command(label=name, 
                                  command=command)
        
        try:
            popup_menu.tk_popup(event.x_root, event.y_root)
        finally:
            popup_menu.grab_release()

    def on_widget_selected(self, widget_name):
        pass

    def _process_commands(self):
        # Schedule next update
        self.canvas.delete("all")
        self._draw_img()

        if self.root:
            self.root.after(10, self._process_commands)

    def schedule_after(self, timeout_ms, function):
        if self.root:
            self.root.after(timeout_ms, function)

    def set_img(self, img: Img):
        self.img = img
        w = self.img.width
        h = self.img.height
        self.root.title(f"ppdraw preview: {self.selected_widget} ({w}x{h})")
        self.canvas.setvar("width", w * self.scale)
        self.canvas.setvar("height", h * self.scale)

        winw = self.scale * (w + 2)
        winh = self.scale * (h + 2)
        self.root.minsize(winw, winh)


class PreviewExe:
    def __init__(self, path):
        self.path = Path(path)
        self._last_mtime = 0
        self.params = {}

    def list_widgets(self) -> list[str]:
        result = subprocess.run([self.path], capture_output=True)
        if result.returncode != 0:
            print(f"CMD Failed with returncode: {result.returncode}")
        widgets = []
        for line in result.stdout.decode().splitlines():
            if line.strip():
                widgets.append(line)
        return widgets

    def get_param(self, name, min, max, default_value):
        if name not in self.params:
            self.params[name] = Param(value=default_value, min=min, max=max)
        return self.params[name].value

    def render_widget(self, name: str) -> Img:
        img = Img(0, 0, bytearray())
        process = subprocess.Popen([self.path, name], stdout=subprocess.PIPE, stdin=subprocess.PIPE, text=True, bufsize=1)

        for line in iter(process.stdout.readline, ''):
            line = line.strip()
            if not line:
                continue

            split = line.split("=", 1)
            field, rest = split[0], split[1]
            if field == "param":
                param, kind, min, max, default_val = tuple(rest.split(":", 4))
                newval = self.get_param(param, int(min), int(max), int(default_val))
                process.stdin.write(f"param={param}:{newval}\n")

            if field == "width":
                img.width = int(rest)
            if field == "height":
                img.height = int(rest)
            if field == "bpp":
                img.bpp = int(rest)
            if field == "data":
                data = list()
                while len(rest) > 0:
                    data.append(int(rest[0:2], 16))
                    rest = rest[2:]
                img.pixels = list(iter_pixels(data, img.bpp))

        process.wait()
        return img

    def check_changed(self) -> bool:
        try:
            mtime = self.path.stat().st_mtime
        except RuntimeError:
            return False # File might be being recompiled, act like nothing happened
        if mtime != self._last_mtime:
            self._last_mtime = mtime
            return True
        return False

def run_cmake(cmd):
    result = subprocess.run(cmd)
    return result

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--preview-exec", help="Preview executable")
    parser.add_argument("-w", "--watch", help="Watch for the binary to be updated", action='store_true')
    parser.add_argument("-s", "--scale", help="render scale", type=int, default=3)
    parser.add_argument("--build", help="Cmake build dir")
    parser.add_argument("-t", "--target", help="cmake target")
    args = parser.parse_args()

    exe_name = args.preview_exec
    if not exe_name:
        exe_name = f"{args.build}/{args.target}"

    cmake_cmd = ["cmake", "--build", args.build, "--target", args.target]

    exe = PreviewExe(exe_name)
    ui = PreviewUI()
    ui.scale = args.scale

    def render():
        print("rendering", ui.selected_widget)
        exe.params = ui.params
        img = exe.render_widget(ui.selected_widget or "")
        ui.params = exe.params
        ui.set_img(img)


    def load_and_render():
        run_cmake(cmake_cmd)
        if exe.check_changed():
            ui.widgets = exe.list_widgets()
            if ui.selected_widget not in ui.widgets:
                ui.selected_widget = ui.widgets[0]
            render()

        if args.watch:
            ui.schedule_after(1000, load_and_render)


    load_and_render()

    ui.params_changed = render
    ui.on_widget_selected = lambda x: render()

    ui.main_loop()

    # if watch:
    #     modtime = lib_path.stat().st_mtime
    #     while ui.running:
    #         try:
    #             run_cmake(cmake_cmd)
    #         except Exception as error:
    #             print("cmake run failed: ", error)
    #         try:
    #             mtime = lib_path.stat().st_mtime
    #         except FileNotFoundError:
    #             pass
    #         if mtime > modtime:
    #             modtime = mtime
    #             load_and_render()

    #         # Wait for enter in terminal or timeout
    #         input, _, _ = select.select([sys.stdin], [], [], 1)
    #         if input:
    #             sys.stdin.readline()


if __name__ == "__main__":
    main()
