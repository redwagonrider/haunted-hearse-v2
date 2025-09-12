#!/usr/bin/env python3
# Haunted Hearse Serial Console GUI
# Requires: pip install pyserial

import sys, time, threading, queue
try:
    import tkinter as tk
    from tkinter import ttk, messagebox
except:
    print("Tkinter not available")
    sys.exit(1)

try:
    import serial
    import serial.tools.list_ports as list_ports
except:
    print("pyserial is required. Install with: pip install pyserial")
    sys.exit(1)

APP_TITLE = "Haunted Hearse Console GUI"
BAUD = 115200

SCENE_LIST = [
    "STANDBY", "PHONELOADING", "INTRO", "BLOODROOM",
    "GRAVEYARD", "FURROOM", "ORCADINO", "FRANKENLAB",
    "MIRRORROOM", "EXITHOLE"
]

class SerialWorker(threading.Thread):
    def __init__(self, ser, out_queue, stop_event):
        super().__init__(daemon=True)
        self.ser = ser
        self.out_queue = out_queue
        self.stop_event = stop_event

    def run(self):
        buf = b""
        while not self.stop_event.is_set():
            try:
                if self.ser and self.ser.in_waiting:
                    data = self.ser.read(self.ser.in_waiting)
                    buf += data
                    while b"\n" in buf:
                        line, buf = buf.split(b"\n", 1)
                        try:
                            self.out_queue.put(line.decode(errors="replace"))
                        except:
                            self.out_queue.put(repr(line))
                else:
                    time.sleep(0.02)
            except Exception as e:
                self.out_queue.put(f"[ERROR] {e}")
                time.sleep(0.25)

class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title(APP_TITLE)
        self.geometry("900x650")

        self.ser = None
        self.reader_queue = queue.Queue()
        self.reader_stop = threading.Event()
        self.reader_thread = None

        self._build_ui()
        self._poll_reader()

    def _build_ui(self):
        ports_frame = ttk.Frame(self)
        ports_frame.pack(fill="x", padx=10, pady=8)

        ttk.Label(ports_frame, text="Serial Port:").pack(side="left")
        self.port_cmb = ttk.Combobox(ports_frame, values=self._list_ports(), width=30)
        self.port_cmb.pack(side="left", padx=6)
        ttk.Button(ports_frame, text="Refresh", command=self._refresh_ports).pack(side="left")
        ttk.Button(ports_frame, text="Connect", command=self._connect).pack(side="left", padx=6)
        ttk.Button(ports_frame, text="Disconnect", command=self._disconnect).pack(side="left")

        left = ttk.Frame(self)
        left.pack(side="left", fill="y", padx=10, pady=4)

        self._add_slider(left, "HOLD (ms)", 0, 30000, lambda v: self._send_cmd(f"HOLD {v}"))
        self._add_slider(left, "COOL (ms)", 0, 600000, lambda v: self._send_cmd(f"COOL {v}"))
        self._add_slider(left, "BRIGHT (0..15)", 0, 15, lambda v: self._send_cmd(f"BRIGHT {v}"))
        self._add_slider(left, "SDEB (ms)", 0, 2000, lambda v: self._send_cmd(f"SDEB {v}"))
        self._add_slider(left, "SREARM (ms)", 0, 600000, lambda v: self._send_cmd(f"SREARM {v}"))

        btns = ttk.LabelFrame(left, text="Actions")
        btns.pack(fill="x", pady=8)
        for cmd in ["CFG","MAP","SAVE","LOAD","LOG ON","LOG OFF"]:
            ttk.Button(btns, text=cmd, command=lambda c=cmd: self._send_cmd(c)).pack(fill="x", pady=2)

        mid = ttk.Frame(self)
        mid.pack(side="left", fill="both", expand=True, padx=6, pady=4)

        scene_frame = ttk.LabelFrame(mid, text="Scene Control")
        scene_frame.pack(fill="x")
        self.scene_var = tk.StringVar(value="FRANKENLAB")
        ttk.OptionMenu(scene_frame, self.scene_var, "FRANKENLAB", *SCENE_LIST).pack(side="left", padx=4, pady=6)
        ttk.Button(scene_frame, text="SCENE GO", command=self._scene_go).pack(side="left", padx=4)

        right = ttk.Frame(self)
        right.pack(side="left", fill="both", expand=True, padx=8, pady=4)

        self.out_txt = tk.Text(right, height=30, wrap="word")
        self.out_txt.pack(fill="both", expand=True)
        out_btns = ttk.Frame(right)
        out_btns.pack(fill="x")
        ttk.Button(out_btns, text="Clear", command=lambda: self.out_txt.delete("1.0", "end")).pack(side="left")
        ttk.Button(out_btns, text="Send Raw...", command=self._send_raw_dialog).pack(side="right")

    def _add_slider(self, parent, label, a, b, onrelease):
        frm = ttk.LabelFrame(parent, text=label)
        frm.pack(fill="x", pady=6)
        var = tk.IntVar(value=a)
        sld = ttk.Scale(frm, from_=a, to=b, orient="horizontal", variable=var)
        sld.pack(fill="x", padx=6, pady=4)
        def on_release(event):
            onrelease(int(var.get()))
        sld.bind("<ButtonRelease-1>", on_release)

    def _list_ports(self):
        return [p.device for p in list_ports.comports()]

    def _refresh_ports(self):
        self.port_cmb["values"] = self._list_ports()

    def _connect(self):
        port = self.port_cmb.get().strip()
        if not port:
            messagebox.showerror("Port", "Select a serial port.")
            return
        try:
            self._disconnect()
            self.ser = serial.Serial(port, BAUD, timeout=0.05)
            self.reader_stop.clear()
            self.reader_thread = SerialWorker(self.ser, self.reader_queue, self.reader_stop)
            self.reader_thread.start()
            self._log(f"[OK] Connected {port} @ {BAUD}")
            self._send_cmd("CFG")
        except Exception as e:
            messagebox.showerror("Serial", f"Failed to connect: {e}")

    def _disconnect(self):
        try:
            if self.reader_thread:
                self.reader_stop.set()
                self.reader_thread.join(timeout=0.3)
        except: pass
        finally:
            self.reader_thread = None
            self.reader_stop.clear()
        try:
            if self.ser:
                self.ser.close()
        except: pass
        self.ser = None

    def _send(self, text):
        if not self.ser: return
        try:
            self.ser.write((text + "\n").encode())
        except Exception as e:
            self._log(f"[ERROR] write: {e}")

    def _send_cmd(self, cmd):
        self._log(f"> {cmd}")
        self._send(cmd)

    def _send_raw_dialog(self):
        win = tk.Toplevel(self)
        win.title("Send Raw Command")
        ent = tk.Entry(win, width=40)
        ent.pack(padx=8, pady=6)
        ent.focus_set()
        def go():
            self._send_cmd(ent.get())
            win.destroy()
        ttk.Button(win, text="Send", command=go).pack(pady=6)
        win.bind("<Return>", lambda e: go())

    def _scene_go(self):
        name = self.scene_var.get().strip().upper()
        self._send_cmd(f"SCENE {name}")

    def _log(self, text):
        self.out_txt.insert("end", text + "\n")
        self.out_txt.see("end")

    def _poll_reader(self):
        try:
            while True:
                line = self.reader_queue.get_nowait()
                self._log(line)
        except queue.Empty:
            pass
        self.after(30, self._poll_reader)

    def on_close(self):
        self._disconnect()
        self.destroy()

if __name__ == "__main__":
    app = App()
    app.protocol("WM_DELETE_WINDOW", app.on_close)
    app.mainloop()
