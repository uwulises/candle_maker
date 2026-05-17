from flask import Flask, render_template, request, jsonify
import serial
import serial.tools.list_ports
import json
import os
import time

app = Flask(__name__)

PRESET_FILE = "presets.json"

ser = None
status_text = "Idle"

# ---------------- Presets ----------------

def load_presets():
    if os.path.exists(PRESET_FILE):
        with open(PRESET_FILE, "r") as f:
            return json.load(f)
    return {}

presets = load_presets()

# ---------------- Serial ----------------

def get_ports():
    return [p.device for p in serial.tools.list_ports.comports()]

def connect_serial(port):
    global ser

    if ser and ser.is_open:
        ser.close()

    ser = serial.Serial(port, 115200, timeout=0.1)
    return True

def send(cmd):
    global ser

    if ser and ser.is_open:
        ser.write((cmd + "\n").encode())
        return True

    return False

# ---------------- Routes ----------------

@app.route("/")
def index():
    return render_template(
        "index.html",
        ports=get_ports(),
        presets=presets.keys()
    )

@app.route("/connect", methods=["POST"])
def connect():
    global status_text

    port = request.form["port"]

    try:
        connect_serial(port)
        status_text = "Connected"
        return jsonify(success=True)

    except Exception as e:
        return jsonify(success=False, error=str(e))

@app.route("/home", methods=["POST"])
def home():
    global status_text

    send("Homing")
    status_text = "Homing"

    return jsonify(success=True)

@app.route("/move_absolute", methods=["POST"])
def move_absolute():
    global status_text

    target = request.form["target"]

    send(f"Moveto_mm{target}")

    status_text = f"Moving to {target} mm"

    return jsonify(success=True)

@app.route("/next_index", methods=["POST"])
def next_index():
    global status_text

    send("Next_index")

    status_text = "Moving to next index"

    return jsonify(success=True)

@app.route("/run_preset", methods=["POST"])
def run_preset():
    global status_text

    name = request.form["preset"]

    if name not in presets:
        return jsonify(success=False)

    preset = presets[name]

    for position in preset["positions"]:
        send("Next_index")
        time.sleep(1)

        send(f"Moveto_mm{position}")
        time.sleep(10)

    status_text = f"Preset '{name}' completed"

    return jsonify(success=True)

@app.route("/status")
def status():
    return jsonify(status=status_text)

# ---------------- Main ----------------

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)