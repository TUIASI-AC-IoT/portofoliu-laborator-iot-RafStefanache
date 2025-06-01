from flask import Flask, request, jsonify, abort
import os

app = Flask(__name__)
BASE_DIR = 'files'
SENSOR_CONFIG_DIR = 'sensor_configs'
SENSOR_VALUES = {'1': 22.5, '2': 35.0}  # simulare valori senzori

os.makedirs(BASE_DIR, exist_ok=True)
os.makedirs(SENSOR_CONFIG_DIR, exist_ok=True)

@app.route('/files', methods=['GET'])
def list_files():
    return jsonify(os.listdir(BASE_DIR))

@app.route('/files/<filename>', methods=['GET'])
def read_file(filename):
    try:
        with open(os.path.join(BASE_DIR, filename), 'r') as f:
            return f.read()
    except FileNotFoundError:
        abort(404, description="File not found")

@app.route('/files', methods=['POST'])
def create_file():
    data = request.get_json()
    filename = data.get('filename')
    content = data.get('content', '')
    if not filename:
        filename = f"file_{len(os.listdir(BASE_DIR))+1}.txt"
    with open(os.path.join(BASE_DIR, filename), 'w') as f:
        f.write(content)
    return jsonify({"message": "File created", "filename": filename}), 201

@app.route('/files/<filename>', methods=['DELETE'])
def delete_file(filename):
    try:
        os.remove(os.path.join(BASE_DIR, filename))
        return jsonify({"message": "File deleted"})
    except FileNotFoundError:
        abort(404, description="File not found")

@app.route('/files/<filename>', methods=['PUT'])
def update_file(filename):
    content = request.get_json().get('content', '')
    try:
        with open(os.path.join(BASE_DIR, filename), 'w') as f:
            f.write(content)
        return jsonify({"message": "File updated"})
    except:
        abort(400, description="Could not update file")

# SENSOR LOGIC
@app.route('/sensor/<sensor_id>', methods=['GET'])
def get_sensor_value(sensor_id):
    if sensor_id in SENSOR_VALUES:
        return jsonify({"sensor_id": sensor_id, "value": SENSOR_VALUES[sensor_id]})
    else:
        abort(404, description="Sensor not found")

@app.route('/sensor/<sensor_id>', methods=['POST'])
def create_sensor_config(sensor_id):
    config_path = os.path.join(SENSOR_CONFIG_DIR, f"{sensor_id}.json")
    if os.path.exists(config_path):
        abort(409, description="Configuration already exists")
    config_data = request.get_json()
    with open(config_path, 'w') as f:
        f.write(str(config_data))
    return jsonify({"message": "Configuration created"})

@app.route('/sensor/<sensor_id>/<filename>', methods=['PUT'])
def replace_sensor_config(sensor_id, filename):
    config_path = os.path.join(SENSOR_CONFIG_DIR, filename)
    if not os.path.exists(config_path):
        abort(404, description="Configuration does not exist")
    config_data = request.get_json()
    with open(config_path, 'w') as f:
        f.write(str(config_data))
    return jsonify({"message": "Configuration replaced"})

if __name__ == '__main__':
    app.run(debug=True)
