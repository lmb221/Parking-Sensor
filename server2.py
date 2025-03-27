from flask import Flask, request, jsonify
import socket
import threading

app = Flask(__name__)

# Latest data received
latest_data = {"id": 0, "spot0": "1S", "spot1": "1S", "spot2": "1S"}


def get_ip_address():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(('10.255.255.255', 1))
        IP = s.getsockname()[0]
    except Exception:
        IP = '127.0.0.1'
    finally:
        s.close()
    return IP


@app.route('/submit', methods=['POST'])
def submit_data():
    global latest_data
    request_data = request.json
    for key, value in request_data.items():
        if key in latest_data:
            if key.startswith("spot") and isinstance(value, int):
                # Extract the current suffix ("H" or "S") from the existing value
                current_suffix = latest_data[key][-1] if len(latest_data[key]) > 1 else ""
                latest_data[key] = f"{value}{current_suffix}"
            else:
                latest_data[key] = value
    latest_data.update(request_data)  # Update received JSON
    print(f"Received data: {request_data}")
    return jsonify({"message": "Data received successfully!"}), 200


@app.route('/get', methods=['GET'])
def get_data():
    print(f"Sending data: {latest_data}")  # Debugging output
    return jsonify(latest_data), 200


def terminal_update():
    global latest_data
    while True:
        print("\nCurrent data:", latest_data)
        key = input("Enter the key to update the tag (e.g., spot0, spot1): ")
        if key not in latest_data or not key.startswith("spot"):
            print(f"Invalid key: {key}. Only 'spot' keys can be updated. Try again.")
            continue
        value = input(f"Enter the new tag for {key} (H or S): ").strip().upper()
        if value not in ["H", "S"]:
            print(f"Invalid tag: {value}. Only 'H' or 'S' are allowed. Try again.")
            continue
        # Preserve the numeric part and update only the tag
        num = latest_data[key][:-1] if len(latest_data[key]) > 1 else latest_data[key]
        latest_data[key] = f"{num}{value}"
        print(f"Updated {key} to {latest_data[key]}.")


if __name__ == '__main__':
    ip_address = get_ip_address()
    print(f"Server starting on IP: {ip_address}")

    # Run the terminal update function in a separate thread
    threading.Thread(target=terminal_update, daemon=True).start()

    app.run(host='0.0.0.0', port=5000)