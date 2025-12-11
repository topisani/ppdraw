#!/bin/env python -u

import ctypes
import sys
import math
import select
import tkinter as tk
import threading
import queue
import time
import contextlib
import subprocess
from dataclasses import dataclass
from pathlib import Path

@dataclass
class Img:
    width: int
    height: int
    data: list

class PreviewUI:
    grid_color = '#101010'

    def __init__(self):
        self.command_queue = queue.Queue()
        self.root = None
        self.canvas = None
        self.ui_thread = None
        self.img = None
        self.running = False
        self.scale = 9
        self.stop_event = threading.Event()
        
    def draw_img(self):
        """Draw a grid of pixels"""
        w = self.canvas.winfo_width()
        h = self.canvas.winfo_height()

        scale = max(1, min(w / self.img.width, h / self.img.height))
        x0 = (w - self.img.width * scale) / 2
        y0 = (h - self.img.height* scale) / 2

        for y in range(self.img.height):
            for x in range(self.img.width):
                idx = x + y * self.img.width
                pixel = self.img.data[idx] if idx < len(self.img.data) else 0
                color = 'white' if pixel != 0 else 'black'
                
                # Draw the pixel
                self.canvas.create_rectangle(
                    x0 + x * scale,
                    y0 + y * scale,
                    x0 + (x + 1) * scale,
                    y0 + (y + 1) * scale,
                    fill=color,
                    outline=self.grid_color if scale > 1 else ''
                )
    
    def main_loop(self):
        """Main UI loop running in background thread"""
        self.root = tk.Tk()
        self.root.title("ppdraw preview")
        self.root.configure(background=self.grid_color, borderwidth=self.scale)
        
        self.canvas = tk.Canvas(self.root, bg=self.grid_color, borderwidth=0, highlightthickness=0)
        self.canvas.pack(fill=tk.BOTH, expand=True)

        # Process commands periodically
        self.process_commands()

        self.root.geometry(f"{self.scale * self.img.width}x{self.scale * self.img.height}")
        
        self.root.mainloop()
        self.running = False
        self.stop_event.set()

    def process_commands(self):
        """Process commands from the queue"""
        while not self.command_queue.empty():
            try:
                cmd, data = self.command_queue.get_nowait()
                
                if cmd == 'img':
                    self.img = data
                    
                    w = self.img.width
                    h = self.img.height
                    
                    winw = self.scale * (w + 2)
                    winh = self.scale * (h + 2)
                    self.root.minsize(winw, winh)
                    
                elif cmd == 'exit':
                    self.root.quit()
                    return
                    
            except queue.Empty:
                break
        
        self.canvas.delete('all')
        self.draw_img()

        # Schedule next update
        if self.root:
            self.root.after(10, self.process_commands)
    
    def start(self):
        """Start the UI in a background thread"""
        if self.running:
            return
        
        self.running = True
        self.stop_event.clear()
        self.ui_thread = threading.Thread(target=self.main_loop, daemon=True)
        self.ui_thread.start()
        
        # Give UI time to initialize
        time.sleep(0.5)
    
    def stop(self):
        """Stop the UI thread"""
        self.command_queue.put(('exit', None))
    
    def wait(self):
        """Wait for the UI thread to finish"""
        if self.ui_thread:
            self.stop_event.wait()
            self.ui_thread.join()
    
    def set_img(self, img: Img):
        """
        Send an image to the UI
        img should be a dict with keys: 'w', 'h', 'data'
        where data is a flat list of pixel values (0 or 1)
        """
        self.command_queue.put(('img', img))

# Opaque type for draw_t
class c_draw_t(ctypes.Structure):
    pass

# Function pointer type for draw callback
c_draw_cb = ctypes.CFUNCTYPE(None, c_draw_t)

# Structure definitions
class c_ppdraw_widget(ctypes.Structure):
    _fields_ = [
        ("width", ctypes.c_int),
        ("height", ctypes.c_int),
        ("draw_cb", c_draw_cb),
    ]

class c_ppdraw_widget_registration(ctypes.Structure):
    _fields_ = [
        ("widget", ctypes.POINTER(c_ppdraw_widget)),
        ("name", ctypes.c_char_p),
    ]

@dataclass
class Widget:
    c_reg: c_ppdraw_widget_registration

    @property
    def name(self):
        return self.c_reg.name.decode("utf-8")

    @property
    def width(self) -> int:
        return int(self.c_reg.widget.contents.width)

    @property
    def height(self) -> int:
        return int(self.c_reg.widget.contents.height)

class PreviewLib:

    def __init__(self, path):
        self.lib = ctypes.CDLL(str(path))

        self.lib.ppdraw_widget_registry_count.argtypes = []
        self.lib.ppdraw_widget_registry_count.restype = ctypes.c_size_t

        self.lib.ppdraw_widget_registry_get.argtypes = [ctypes.c_size_t]
        self.lib.ppdraw_widget_registry_get.restype = ctypes.POINTER(c_ppdraw_widget_registration)

        self.lib.ppdraw_ctx_alloc.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_int]
        self.lib.ppdraw_ctx_alloc.restype = ctypes.POINTER(c_draw_t)

        self.lib.ppdraw_ctx_free.argtypes = [ctypes.POINTER(c_draw_t)]
        self.lib.ppdraw_ctx_free.restype = None

        # self.lib.ppdraw_ctx_get_buf.argtypes = [ctypes.POINTER(c_draw_t)]
        # self.lib.ppdraw_ctx_get_buf.restype = ctypes.POINTER(ctypes.c_ubyte)
         
        self.lib.ppdraw_ctx_render_widget.argtypes = [ctypes.POINTER(c_draw_t), ctypes.POINTER(c_ppdraw_widget)]
        self.lib.ppdraw_ctx_render_widget.restype = ctypes.POINTER(ctypes.c_ubyte)

        count = self.lib.ppdraw_widget_registry_count()
        self.widgets = [Widget(self.lib.ppdraw_widget_registry_get(i).contents) for i in range(count)]

    def close(self):
        if self.lib:
            dlclose_func = ctypes.CDLL(None).dlclose
            dlclose_func.argtypes = [ctypes.c_void_p]
            dlclose_func.restype = ctypes.c_int

            dlclose_func(self.lib._handle)
            self.lib = None

    def __enter__(self):
        return self

    def __exit__(self, *args):
        self.close()

    @contextlib.contextmanager
    def draw_ctx(self, width, height, bpp=8):
        ctx = self.lib.ppdraw_ctx_alloc(width, height, bpp)
        yield ctx
        self.lib.ppdraw_ctx_free(ctx)

    def render_widget(self, widget) -> Img:
        img = Img(width=widget.width, height=widget.height, data=None) 
        with self.draw_ctx(widget.width, widget.height) as ctx:
            buf = self.lib.ppdraw_ctx_render_widget(ctx, widget.c_reg.widget)
            img.data = bytes(buf[i] for i in range(widget.width * widget.height))
        return img


def run_cmake(cmd):
    result = subprocess.run(cmd)
    return result


def main():
    ui = PreviewUI()

    lib_path = Path(sys.argv[1])
    watch = sys.argv[2] == "--watch"
    if watch:
        cmake_cmd = sys.argv[3:]

    def load_and_render():
        with PreviewLib(lib_path) as lib:
            widget = lib.widgets[0]
            img = lib.render_widget(widget)
            ui.set_img(img)

    load_and_render()

    ui.start()

    if watch:
        modtime = lib_path.stat().st_mtime
        while ui.running:
            try:
                run_cmake(cmake_cmd)
            except Exception as error:
                print("cmake run failed: ", error)
            try:
                mtime = lib_path.stat().st_mtime
            except FileNotFoundError:
                pass
            if mtime > modtime:
                modtime = mtime
                load_and_render()

            # Wait for enter in terminal or timeout
            input, _, _ = select.select([sys.stdin], [], [], 1)
            if input:
                sys.stdin.readline()

    ui.wait()


if __name__ == "__main__":
    main()
